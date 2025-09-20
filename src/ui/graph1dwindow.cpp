#include "graph1dwindow.h"

#include <algorithm>
#include <limits>

#include <QPainter>
#include <QPolygonF>
#include <QStringList>
#include <QVBoxLayout>

#include "core/math_utils.h"
#include "core/project.h"

namespace {
class Graph1DPlotWidget : public QWidget
{
public:
    struct Series {
        QPolygonF polyline;
        QColor color;
        Qt::PenStyle style = Qt::SolidLine;
        QString name;
        float minValue = 0.0f;
        float maxValue = 0.0f;
    };

    explicit Graph1DPlotWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAutoFillBackground(true);
    }

    void setData(const QVector<Series> &series, bool showGrid, double xMin, double xMax,
                 double yMin, double yMax, const QString &xLabel, const QString &yLabel)
    {
        m_series = series;
        m_showGrid = showGrid;
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
        m_xLabel = xLabel;
        m_yLabel = yLabel;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), palette().window());
        painter.setRenderHint(QPainter::Antialiasing, true);

        if (m_series.isEmpty()) {
            painter.setPen(palette().text().color());
            painter.drawText(rect(), Qt::AlignCenter, QObject::tr("No data"));
            return;
        }

        const QRectF plotArea = rect().adjusted(70, 20, -180, -70);
        painter.setPen(palette().mid().color());
        painter.drawRect(plotArea);

        if (m_showGrid) {
            painter.setPen(QPen(palette().midlight().color(), 1, Qt::DashLine));
            const int gridLines = 6;
            for (int i = 1; i < gridLines; ++i) {
                const qreal ratio = static_cast<qreal>(i) / gridLines;
                const qreal x = plotArea.left() + ratio * plotArea.width();
                painter.drawLine(QPointF(x, plotArea.top()), QPointF(x, plotArea.bottom()));
                const qreal y = plotArea.bottom() - ratio * plotArea.height();
                painter.drawLine(QPointF(plotArea.left(), y), QPointF(plotArea.right(), y));
            }
        }

        painter.setPen(palette().text().color());
        painter.drawText(QRectF(plotArea.left(), plotArea.bottom() + 8, plotArea.width(), 40),
                         Qt::AlignCenter, m_xLabel);
        painter.save();
        painter.translate(plotArea.left() - 40, plotArea.center().y());
        painter.rotate(-90);
        painter.drawText(QRectF(-plotArea.height() / 2, -40, plotArea.height(), 40), Qt::AlignCenter, m_yLabel);
        painter.restore();

        for (const Series &series : m_series) {
            if (series.polyline.isEmpty()) {
                continue;
            }
            QPolygonF mapped;
            mapped.reserve(series.polyline.size());
            for (const QPointF &point : series.polyline) {
                const qreal xRatio = (point.x() - m_xMin) / (m_xMax - m_xMin);
                const qreal yRatio = (point.y() - m_yMin) / (m_yMax - m_yMin);
                const qreal px = plotArea.left() + xRatio * plotArea.width();
                const qreal py = plotArea.bottom() - yRatio * plotArea.height();
                mapped.append(QPointF(px, py));
            }
            painter.setPen(QPen(series.color, 2, series.style, Qt::RoundCap, Qt::RoundJoin));
            painter.drawPolyline(mapped);
        }

        // Legend
        painter.setPen(palette().text().color());
        const QRectF legendRect(plotArea.right() + 20, plotArea.top(), 150, plotArea.height());
        painter.drawText(legendRect.adjusted(0, 0, 0, -legendRect.height() + 20), Qt::AlignLeft | Qt::AlignTop,
                         QObject::tr("Legend"));
        qreal legendY = legendRect.top() + 30;
        const qreal legendLineWidth = 24;
        for (const Series &series : m_series) {
            painter.setPen(QPen(series.color, 2, series.style));
            painter.drawLine(QPointF(legendRect.left(), legendY),
                             QPointF(legendRect.left() + legendLineWidth, legendY));
            painter.setPen(palette().text().color());
            painter.drawText(QRectF(legendRect.left() + legendLineWidth + 6, legendY - 10,
                                    legendRect.width() - legendLineWidth - 6, 20),
                             Qt::AlignLeft | Qt::AlignVCenter, series.name);
            legendY += 24;
        }

        const QString rangeText = QObject::tr("X: %1 – %2\nY: %3 – %4")
                                      .arg(m_xMin, 0, 'f', 3)
                                      .arg(m_xMax, 0, 'f', 3)
                                      .arg(m_yMin, 0, 'f', 3)
                                      .arg(m_yMax, 0, 'f', 3);
        painter.drawText(QRectF(plotArea.left(), plotArea.top() - 40, plotArea.width(), 32),
                         Qt::AlignCenter, rangeText);
    }

