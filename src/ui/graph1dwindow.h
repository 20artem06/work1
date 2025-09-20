#pragma once

#include <QColor>
#include <QVector>
#include <QPointF>
#include <memory>

#include <QWidget>

#include "core/session.h"

namespace video {
class Project;
}

class Graph1DWidget;

class Graph1DWindow : public QWidget
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
        QColor color = QColor(0, 0, 255);
    };

    explicit Graph1DWindow(const Config &config, QWidget *parent = nullptr);

    video::Project *project() const { return m_config.project; }
    int projectIndex() const { return m_config.projectIndex; }
    int fieldIndex() const { return m_config.fieldIndex; }

    video::VideoSession1Dim sessionState() const;

private:
    void populateData();
    void updateWindowTitle();

    Config m_config;
    Graph1DWidget *m_widget = nullptr;
    QVector<QPointF> m_points;
    QString m_xAxisLabel;
    QString m_yAxisLabel;
    float m_minValue = 0.0f;
    float m_maxValue = 0.0f;
};

