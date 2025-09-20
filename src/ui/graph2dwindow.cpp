#include "graph2dwindow.h"

#include <algorithm>
#include <limits>

#include <QPainter>
#include <QVBoxLayout>

#include "core/math_utils.h"
#include "core/project.h"

namespace {

QColor interpolateColor(const QColor &start, const QColor &end, float ratio);
class Graph2DWidget : public QWidget
{
public:
    explicit Graph2DWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAutoFillBackground(true);
    }

    void setData(const QImage &image, const QVector<double> &xAxis, const QVector<double> &yAxis,
                 float minValue, float maxValue, bool showGrid, const QString &xLabel,
                 const QString &yLabel, const QColor &colorMin, const QColor &colorMax)
    {
        m_image = image;
        m_xAxis = xAxis;
        m_yAxis = yAxis;
        m_minValue = minValue;
        m_maxValue = maxValue;
        m_showGrid = showGrid;
        m_xLabel = xLabel;
        m_yLabel = yLabel;
        m_colorMin = colorMin;
        m_colorMax = colorMax;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), palette().window());
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        if (m_image.isNull()) {
            painter.setPen(palette().text().color());
            painter.drawText(rect(), Qt::AlignCenter, QObject::tr("No data"));
            return;
        }

        const QRectF plotArea = rect().adjusted(80, 20, -160, -70);
        painter.setPen(palette().mid().color());
        painter.drawRect(plotArea);
        painter.drawImage(plotArea, m_image);

        if (m_showGrid && !m_xAxis.isEmpty() && !m_yAxis.isEmpty()) {
            painter.setPen(QPen(palette().midlight().color(), 1, Qt::DashLine));
            for (int i = 1; i < m_xAxis.size(); ++i) {
                const qreal ratio = static_cast<qreal>(i) / m_xAxis.size();
                const qreal x = plotArea.left() + ratio * plotArea.width();
                painter.drawLine(QPointF(x, plotArea.top()), QPointF(x, plotArea.bottom()));
            }
            for (int j = 1; j < m_yAxis.size(); ++j) {
                const qreal ratio = static_cast<qreal>(j) / m_yAxis.size();
                const qreal y = plotArea.bottom() - ratio * plotArea.height();
                painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
            }
        }

        painter.setPen(palette().text().color());
        painter.drawText(QRectF(plotArea.left(), plotArea.bottom() + 4, plotArea.width(), 30),
                         Qt::AlignCenter, m_xLabel);
        painter.save();
        painter.translate(plotArea.left() - 40, plotArea.center().y());
        painter.rotate(-90);
        painter.drawText(QRectF(-plotArea.height() / 2, -40, plotArea.height(), 40), Qt::AlignCenter, m_yLabel);
        painter.restore();

        const QRectF legendRect(plotArea.right() + 20, plotArea.top(), 120, plotArea.height());
        painter.drawText(legendRect.adjusted(0, 0, 0, -legendRect.height() + 20), Qt::AlignLeft | Qt::AlignTop,
                         QObject::tr("Range"));
        painter.drawText(QRectF(legendRect.left(), legendRect.top() + 30, legendRect.width(), 40), Qt::AlignLeft,
                         QObject::tr("Min: %1\nMax: %2").arg(m_minValue, 0, 'f', 3).arg(m_maxValue, 0, 'f', 3));

        if (!m_image.isNull()) {
            const QRectF barRect(legendRect.left(), legendRect.top() + 80, 24, legendRect.height() - 120);
            for (int y = 0; y < barRect.height(); ++y) {
                const qreal ratio = 1.0 - static_cast<qreal>(y) / barRect.height();
                const QColor color = interpolateColor(m_colorMin, m_colorMax, static_cast<float>(ratio));
                painter.fillRect(QRectF(barRect.left(), barRect.top() + y, barRect.width(), 1), color);
            }
        }
    }

