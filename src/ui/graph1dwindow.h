#pragma once

#include <QColor>
#include <QPointF>
#include <QWidget>
#include <QVector>

#include "core/session.h"

namespace video {
class Project;
}

class Graph1DPlotWidget;

class Graph1DWindow : public QWidget
{
    Q_OBJECT

public:
    explicit Graph1DWindow(const video::OneDimWindowState &state, QWidget *parent = nullptr);

    void setWindowState(const video::OneDimWindowState &state);
    const video::OneDimWindowState &windowState() const { return m_state; }

    video::OneDimWindowState sessionState() const { return m_state; }

    struct ExportSeries {
        QString name;
        QString yLabel;
        QVector<QPointF> points;
    };

    QVector<ExportSeries> exportSeries() const;
    QString xAxisLabel() const;

private:
    struct SeriesData {
        video::VideoSession1Dim state;
        QVector<QPointF> points;
        QString xLabel;
        QString yLabel;
        float minValue = 0.0f;
        float maxValue = 0.0f;
    };

    struct VisualSeries {
        QPolygonF polyline;
        QColor color;
        Qt::PenStyle style;
        QString name;
        float minValue = 0.0f;
        float maxValue = 0.0f;
    };

    void rebuildData();
    void updateWindowTitle();
    void updatePlot();
    void ensureSeriesIndices(video::VideoSession1Dim &session, const video::Field &field) const;

    video::OneDimWindowState m_state;
    QVector<SeriesData> m_series;
    Graph1DPlotWidget *m_plotWidget = nullptr;
};

