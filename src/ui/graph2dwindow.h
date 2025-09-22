#pragma once

#include <QColor>
#include <QImage>
#include <QVector>
#include <QWidget>

#include "core/session.h"

namespace video {
class Project;
}

class Graph2DWidget;

class Graph2DWindow : public QWidget
{
    Q_OBJECT

public:
    explicit Graph2DWindow(const video::VideoSession2Dim &state, QWidget *parent = nullptr);

    void setWindowState(const video::VideoSession2Dim &state);
    video::VideoSession2Dim sessionState() const { return m_state; }

    QVector<double> xAxis() const { return m_xAxis; }
    QVector<double> yAxis() const { return m_yAxis; }
    QVector<float> values() const { return m_values; }

private:
    void rebuildImage();
    void updateWindowTitle();

    video::VideoSession2Dim m_state;
    Graph2DWidget *m_widget = nullptr;
    QVector<double> m_xAxis;
    QVector<double> m_yAxis;
    QImage m_image;
    QVector<float> m_values;
    float m_minValue = 0.0f;
    float m_maxValue = 0.0f;
};