private:
    QImage m_image;
    QVector<double> m_xAxis;
    QVector<double> m_yAxis;
    float m_minValue = 0.0f;
    float m_maxValue = 0.0f;
    bool m_showGrid = false;
    QString m_xLabel;
    QString m_yLabel;
    QColor m_colorMin;
    QColor m_colorMax;
};

QString axisLabel(const video::Axis &axis)
{
    QString label = axis.name;
    if (!axis.unit.isEmpty()) {
        label += QObject::tr(" (%1)").arg(axis.unit);
    }
    if (label.isEmpty()) {
        label = QObject::tr("Axis");
    }
    return label;
}

QString fieldLabel(const video::Field &field, int index)
{
    QString name = field.name;
    if (name.isEmpty()) {
        name = QObject::tr("Field %1").arg(index + 1);
    }
    if (!field.unit.isEmpty()) {
        name += QObject::tr(" (%1)").arg(field.unit);
    }
    return name;
}

QColor interpolateColor(const QColor &start, const QColor &end, float ratio)
{
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    const int r = static_cast<int>(start.red() + ratio * (end.red() - start.red()));
    const int g = static_cast<int>(start.green() + ratio * (end.green() - start.green()));
    const int b = static_cast<int>(start.blue() + ratio * (end.blue() - start.blue()));
    return QColor(r, g, b);
}

} // namespace

Graph2DWindow::Graph2DWindow(const video::VideoSession2Dim &state, QWidget *parent)
    : QWidget(parent)
    , m_state(state)
{
    setAttribute(Qt::WA_DeleteOnClose);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_widget = new Graph2DWidget(this);
    layout->addWidget(m_widget);

    rebuildImage();
}

void Graph2DWindow::setWindowState(const video::VideoSession2Dim &state)
{
    m_state = state;
    rebuildImage();
}

