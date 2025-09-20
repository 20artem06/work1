#pragma once

#include <array>
#include <limits>
#include <memory>
#include <vector>

#include <QColor>
#include <QFile>
#include <QString>
#include <QVector>

namespace video {

constexpr int kMaxLayers = 20;
constexpr qint64 kMemMaxBytes = 4'000'000;
constexpr int kTabMaxRecords = 32735;

struct Axis {
    QString name;
    QString unit;
    QVector<float> points;

    qsizetype pointCount() const noexcept { return points.size(); }
};

class Field {
public:
    QString name;
    QString unit;
    QVector<Axis> axes;
    QVector<float> samples;
    float minimum = std::numeric_limits<float>::max();
    float maximum = std::numeric_limits<float>::lowest();

    qsizetype sampleCount() const noexcept { return m_sampleCount; }
    void setSampleCount(qsizetype value) noexcept { m_sampleCount = value; }

private:
    qsizetype m_sampleCount = 1;
};

class Project {
public:
    Project();
    ~Project();

    Project(Project&&) noexcept;
    Project& operator=(Project&&) noexcept;

    Project(const Project&) = delete;
    Project& operator=(const Project&) = delete;

    static Project loadFromDescription(const QString& descriptionPath);

    const QString& name() const noexcept { return m_name; }
    void setName(const QString& name) { m_name = name; }

    const QString& timeName() const noexcept { return m_timeName; }
    void setTimeName(const QString& timeName) { m_timeName = timeName; }

    const QString& timeUnit() const noexcept { return m_timeUnit; }
    void setTimeUnit(const QString& timeUnit) { m_timeUnit = timeUnit; }

    const QVector<float>& timeValues() const noexcept { return m_timeValues; }
    QVector<float>& timeValues() noexcept { return m_timeValues; }

    bool isInMemory() const noexcept { return m_inMemory; }
    bool isOneDimOnly() const noexcept { return m_oneDimOnly; }

    QVector<Field>& fields() noexcept { return m_fields; }
    const QVector<Field>& fields() const noexcept { return m_fields; }

    qsizetype fieldSampleTotal() const noexcept { return m_fieldSampleTotal; }
    qsizetype recordStride() const noexcept { return m_recordStride; }

    const QString& projectPath() const noexcept { return m_projectPath; }
    void setProjectPath(const QString& path);

    const QString& dataFilePath() const noexcept { return m_dataFilePath; }
    void setDataFilePath(const QString& path);

    bool hasCells() const noexcept { return m_hasCells; }
    const std::array<bool, kMaxLayers>& layerFlags() const noexcept { return m_layerFlags; }

    const QVector<double>& xScale() const noexcept { return m_xScale; }
    const QVector<double>& yScale() const noexcept { return m_yScale; }
    const QVector<double>& zScale() const noexcept { return m_zScale; }

    int layerForPosition(double x, double y, double z) const;

    bool readGrid(const QString& gridFileName, QString* errorMessage = nullptr);

    void readPoint(int fieldIndex, qsizetype startIndex, qsizetype step, qsizetype count,
                   QVector<float>& output, bool logScale) const;
    void readPoints(int fieldIndex, qint64 startIndex, qint64 stepX1, int countX1,
                    qint64 stepX2, int countX2, QVector<float>& output, bool logScale) const;

private:
    void ensureDataFileOpen() const;
    qint64 fieldOffset(int fieldIndex) const;
    static float applyLogarithm(float value, bool logScale);

    QString m_name;
    QString m_timeName;
    QString m_timeUnit;
    QString m_projectPath;
    QString m_dataFilePath;

    QVector<float> m_timeValues;
    QVector<Field> m_fields;

    qsizetype m_fieldSampleTotal = 0;
    qsizetype m_recordStride = 0; // includes time value

    bool m_inMemory = true;
    bool m_oneDimOnly = true;

    mutable std::unique_ptr<QFile> m_dataFile;

    bool m_hasCells = false;
    std::array<bool, kMaxLayers> m_layerFlags;
    QVector<int> m_cells; // flattened X x Y x Z grid
    int m_xCount = 0;
    int m_yCount = 0;
    int m_zCount = 0;
    QVector<double> m_xScale;
    QVector<double> m_yScale;
    QVector<double> m_zScale;
};

}  // namespace video

