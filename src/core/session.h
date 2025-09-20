#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QColor>
#include <QIODevice>
#include <QJsonObject>

#include "project.h"

namespace video {

inline constexpr const char kSessionVersion[] = "video_session_0.1";

struct VideoSession1Dim {
    Project* project = nullptr;

    int fieldIndex = 0;
    bool polar = false;
    float minValue = 0.0f;
    float maxValue = 0.0f;
    int argumentIndex = 0;
    int xMin = 0;
    int xMax = 0;
    QColor color = QColor(0, 0, 255);
    int style = 0;
    bool gridLines = false;
    bool logarithmic = false;
    QString name;
    QVector<int> indices;

    int projectIndex = -1;
};

struct VideoSession2Dim {
    Project* project = nullptr;

    int fieldIndex = 0;
    float minValue = 0.0f;
    float maxValue = 0.0f;
    bool polar = false;
    bool logarithmic = false;
    int argument1 = 0;
    int x1Min = 0;
    int x1Max = 0;
    int argument2 = 0;
    int x2Min = 0;
    int x2Max = 0;
    int gridCount = 0;
    bool gridLines = false;
    bool autoGrid = false;
    int palette = 0;

    QColor colorMin = QColor(0, 0, 255);
    QColor colorMax = QColor(255, 0, 0);
    int layers = 0;
    int cloud = 0;
    int isolines = 0;
    QVector<int> indices;

    int projectIndex = -1;
};

struct OneDimWindowState {
    QVector<VideoSession1Dim> graphs;

    float x1Min = 0.0f;
    float x1Max = 0.0f;
    float x2Min = 0.0f;
    float x2Max = 0.0f;
    int axisStatus = 0;
};

struct VideoSaveState {
    QStringList projects;
    QVector<VideoSession2Dim> twoDimGraphs;
    QVector<OneDimWindowState> oneDimGraphs;
};

VideoSaveState readVideoSave(QIODevice* device);
void writeVideoSave(const VideoSaveState& state, QIODevice* device);

VideoSaveState sessionStateFromJson(const QJsonObject& object);
QJsonObject sessionStateToJson(const VideoSaveState& state);

}  // namespace video