void Graph2DWindow::rebuildImage()
{
    if (!m_state.project || m_state.fieldIndex < 0 || m_state.fieldIndex >= m_state.project->fields().size()) {
        m_values.clear();
        m_widget->setData(QImage(), {}, {}, 0.0f, 0.0f, false, tr("X"), tr("Y"), QColor(), QColor());
        updateWindowTitle();
        return;
    }

    video::Project *project = m_state.project;
    const video::Field &field = project->fields().at(m_state.fieldIndex);
    if (field.axes.size() < 2) {
        m_values.clear();
        m_widget->setData(QImage(), {}, {}, 0.0f, 0.0f, false, tr("X"), tr("Y"), QColor(), QColor());
        updateWindowTitle();
        return;
    }

    const int axisX = std::clamp(m_state.argument1, 0, field.axes.size() - 1);
    const int axisY = std::clamp(m_state.argument2, 0, field.axes.size() - 1);
    const video::Axis &xAxis = field.axes.at(axisX);
    const video::Axis &yAxis = field.axes.at(axisY);

    const int xStart = std::clamp(m_state.x1Min, 0, xAxis.points.size() - 1);
    const int xEnd = std::clamp(m_state.x1Max, xStart, xAxis.points.size() - 1);
    const int yStart = std::clamp(m_state.x2Min, 0, yAxis.points.size() - 1);
    const int yEnd = std::clamp(m_state.x2Max, yStart, yAxis.points.size() - 1);

    const int width = xEnd - xStart + 1;
    const int height = yEnd - yStart + 1;
    if (width <= 0 || height <= 0) {
        m_values.clear();
        m_widget->setData(QImage(), {}, {}, 0.0f, 0.0f, false, tr("X"), tr("Y"), QColor(), QColor());
        updateWindowTitle();
        return;
    }

    const qsizetype fieldSampleCount = field.sampleCount();
    QVector<qsizetype> strides(field.axes.size());
    qsizetype stride = 1;
    for (int axis = field.axes.size() - 1; axis >= 0; --axis) {
        strides[axis] = stride;
        stride *= field.axes[axis].points.size();
    }

    if (m_state.indices.size() < field.axes.size() + 1) {
        m_state.indices.resize(field.axes.size() + 1);
    }

    const int maxTimeIndex = project->timeValues().isEmpty() ? 0 : project->timeValues().size() - 1;
    const int timeIndex = std::clamp(m_state.indices.value(0, 0), 0, maxTimeIndex);

    qsizetype baseIndex = static_cast<qsizetype>(timeIndex) * fieldSampleCount;
    for (int axis = 0; axis < field.axes.size(); ++axis) {
        if (axis == axisX || axis == axisY) {
            continue;
        }
        const int idx = std::clamp(m_state.indices.value(axis + 1, 0), 0, field.axes.at(axis).points.size() - 1);
        baseIndex += strides[axis] * static_cast<qsizetype>(idx);
    }
    baseIndex += strides[axisY] * static_cast<qsizetype>(yStart);

    QVector<float> values(width * height);
    QVector<float> row(width);
    int out = 0;
    for (int rowIndex = 0; rowIndex < height; ++rowIndex) {
        const qsizetype rowBase = baseIndex + static_cast<qsizetype>(rowIndex) * strides[axisY] +
                                  strides[axisX] * static_cast<qsizetype>(xStart);
        project->readPoint(m_state.fieldIndex, rowBase, strides[axisX], width, row, m_state.logarithmic);
        std::copy(row.constBegin(), row.constEnd(), values.begin() + out);
        out += width;
    }

    m_minValue = *std::min_element(values.constBegin(), values.constEnd());
    m_maxValue = *std::max_element(values.constBegin(), values.constEnd());
    if (m_minValue == m_maxValue) {
        m_minValue -= 1.0f;
        m_maxValue += 1.0f;
    }

    m_image = QImage(width, height, QImage::Format_RGB32);
    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(m_image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            const float value = values.at(y * width + x);
            const float ratio = (value - m_minValue) / (m_maxValue - m_minValue);
            const QColor color = interpolateColor(m_state.colorMin, m_state.colorMax, ratio);
            line[x] = color.rgb();
        }
    }
    m_image = m_image.mirrored(false, true);

    m_xAxis = QVector<double>(xAxis.points.begin() + xStart, xAxis.points.begin() + xEnd + 1);
    m_yAxis = QVector<double>(yAxis.points.begin() + yStart, yAxis.points.begin() + yEnd + 1);

    const QString xLabel = axisLabel(xAxis);
    const QString yLabel = axisLabel(yAxis);
    m_values = values;

    m_widget->setData(m_image, m_xAxis, m_yAxis, m_minValue, m_maxValue,
                      m_state.gridLines, xLabel, yLabel, m_state.colorMin, m_state.colorMax);

    m_state.minValue = m_minValue;
    m_state.maxValue = m_maxValue;
    m_state.x1Min = xStart;
    m_state.x1Max = xEnd;
    m_state.x2Min = yStart;
    m_state.x2Max = yEnd;
    if (m_state.indices.isEmpty()) {
        m_state.indices.append(timeIndex);
    } else {
        m_state.indices[0] = timeIndex;
    }

    updateWindowTitle();
}

void Graph2DWindow::updateWindowTitle()
{
    if (!m_state.project) {
        setWindowTitle(tr("2D Graph"));
        return;
    }
    const video::Field &field = m_state.project->fields().at(m_state.fieldIndex);
    QString fieldName = fieldLabel(field, m_state.fieldIndex);
    QString projectName = m_state.project->name();
    if (projectName.isEmpty()) {
        projectName = tr("Project");
    }

    const int timeIndex = m_state.indices.isEmpty() ? 0 : m_state.indices.first();
    QString timeText;
    if (!m_state.project->timeValues().isEmpty() && timeIndex < m_state.project->timeValues().size()) {
        timeText = QString::number(m_state.project->timeValues().at(timeIndex), 'f', 3);
    } else {
        timeText = QString::number(timeIndex);
    }

    setWindowTitle(QStringLiteral("%1 — %2 (t = %3)").arg(projectName, fieldName, timeText));
}

