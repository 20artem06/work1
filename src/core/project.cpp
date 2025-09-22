#include "project.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QTextStream>

namespace video {

namespace {

QString resolveRelativePath(const QString& baseDir, const QString& fileName)
{
    QFileInfo info(fileName);
    if (info.isAbsolute()) {
        return info.absoluteFilePath();
    }
    QDir dir(baseDir);
    return dir.filePath(fileName);
}

}  // namespace

Project::Project()
{
    m_layerFlags.fill(false);
}

Project::~Project() = default;

Project::Project(Project&&) noexcept = default;
Project& Project::operator=(Project&&) noexcept = default;

void Project::setProjectPath(const QString& path)
{
    QString normalised = QDir::cleanPath(path);
    if (!normalised.isEmpty() && !normalised.endsWith(QDir::separator())) {
        normalised += QDir::separator();
    }
    m_projectPath = normalised;
}

void Project::setDataFilePath(const QString& path)
{
    m_dataFilePath = resolveRelativePath(m_projectPath, path);
}

Project Project::loadFromDescription(const QString& descriptionPath)
{
    QFile descFile(descriptionPath);
    if (!descFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(QStringLiteral("Failed to open project description: %1")
                                     .arg(descriptionPath)
                                     .toStdString());
    }

    QTextStream stream(&descFile);
    stream.setLocale(QLocale::c());

    Project project;
    QFileInfo info(descFile);
    project.setProjectPath(info.absolutePath());
    project.setName(info.fileName());

    QString dataFileName;
    stream >> dataFileName;
    if (stream.status() != QTextStream::Ok) {
        throw std::runtime_error("Project description is malformed (missing data file name).");
    }
    project.setDataFilePath(dataFileName);

    QString timeName;
    stream >> timeName;
    if (stream.status() != QTextStream::Ok) {
        throw std::runtime_error("Project description is malformed (missing time axis name).");
    }
    project.setTimeName(timeName);

    QString timeUnit;
    stream >> timeUnit;
    if (stream.status() != QTextStream::Ok) {
        throw std::runtime_error("Project description is malformed (missing time axis unit).");
    }
    if (timeUnit == QLatin1String("_")) {
        timeUnit.clear();
    }
    project.setTimeUnit(timeUnit);

    int fieldCount = 0;
    stream >> fieldCount;
    if (stream.status() != QTextStream::Ok || fieldCount <= 0) {
        throw std::runtime_error("Project description is malformed (invalid field count).");
    }

    project.m_fields.resize(fieldCount);
    project.m_fieldSampleTotal = 0;

    for (int fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex) {
        Field field;

        stream >> field.name;
        if (stream.status() != QTextStream::Ok) {
            throw std::runtime_error("Project description is malformed (missing field name).");
        }

        stream >> field.unit;
        if (stream.status() != QTextStream::Ok) {
            throw std::runtime_error("Project description is malformed (missing field unit).");
        }
        if (field.unit == QLatin1String("_")) {
            field.unit.clear();
        }

        int axisCount = 0;
        stream >> axisCount;
        if (stream.status() != QTextStream::Ok || axisCount < 0) {
            throw std::runtime_error("Project description is malformed (invalid axis count).");
        }
        if (axisCount >= 1) {
            project.m_oneDimOnly = false;
        }
        field.axes.resize(axisCount);

        qsizetype sampleCount = 1;
        for (int axisIndex = 0; axisIndex < axisCount; ++axisIndex) {
            Axis axis;

            stream >> axis.name;
            if (stream.status() != QTextStream::Ok) {
                throw std::runtime_error("Project description is malformed (missing axis name).");
            }

            stream >> axis.unit;
            if (stream.status() != QTextStream::Ok) {
                throw std::runtime_error("Project description is malformed (missing axis unit).");
            }
            if (axis.unit == QLatin1String("_")) {
                axis.unit.clear();
            }

            int axisPoints = 0;
            stream >> axisPoints;
            if (stream.status() != QTextStream::Ok) {
                throw std::runtime_error("Project description is malformed (invalid axis point count).");
            }

            if (axisPoints != 0) {
                const int pointsToRead = std::abs(axisPoints);
                axis.points.resize(pointsToRead);

                if (axisPoints > 0) {
                    double start = 0.0;
                    double step = 0.0;
                    stream >> start;
                    stream >> step;
                    if (stream.status() != QTextStream::Ok || step <= 0.0) {
                        throw std::runtime_error("Project description is malformed (invalid axis step).");
                    }
                    axis.points[0] = static_cast<float>(start);
                    for (int point = 1; point < pointsToRead; ++point) {
                        axis.points[point] = axis.points[point - 1] + static_cast<float>(step);
                    }
                } else {
                    for (int point = 0; point < pointsToRead; ++point) {
                        double value = 0.0;
                        stream >> value;
                        if (stream.status() != QTextStream::Ok) {
                            throw std::runtime_error("Project description is malformed (invalid axis value).");
                        }
                        axis.points[point] = static_cast<float>(value);
                    }
                }
                sampleCount *= axis.points.size();
            }

            field.axes[axisIndex] = std::move(axis);
        }

        field.setSampleCount(sampleCount);
        project.m_fieldSampleTotal += sampleCount;
        project.m_fields[fieldIndex] = std::move(field);
    }

    project.m_recordStride = project.m_fieldSampleTotal + 1; // include time value

    descFile.close();

    project.readGrid(QStringLiteral("data.grd"));

    QFile dataFile(project.m_dataFilePath);
    if (!dataFile.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(QStringLiteral("Failed to open project data file: %1")
                                     .arg(project.m_dataFilePath)
                                     .toStdString());
    }

    const qint64 fileSize = dataFile.size();
    if (project.m_recordStride <= 0) {
        throw std::runtime_error("Project has invalid record stride.");
    }
    const qint64 recordSizeBytes = static_cast<qint64>(project.m_recordStride) * sizeof(float);
    if (recordSizeBytes == 0) {
        throw std::runtime_error("Project has zero-sized records.");
    }

    const qsizetype timeCount = static_cast<qsizetype>(fileSize / recordSizeBytes);
    if (timeCount < 1) {
        throw std::runtime_error("Project data file is too small.");
    }

    project.m_timeValues.resize(timeCount);

    project.m_inMemory = fileSize < kMemMaxBytes;
    if (project.m_inMemory) {
        for (Field& field : project.m_fields) {
            field.samples.resize(field.sampleCount() * timeCount);
            field.minimum = std::numeric_limits<float>::max();
            field.maximum = std::numeric_limits<float>::lowest();
        }
    } else {
        for (Field& field : project.m_fields) {
            field.minimum = -std::numeric_limits<float>::max();
            field.maximum = std::numeric_limits<float>::max();
        }
    }

    QDataStream dataStream(&dataFile);
    dataStream.setByteOrder(QDataStream::LittleEndian);
    dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    for (qsizetype timeIndex = 0; timeIndex < timeCount; ++timeIndex) {
        float timeValue = 0.0f;
        dataStream >> timeValue;
        if (dataStream.status() != QDataStream::Ok) {
            throw std::runtime_error("Failed to read time value from data file.");
        }
        project.m_timeValues[timeIndex] = timeValue;

        if (project.m_inMemory) {
            for (Field& field : project.m_fields) {
                const qsizetype sampleCount = field.sampleCount();
                float* destination = field.samples.data() + timeIndex * sampleCount;
                for (qsizetype sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
                    float value = 0.0f;
                    dataStream >> value;
                    if (dataStream.status() != QDataStream::Ok) {
                        throw std::runtime_error("Failed to read field data from data file.");
                    }
                    destination[sampleIndex] = value;
                    field.minimum = std::min(field.minimum, value);
                    field.maximum = std::max(field.maximum, value);
                }
            }
        } else {
            const qint64 skipBytes = static_cast<qint64>(project.m_fieldSampleTotal) * sizeof(float);
            if (!dataFile.seek(dataFile.pos() + skipBytes)) {
                throw std::runtime_error("Failed to skip data block in project file.");
            }
        }
    }

    if (project.m_inMemory) {
        dataFile.close();
    } else {
        project.m_dataFile = std::make_unique<QFile>(project.m_dataFilePath);
        if (!project.m_dataFile->open(QIODevice::ReadOnly)) {
            throw std::runtime_error(QStringLiteral("Failed to reopen data file: %1")
                                         .arg(project.m_dataFilePath)
                                         .toStdString());
        }
    }

    return project;
}

void Project::ensureDataFileOpen() const
{
    if (m_inMemory) {
        return;
    }

    if (!m_dataFile) {
        m_dataFile = std::make_unique<QFile>(m_dataFilePath);
    }

    if (!m_dataFile->isOpen()) {
        if (!m_dataFile->open(QIODevice::ReadOnly)) {
            throw std::runtime_error(QStringLiteral("Unable to open data file '%1'.")
                                         .arg(m_dataFilePath)
                                         .toStdString());
        }
    }
}

qint64 Project::fieldOffset(int fieldIndex) const
{
    qint64 offset = 1; // skip time value
    for (int i = 0; i < fieldIndex; ++i) {
        offset += static_cast<qint64>(m_fields[i].sampleCount());
    }
    return offset;
}

float Project::applyLogarithm(float value, bool logScale)
{
    if (!logScale) {
        return value;
    }
    if (value > 0.0f) {
        return std::log10(value);
    }
    return 0.0f;
}

void Project::readPoint(int fieldIndex, qsizetype startIndex, qsizetype step, qsizetype count,
                        QVector<float>& output, bool logScale) const
{
    if (fieldIndex < 0 || fieldIndex >= m_fields.size()) {
        throw std::out_of_range("Field index is out of range.");
    }

    const Field& field = m_fields[fieldIndex];
    if (count < 0) {
        throw std::invalid_argument("Count must be non-negative.");
    }

    output.resize(count);

    if (m_inMemory) {
        qsizetype index = startIndex;
        for (qsizetype i = 0; i < count; ++i) {
            if (index < 0 || index >= field.samples.size()) {
                throw std::out_of_range("Sample index is out of range.");
            }
            output[i] = applyLogarithm(field.samples.at(index), logScale);
            index += step;
        }
        return;
    }

    ensureDataFileOpen();

    const qint64 fieldSampleCount = static_cast<qint64>(field.sampleCount());
    if (fieldSampleCount <= 0) {
        throw std::runtime_error("Field has no samples.");
    }

    qint64 recordIndex = static_cast<qint64>(startIndex) / fieldSampleCount;
    qint64 pointOffset = static_cast<qint64>(startIndex) - fieldSampleCount * recordIndex;
    qint64 floatIndex = recordIndex * static_cast<qint64>(m_recordStride) + fieldOffset(fieldIndex) + pointOffset;

    if (!m_dataFile->seek(floatIndex * sizeof(float))) {
        throw std::runtime_error("Failed to seek to data position.");
    }

    const qint64 skipFloats = (step == fieldSampleCount) ? (static_cast<qint64>(m_recordStride) - 1)
                                                         : (static_cast<qint64>(step) - 1);

    for (qsizetype i = 0; i < count; ++i) {
        float value = 0.0f;
        const qint64 readBytes = m_dataFile->read(reinterpret_cast<char*>(&value), sizeof(float));
        if (readBytes != sizeof(float)) {
            throw std::runtime_error("Failed to read sample from data file.");
        }
        output[i] = applyLogarithm(value, logScale);

        if (i + 1 < count) {
            const qint64 targetPos = m_dataFile->pos() + skipFloats * static_cast<qint64>(sizeof(float));
            if (!m_dataFile->seek(targetPos)) {
                throw std::runtime_error("Failed to advance to next sample.");
            }
        }
    }
}

void Project::readPoints(int fieldIndex, qint64 startIndex, qint64 stepX1, int countX1,
                         qint64 stepX2, int countX2, QVector<float>& output, bool logScale) const
{
    if (fieldIndex < 0 || fieldIndex >= m_fields.size()) {
        throw std::out_of_range("Field index is out of range.");
    }
    if (countX1 < 0 || countX2 < 0) {
        throw std::invalid_argument("Point counts must be non-negative.");
    }

    const Field& field = m_fields[fieldIndex];
    output.resize(static_cast<qsizetype>(countX1) * static_cast<qsizetype>(countX2));

    if (m_inMemory) {
        qint64 cursor = startIndex;
        qsizetype index = 0;
        for (int j = 0; j < countX2; ++j) {
            for (int i = 0; i < countX1; ++i) {
                if (cursor < 0 || cursor >= field.samples.size()) {
                    throw std::out_of_range("Sample index is out of range.");
                }
                output[index++] = applyLogarithm(field.samples.at(static_cast<qsizetype>(cursor)), logScale);
                cursor += stepX1;
            }
            cursor += stepX2;
        }
        return;
    }

    ensureDataFileOpen();

    const qint64 fieldSampleCount = static_cast<qint64>(field.sampleCount());
    if (fieldSampleCount <= 0) {
        throw std::runtime_error("Field has no samples.");
    }

    qint64 recordIndex = startIndex / fieldSampleCount;
    qint64 pointOffset = startIndex - fieldSampleCount * recordIndex;
    qint64 floatIndex = recordIndex * static_cast<qint64>(m_recordStride) + fieldOffset(fieldIndex) + pointOffset;

    if (!m_dataFile->seek(floatIndex * sizeof(float))) {
        throw std::runtime_error("Failed to seek to data position.");
    }

    qint64 skipWithinRow;
    qint64 skipBetweenRows;

    if (stepX1 == fieldSampleCount) {
        skipWithinRow = static_cast<qint64>(m_recordStride) - 1;
        skipBetweenRows = -(static_cast<qint64>(countX1) * static_cast<qint64>(m_recordStride) - stepX2 -
                            static_cast<qint64>(countX1) * fieldSampleCount);
    } else {
        skipWithinRow = stepX1 - 1;
        if (stepX2 == (fieldSampleCount - static_cast<qint64>(countX1) * stepX1)) {
            skipBetweenRows = static_cast<qint64>(m_recordStride) - static_cast<qint64>(countX1) * stepX1;
        } else {
            skipBetweenRows = stepX2;
        }
    }

    qsizetype outIndex = 0;
    for (int j = 0; j < countX2; ++j) {
        for (int i = 0; i < countX1; ++i) {
            float value = 0.0f;
            const qint64 bytesRead = m_dataFile->read(reinterpret_cast<char*>(&value), sizeof(float));
            if (bytesRead != sizeof(float)) {
                throw std::runtime_error("Failed to read sample from data file.");
            }
            output[outIndex++] = applyLogarithm(value, logScale);

            if (i + 1 < countX1) {
                const qint64 targetPos = m_dataFile->pos() +
                                         skipWithinRow * static_cast<qint64>(sizeof(float));
                if (!m_dataFile->seek(targetPos)) {
                    throw std::runtime_error("Failed to advance within row.");
                }
            }
        }

        if (j + 1 < countX2) {
            const qint64 targetPos = m_dataFile->pos() +
                                     skipBetweenRows * static_cast<qint64>(sizeof(float));
            if (!m_dataFile->seek(targetPos)) {
                throw std::runtime_error("Failed to advance to next row.");
            }
        }
    }
}

bool Project::readGrid(const QString& gridFileName, QString* errorMessage)
{
    m_hasCells = false;
    m_cells.clear();
    m_xScale.clear();
    m_yScale.clear();
    m_zScale.clear();
    m_xCount = m_yCount = m_zCount = 0;
    m_layerFlags.fill(false);

    const QString gridPath = resolveRelativePath(m_projectPath, gridFileName);
    QFile gridFile(gridPath);
    if (!gridFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unable to open grid file '%1'.").arg(gridPath);
        }
        return false;
    }

