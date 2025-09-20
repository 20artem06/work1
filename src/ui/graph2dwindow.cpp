#include "graph2dwindow.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <exception>
#include <limits>

#include "core/project.h"

namespace {
QColor interpolateColor(float value, float minValue, float maxValue)
{
    if (maxValue <= minValue) {
        return QColor(128, 128, 128);
    }
    const float ratio = std::clamp((value - minValue) / (maxValue - minValue), 0.0f, 1.0f);
    const int hue = static_cast<int>((1.0f - ratio) * 240.0f);
    QColor color;
    color.setHsv(hue, 255, 255);
    return color;
}

class Graph2DWidget : public QWidget
{
public:
    explicit Graph2DWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAutoFillBackground(true);
    }

    void setData(const QImage &image, const QVector<double> &xAxis, const QVector<double> &yAxis,
                 bool grid, float minValue, float maxValue)
    {
        m_image = image;
        m_xAxis = xAxis;
        m_yAxis = yAxis;
        m_grid = grid;
        m_minValue = minValue;
        m_maxValue = maxValue;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        painter.fillRect(rect(), palette().window());

        if (m_image.isNull()) {
            painter.setPen(palette().text().color());
            painter.drawText(rect(), Qt::AlignCenter, tr("No data"));
            return;
        }

        const QRectF plotArea = rect().adjusted(60, 20, -60, -40);
        painter.setPen(palette().mid().color());
        painter.drawRect(plotArea);

        painter.drawImage(plotArea, m_image);

        if (m_grid && !m_xAxis.isEmpty() && !m_yAxis.isEmpty()) {
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
                         Qt::AlignCenter, tr("X: %1 – %2").arg(m_xAxis.first(), 0, 'f', 3).arg(m_xAxis.last(), 0, 'f', 3));
        painter.drawText(QRectF(plotArea.left() - 56, plotArea.top(), 50, plotArea.height()), Qt::AlignCenter | Qt::TextWordWrap,
                         tr("Y: %1 – %2").arg(m_yAxis.first(), 0, 'f', 3).arg(m_yAxis.last(), 0, 'f', 3));

        painter.drawText(rect().adjusted(0, 0, -10, -rect().height() + 20), Qt::AlignRight | Qt::AlignTop,
                         tr("Min: %1\nMax: %2").arg(m_minValue, 0, 'f', 3).arg(m_maxValue, 0, 'f', 3));
    }

private:
    QImage m_image;
    QVector<double> m_xAxis;
    QVector<double> m_yAxis;
    bool m_grid = false;
    float m_minValue = 0.0f;
    float m_maxValue = 0.0f;
};
} // namespace

Graph2DWindow::Graph2DWindow(const Config &config, QWidget *parent)
    : QWidget(parent)
    , m_config(config)
{
    setAttribute(Qt::WA_DeleteOnClose);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_widget = new Graph2DWidget(this);
    layout->addWidget(m_widget);

    populateData();
    updateWindowTitle();
}

void Graph2DWindow::populateData()
{
    if (!m_config.project || m_config.fieldIndex < 0 || m_config.fieldIndex >= m_config.project->fields().size()) {
        return;
    }

    const video::Field &field = m_config.project->fields()[m_config.fieldIndex];
    if (field.axes.size() < 2) {
        return;
    }

    const video::Axis &axisX = field.axes.at(0);
    const video::Axis &axisY = field.axes.at(1);

    const int width = axisX.points.size();
    const int height = axisY.points.size();
    if (width <= 0 || height <= 0) {
        return;
    }

    const qsizetype samplesPerTime = field.sampleCount();
    const qsizetype totalTimes = m_config.project->timeValues().size();
    if (totalTimes <= 0) {
        return;
    }
    const qsizetype timeIndex = std::clamp<qsizetype>(static_cast<qsizetype>(m_config.timeIndex), 0, totalTimes - 1);
    const qint64 startIndex = static_cast<qint64>(timeIndex) * static_cast<qint64>(samplesPerTime);

    QVector<float> values;
    try {
        m_config.project->readPoints(m_config.fieldIndex, startIndex, 1, width, 0, height, values, m_config.logarithmic);
    } catch (const std::exception &) {
        values.clear();
    }

    if (values.size() != width * height) {
        return;
    }

    m_values = values;
    m_xAxis = QVector<double>(axisX.points.begin(), axisX.points.end());
    m_yAxis = QVector<double>(axisY.points.begin(), axisY.points.end());

    m_minValue = std::numeric_limits<float>::max();
    m_maxValue = std::numeric_limits<float>::lowest();
    for (float value : m_values) {
        m_minValue = std::min(m_minValue, value);
        m_maxValue = std::max(m_maxValue, value);
    }

    QImage image(width, height, QImage::Format_RGB32);
    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            const float value = m_values.at(y * width + x);
            const QColor color = interpolateColor(value, m_minValue, m_maxValue);
            line[x] = color.rgb();
        }
    }
    image = image.mirrored(false, true);

    m_widget->setData(image, m_xAxis, m_yAxis, m_config.gridLines, m_minValue, m_maxValue);
}

void Graph2DWindow::updateWindowTitle()
{
    if (!m_config.project) {
        setWindowTitle(tr("2D Graph"));
        return;
    }

    const video::Field &field = m_config.project->fields().at(m_config.fieldIndex);
    QString fieldName = field.name;
    if (fieldName.isEmpty()) {
        fieldName = tr("Field %1").arg(m_config.fieldIndex + 1);
    }

    QString projectName = m_config.project->name();
    if (projectName.isEmpty()) {
        projectName = tr("Project");
    }

    setWindowTitle(tr("%1 — %2 (t = %3)").arg(projectName, fieldName, QString::number(m_config.timeIndex)));
}

video::VideoSession2Dim Graph2DWindow::sessionState() const
{
    video::VideoSession2Dim session;
    session.project = m_config.project;
    session.projectIndex = m_config.projectIndex;
    session.fieldIndex = m_config.fieldIndex;
    session.argument1 = 0;
    session.argument2 = 1;
    session.x1Min = m_config.timeIndex;
    session.x1Max = m_config.timeIndex;
    session.x2Min = 0;
    session.x2Max = 0;
    session.logarithmic = m_config.logarithmic;
    session.gridLines = m_config.gridLines;
    session.minValue = m_minValue;
    session.maxValue = m_maxValue;
    return session;
}