private:
    QVector<Series> m_series;
    bool m_showGrid = false;
    double m_xMin = 0.0;
    double m_xMax = 1.0;
    double m_yMin = 0.0;
    double m_yMax = 1.0;
    QString m_xLabel;
    QString m_yLabel;
};

QString defaultSeriesName(const video::VideoSession1Dim &state, int index)
{
    if (!state.name.isEmpty()) {
        return state.name;
    }
    return QObject::tr("Series %1").arg(index + 1);
}

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

} // namespace

Graph1DWindow::Graph1DWindow(const video::OneDimWindowState &state, QWidget *parent)
    : QWidget(parent)
    , m_state(state)
{
    setAttribute(Qt::WA_DeleteOnClose);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_plotWidget = new Graph1DPlotWidget(this);
    layout->addWidget(m_plotWidget);

    rebuildData();
}

void Graph1DWindow::setWindowState(const video::OneDimWindowState &state)
{
    m_state = state;
    rebuildData();
}

void Graph1DWindow::ensureSeriesIndices(video::VideoSession1Dim &session, const video::Field &field) const
{
    const int required = field.axes.size() + 1; // time + axes
    if (session.indices.size() < required) {
        session.indices.resize(required);
    }
    for (int axis = 0; axis < field.axes.size(); ++axis) {
        const int idx = session.indices.value(axis + 1);
        const int maxIndex = field.axes.at(axis).points.size() - 1;
        session.indices[axis + 1] = std::clamp(idx, 0, std::max(0, maxIndex));
    }
    session.argumentIndex = std::clamp(session.argumentIndex, 0, std::max(0, field.axes.size() - 1));
}

void Graph1DWindow::rebuildData()
{
    m_series.clear();
    if (m_state.graphs.isEmpty()) {
        updatePlot();
        updateWindowTitle();
        return;
    }

    m_series.reserve(m_state.graphs.size());

    for (int index = 0; index < m_state.graphs.size(); ++index) {
        video::VideoSession1Dim &graph = m_state.graphs[index];
        video::Project *project = graph.project;
        if (!project) {
            continue;
        }
        if (graph.fieldIndex < 0 || graph.fieldIndex >= project->fields().size()) {
            continue;
        }
        const video::Field &field = project->fields().at(graph.fieldIndex);
        if (field.axes.isEmpty()) {
            continue;
        }

        ensureSeriesIndices(graph, field);

        const int axisIndex = graph.argumentIndex;
        const video::Axis &axis = field.axes.at(axisIndex);
        const int axisCount = axis.points.size();
        if (axisCount <= 0) {
            continue;
        }

        const int startIndex = std::clamp(graph.xMin, 0, axisCount - 1);
        const int endIndex = std::clamp(graph.xMax, startIndex, axisCount - 1);
        const int sampleCount = endIndex - startIndex + 1;

        const qsizetype fieldSampleCount = field.sampleCount();
        QVector<qsizetype> strides(field.axes.size());
        qsizetype stride = 1;
        for (int axisPos = field.axes.size() - 1; axisPos >= 0; --axisPos) {
            strides[axisPos] = stride;
            stride *= field.axes[axisPos].points.size();
        }

        qsizetype fixedOffset = 0;
        for (int axisPos = 0; axisPos < field.axes.size(); ++axisPos) {
            if (axisPos == axisIndex) {
                continue;
            }
            const int idx = graph.indices.value(axisPos + 1, 0);
            fixedOffset += strides[axisPos] * static_cast<qsizetype>(std::clamp(idx, 0, field.axes.at(axisPos).points.size() - 1));
        }

        const int timeIndex = std::clamp(graph.indices.value(0, 0), 0, project->timeValues().size() - 1);
        const qsizetype baseIndex = static_cast<qsizetype>(timeIndex) * fieldSampleCount + fixedOffset +
                                    strides[axisIndex] * static_cast<qsizetype>(startIndex);

        QVector<float> values;
        values.resize(sampleCount);
        try {
            project->readPoint(graph.fieldIndex, baseIndex, strides[axisIndex], sampleCount, values, graph.logarithmic);
        } catch (const std::exception &) {
            continue;
        }

        SeriesData series;
        series.state = graph;
        series.points.reserve(sampleCount);
        series.minValue = std::numeric_limits<float>::max();
        series.maxValue = std::numeric_limits<float>::lowest();

        for (int i = 0; i < sampleCount; ++i) {
            const float value = values.at(i);
            const float x = axis.points.at(startIndex + i);
            series.points.append(QPointF(x, value));
            series.minValue = std::min(series.minValue, value);
            series.maxValue = std::max(series.maxValue, value);
        }

        series.xLabel = axisLabel(axis);
        series.yLabel = fieldLabel(field, graph.fieldIndex);

        series.state.minValue = series.minValue;
        series.state.maxValue = series.maxValue;
        series.state.xMin = startIndex;
        series.state.xMax = endIndex;
        series.state.indices[0] = timeIndex;

        m_series.append(series);
        graph = series.state;
    }

    updatePlot();
    updateWindowTitle();
}