    QTextStream stream(&gridFile);
    stream.setLocale(QLocale::c());

    // Skip header lines that are not used in calculations.
    stream.readLine();
    stream.readLine();
    stream.readLine();
    stream.readLine();

    QString token;
    stream >> token;
    if (token.compare(QLatin1String("X"), Qt::CaseInsensitive) != 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid grid file structure (missing X header).");
        }
        return false;
    }

    stream >> m_xCount;
    if (stream.status() != QTextStream::Ok || m_xCount <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid X dimension in grid file.");
        }
        return false;
    }
    m_xScale.resize(m_xCount + 1);
    for (int i = 0; i < m_xCount + 1; ++i) {
        stream >> m_xScale[i];
        if (stream.status() != QTextStream::Ok) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to read X scale values.");
            }
            return false;
        }
        if (i > 0 && m_xScale[i] <= m_xScale[i - 1]) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("X scale values must be strictly increasing.");
            }
            return false;
        }
    }

    stream >> token;
    if (token.compare(QLatin1String("Y"), Qt::CaseInsensitive) != 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid grid file structure (missing Y header).");
        }
        return false;
    }

    stream >> m_yCount;
    if (stream.status() != QTextStream::Ok || m_yCount <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid Y dimension in grid file.");
        }
        return false;
    }
    m_yScale.resize(m_yCount + 1);
    for (int i = 0; i < m_yCount + 1; ++i) {
        stream >> m_yScale[i];
        if (stream.status() != QTextStream::Ok) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to read Y scale values.");
            }
            return false;
        }
        if (i > 0 && m_yScale[i] <= m_yScale[i - 1]) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Y scale values must be strictly increasing.");
            }
            return false;
        }
    }

    stream >> token;
    if (token.compare(QLatin1String("Z"), Qt::CaseInsensitive) != 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid grid file structure (missing Z header).");
        }
        return false;
    }

    stream >> m_zCount;
    if (stream.status() != QTextStream::Ok || m_zCount <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid Z dimension in grid file.");
        }
        return false;
    }
    m_zScale.resize(m_zCount + 1);
    for (int i = 0; i < m_zCount + 1; ++i) {
        stream >> m_zScale[i];
        if (stream.status() != QTextStream::Ok) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to read Z scale values.");
            }
            return false;
        }
        if (i > 0 && m_zScale[i] <= m_zScale[i - 1]) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Z scale values must be strictly increasing.");
            }
            return false;
        }
    }

    gridFile.close();

    const QString cellPath = resolveRelativePath(m_projectPath, QStringLiteral("data.cel"));
    QFile cellFile(cellPath);
    if (!cellFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unable to open cell file '%1'.").arg(cellPath);
        }
        return false;
    }

    QTextStream cellStream(&cellFile);
    cellStream.setLocale(QLocale::c());

    m_cells.resize(m_xCount * m_yCount * m_zCount);

    int currentLayer = 0;
    int remaining = 0;
    for (int x = 0; x < m_xCount; ++x) {
        for (int y = 0; y < m_yCount; ++y) {
            for (int z = 0; z < m_zCount; ++z) {
                while (remaining <= 0) {
                    cellStream >> currentLayer;
                    cellStream >> remaining;
                    if (cellStream.status() != QTextStream::Ok) {
                        if (errorMessage) {
                            *errorMessage = QStringLiteral("Unexpected end of cell file '%1'.").arg(cellPath);
                        }
                        return false;
                    }
                    if (currentLayer >= 0 && currentLayer < kMaxLayers) {
                        m_layerFlags[currentLayer] = true;
                    }
                }

                const qsizetype index = (static_cast<qsizetype>(x) * m_yCount + y) * m_zCount + z;
                m_cells[index] = currentLayer;
                --remaining;
            }
        }
    }

    m_hasCells = true;
    return true;
}

int Project::layerForPosition(double x, double y, double z) const
{
    if (!m_hasCells || m_xScale.isEmpty() || m_yScale.isEmpty() || m_zScale.isEmpty()) {
        return 0;
    }

    const auto findIndex = [](const QVector<double>& scale, double value) -> int {
        if (value > scale.back() || value < scale.front()) {
            return -1;
        }
        int minIndex = 0;
        int maxIndex = scale.size() - 1;
        while (maxIndex - minIndex > 1) {
            const int mid = minIndex + (maxIndex - minIndex) / 2;
            if (scale[mid] > value) {
                maxIndex = mid;
            } else {
                minIndex = mid;
            }
        }
        return minIndex;
    };

    const int xIndex = findIndex(m_xScale, x);
    const int yIndex = findIndex(m_yScale, y);
    const int zIndex = findIndex(m_zScale, z);

    if (xIndex < 0 || yIndex < 0 || zIndex < 0) {
        return 0;
    }

    const qsizetype index = (static_cast<qsizetype>(xIndex) * m_yCount + yIndex) * m_zCount + zIndex;
    if (index < 0 || index >= m_cells.size()) {
        return 0;
    }
    return m_cells.at(index);
}

}  // namespace video

