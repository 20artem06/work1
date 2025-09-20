#pragma once

#include <QColor>
#include <QImage>
#include <QWidget>
#include <QVector>

#include "core/session.h"

namespace video {
class Project;
}

class Graph2DWidget;

class Graph2DWindow : public QWidget
{
    Q_OBJECT

public:
    struct Config {
        video::Project *project = nullptr;
        int projectIndex = -1;
        int fieldIndex = 0;
        int timeIndex = 0;
        bool logarithmic = false;
        bool gridLines = false;
    };

    explicit Graph2DWindow(const Config &config, QWidget *parent = nullptr);

    video::Project *project() const { return m_config.project; }
    int projectIndex() const { return m_config.projectIndex; }
    int fieldIndex() const { return m_config.fieldIndex; }

    video::VideoSession2Dim sessionState() const;

private:
    void populateData();
    void updateWindowTitle();

    Config m_config;
    Graph2DWidget *m_widget = nullptr;
    QVector<double> m_xAxis;
    QVector<double> m_yAxis;
    QVector<float> m_values;
    float m_minValue = 0.0f;
    float m_maxValue = 0.0f;
};

