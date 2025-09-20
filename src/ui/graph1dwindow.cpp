#include "graph1dwindow.h"

#include <QPainter>
#include <algorithm>
#include <exception>
#include <limits>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "core/project.h"

namespace {
class Graph1DWidget : public QWidget
{
public:
    explicit Graph1DWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAutoFillBackground(true);
    }

    void setData(const QVector<QPointF> &points, const QString &xLabel, const QString &yLabel,
                 const QColor &color, bool grid, float minValue, float maxValue)
    {
        m_points = points;
        m_xLabel = xLabel;
        m_yLabel = yLabel;
        m_color = color;
        m_showGrid = grid;
        m_minValue = minValue;
        m_maxValue = maxValue;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.fillRect(rect(), palette().window());

        if (m_points.size() < 2) {
            painter.setPen(palette().text().color());
            painter.drawText(rect(), Qt::AlignCenter, tr("No data"));
            return;
        }

        const QRectF plotArea = rect().adjusted(60, 20, -20, -60);
        painter.setPen(palette().mid().color());
        painter.drawRect(plotArea);

        double xMin = m_points.first().x();
        double xMax = m_points.last().x();
        double yMin = m_minValue;
        double yMax = m_maxValue;
        if (qFuzzyCompare(yMin, yMax)) {
            yMin -= 1.0;
            yMax += 1.0;
        }

        if (m_showGrid) {
            painter.setPen(QPen(palette().midlight().color(), 1, Qt::DashLine));
            const int gridLines = 5;
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
        painter.translate(20, plotArea.center().y());
        painter.rotate(-90);
        painter.drawText(QRectF(-plotArea.height() / 2, -40, plotArea.height(), 40), Qt::AlignCenter, m_yLabel);
        painter.restore();

        painter.setPen(QPen(m_color, 2));
        QPolygonF polyline;
        polyline.reserve(m_points.size());
        for (const QPointF &point : m_points) {
            const qreal xRatio = (point.x() - xMin) / (xMax - xMin);
            const qreal yRatio = (point.y() - yMin) / (yMax - yMin);
            const qreal px = plotArea.left() + xRatio * plotArea.width();
            const qreal py = plotArea.bottom() - yRatio * plotArea.height();
            polyline.append(QPointF(px, py));
        }
        painter.drawPolyline(polyline);

        painter.setPen(palette().text().color());
        painter.drawText(rect().adjusted(0, 0, -10, -rect().height() + 20), Qt::AlignRight | Qt::AlignTop,
                         tr("Min: %1\nMax: %2").arg(m_minValue, 0, 'f', 3).arg(m_maxValue, 0, 'f', 3));
    }

private:
    QVector<QPointF> m_points;
    QString m_xLabel;
    QString m_yLabel;
    QColor m_color = QColor(0, 0, 255);
    bool m_showGrid = false;
    float m_minValue = 0.0f;
    float m_maxValue = 0.0f;
};

} // namespace

Graph1DWindow::Graph1DWindow(const Config &config, QWidget *parent)
    : QWidget(parent)
    , m_config(config)
{
    setAttribute(Qt::WA_DeleteOnClose);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_widget = new Graph1DWidget(this);
    layout->addWidget(m_widget);

    populateData();
    updateWindowTitle();
}

void Graph1DWindow::populateData()
{
    m_points.clear();
    if (!m_config.project || m_config.fieldIndex < 0 || m_config.fieldIndex >= m_config.project->fields().size()) {
        return;
    }

    const video::Field &field = m_config.project->fields()[m_config.fieldIndex];
    if (field.axes.isEmpty()) {
        return;
    }

    const video::Axis &axis = field.axes.first();
    const qsizetype axisCount = axis.points.size();
    if (axisCount == 0) {
        return;
    }

    const qsizetype samplesPerTime = field.sampleCount();
    const qsizetype totalTimes = m_config.project->timeValues().size();
    if (totalTimes <= 0) {
        return;
    }
    const qsizetype timeIndex = std::clamp<qsizetype>(static_cast<qsizetype>(m_config.timeIndex), 0, totalTimes - 1);
    const qsizetype startIndex = timeIndex * samplesPerTime;

    QVector<float> samples;
    try {
        m_config.project->readPoint(m_config.fieldIndex, startIndex, 1, axisCount, samples, m_config.logarithmic);
    } catch (const std::exception &) {
        samples.clear();
    }

    if (samples.size() != axisCount) {
        return;
    }

    m_points.reserve(axisCount);
    m_minValue = std::numeric_limits<float>::max();
    m_maxValue = std::numeric_limits<float>::lowest();
    for (qsizetype i = 0; i < axisCount; ++i) {
        const float x = axis.points.at(i);
        const float y = samples.at(i);
        m_points.append(QPointF(x, y));
        m_minValue = std::min(m_minValue, y);
        m_maxValue = std::max(m_maxValue, y);
    }

    m_xAxisLabel = axis.name.isEmpty() ? tr("Axis") : axis.name;
    m_yAxisLabel = field.name.isEmpty() ? tr("Value") : field.name;

    m_widget->setData(m_points, m_xAxisLabel, m_yAxisLabel, m_config.color, m_config.gridLines, m_minValue, m_maxValue);
}

void Graph1DWindow::updateWindowTitle()
{
    if (!m_config.project) {
        setWindowTitle(tr("1D Graph"));
        return;
    }

    const video::Field &field = m_config.project->fields().at(m_config.fieldIndex);
    QString fieldName = field.name;
    if (fieldName.isEmpty()) {
        fieldName = tr("Field %1").arg(m_config.fieldIndex + 1);
    }

    QString projectName = m_config.project->name();
    if (projectName.isEmpty()) {
        projectName = tr("Project" );
    }

    setWindowTitle(tr("%1 — %2 (t = %3)").arg(projectName, fieldName, QString::number(m_config.timeIndex)));
}

video::VideoSession1Dim Graph1DWindow::sessionState() const
{
    video::VideoSession1Dim session;
    session.project = m_config.project;
    session.projectIndex = m_config.projectIndex;
    session.fieldIndex = m_config.fieldIndex;
    session.argumentIndex = 0;
    session.xMin = m_config.timeIndex;
    session.xMax = m_config.timeIndex;
    session.logarithmic = m_config.logarithmic;
    session.gridLines = m_config.gridLines;
    session.minValue = m_minValue;
    session.maxValue = m_maxValue;
    session.color = m_config.color;
    session.name = windowTitle();
    return session;
}