void Graph1DWindow::updatePlot()
{
    if (!m_plotWidget) {
        return;
    }

    QVector<Graph1DPlotWidget::Series> visuals;
    visuals.reserve(m_series.size());

    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    for (int i = 0; i < m_series.size(); ++i) {
        const SeriesData &series = m_series.at(i);
        Graph1DPlotWidget::Series visual;
        visual.name = defaultSeriesName(series.state, i);
        visual.color = series.state.color;
        const int styleIndex = std::clamp(series.state.style, 0, static_cast<int>(video::kPenStyles.size()) - 1);
        visual.style = video::kPenStyles[styleIndex];
        visual.minValue = series.minValue;
        visual.maxValue = series.maxValue;
        visual.polyline = QPolygonF(series.points);
        visuals.append(visual);

        for (const QPointF &point : series.points) {
            xMin = std::min<double>(xMin, point.x());
            xMax = std::max<double>(xMax, point.x());
            yMin = std::min<double>(yMin, point.y());
            yMax = std::max<double>(yMax, point.y());
        }
    }

    if (visuals.isEmpty()) {
        m_plotWidget->setData({}, false, 0.0, 1.0, 0.0, 1.0, tr("Axis"), tr("Value"));
        return;
    }

    if (xMin == xMax) {
        xMin -= 1.0;
        xMax += 1.0;
    }
    if (yMin == yMax) {
        yMin -= 1.0;
        yMax += 1.0;
    }

    double effectiveXMin = xMin;
    double effectiveXMax = xMax;
    double effectiveYMin = yMin;
    double effectiveYMax = yMax;

    switch (m_state.axisStatus) {
    case 0: // fixed
        effectiveXMin = m_state.x1Min;
        effectiveXMax = m_state.x1Max;
        effectiveYMin = m_state.x2Min;
        effectiveYMax = m_state.x2Max;
        break;
    case 2: // copy from first series
        effectiveXMin = visuals.first().polyline.first().x();
        effectiveXMax = visuals.first().polyline.last().x();
        effectiveYMin = m_series.first().minValue;
        effectiveYMax = m_series.first().maxValue;
        break;
    default:
        break;
    }

    if (effectiveXMin == effectiveXMax) {
        effectiveXMin -= 1.0;
        effectiveXMax += 1.0;
    }
    if (effectiveYMin == effectiveYMax) {
        effectiveYMin -= 1.0;
        effectiveYMax += 1.0;
    }

    bool showGrid = false;
    for (const SeriesData &series : m_series) {
        showGrid = showGrid || series.state.gridLines;
    }

    const QString xLabel = m_series.first().xLabel;
    const QString yLabel = m_series.first().yLabel;

    m_plotWidget->setData(visuals, showGrid, effectiveXMin, effectiveXMax,
                          effectiveYMin, effectiveYMax, xLabel, yLabel);
}

void Graph1DWindow::updateWindowTitle()
{
    if (m_series.isEmpty()) {
        setWindowTitle(tr("1D Graph"));
        return;
    }

    QStringList titles;
    titles.reserve(m_series.size());
    for (int i = 0; i < m_series.size(); ++i) {
        const SeriesData &series = m_series.at(i);
        const video::Project *project = series.state.project;
        QString projectName = project ? project->name() : QString();
        if (projectName.isEmpty()) {
            projectName = tr("Project");
        }
        titles.append(QStringLiteral("%1 — %2").arg(projectName, defaultSeriesName(series.state, i)));
    }

    setWindowTitle(titles.join(QStringLiteral(", ")));
}

QVector<Graph1DWindow::ExportSeries> Graph1DWindow::exportSeries() const
{
    QVector<ExportSeries> result;
    result.reserve(m_series.size());
    for (int i = 0; i < m_series.size(); ++i) {
        const SeriesData &series = m_series.at(i);
        ExportSeries exportSeries;
        exportSeries.name = defaultSeriesName(series.state, i);
        exportSeries.yLabel = series.yLabel;
        exportSeries.points = series.points;
        result.append(exportSeries);
    }
    return result;
}

QString Graph1DWindow::xAxisLabel() const
{
    if (m_series.isEmpty()) {
        return tr("Axis");
    }
    return m_series.first().xLabel;
}

