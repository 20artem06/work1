#include "session.h"

#include <stdexcept>
#include <utility>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

namespace video {

namespace {

bool toBoolFromNumber(const QJsonValue& value, bool defaultValue = false)
{
    if (value.isBool()) {
        return value.toBool();
    }
    if (value.isDouble()) {
        return value.toInt() != 0;
    }
    return defaultValue;
}

QColor colorFromJson(const QJsonObject& object, const QColor& fallback)
{
    QColor color = fallback;
    if (object.contains(QLatin1String("r"))) {
        color.setRed(object.value(QLatin1String("r")).toInt(color.red()));
    }
    if (object.contains(QLatin1String("g"))) {
        color.setGreen(object.value(QLatin1String("g")).toInt(color.green()));
    }
    if (object.contains(QLatin1String("b"))) {
        color.setBlue(object.value(QLatin1String("b")).toInt(color.blue()));
    }
    return color;
}

QJsonObject colorToJson(const QColor& color)
{
    QJsonObject obj;
    obj.insert(QLatin1String("r"), color.red());
    obj.insert(QLatin1String("g"), color.green());
    obj.insert(QLatin1String("b"), color.blue());
    return obj;
}

}  // namespace

VideoSaveState sessionStateFromJson(const QJsonObject& object)
{
    const QString version = object.value(QLatin1String("version")).toString();
    if (!version.isEmpty() && version != QLatin1String(kSessionVersion)) {
        throw std::runtime_error("Unsupported session file version.");
    }

    VideoSaveState state;

    const QJsonArray projectsArray = object.value(QLatin1String("projects")).toArray();
    for (const QJsonValue& value : projectsArray) {
        state.projects.append(value.toString());
    }

    const QJsonArray twoDimArray = object.value(QLatin1String("two_dim")).toArray();
    for (const QJsonValue& value : twoDimArray) {
        const QJsonObject obj = value.toObject();
        VideoSession2Dim session;
        session.fieldIndex = obj.value(QLatin1String("m_nField")).toInt();
        session.minValue = static_cast<float>(obj.value(QLatin1String("m_fFmin")).toDouble());
        session.maxValue = static_cast<float>(obj.value(QLatin1String("m_fFmax")).toDouble());
        session.polar = toBoolFromNumber(obj.value(QLatin1String("m_bPolar")));
        session.logarithmic = toBoolFromNumber(obj.value(QLatin1String("m_bLog")));
        session.argument1 = obj.value(QLatin1String("m_nArg1")).toInt();
        session.x1Min = obj.value(QLatin1String("m_nX1min")).toInt();
        session.x1Max = obj.value(QLatin1String("m_nX1max")).toInt();
        session.argument2 = obj.value(QLatin1String("m_nArg2")).toInt();
        session.x2Min = obj.value(QLatin1String("m_nX2min")).toInt();
        session.x2Max = obj.value(QLatin1String("m_nX2max")).toInt();
        session.gridCount = obj.value(QLatin1String("m_nGrids")).toInt();
        session.gridLines = toBoolFromNumber(obj.value(QLatin1String("m_bGridLines")));
        session.autoGrid = toBoolFromNumber(obj.value(QLatin1String("m_bGridAuto")));
        session.palette = obj.value(QLatin1String("m_Palette")).toInt();
        session.layers = obj.value(QLatin1String("m_layers")).toInt();
        session.cloud = obj.value(QLatin1String("m_cloud")).toInt();
        session.isolines = obj.value(QLatin1String("m_lines")).toInt();
        session.projectIndex = obj.value(QLatin1String("projectIndex")).toInt(-1);

        const QJsonArray indexArray = obj.value(QLatin1String("index")).toArray();
        session.indices.reserve(indexArray.size());
        for (const QJsonValue& indexValue : indexArray) {
            session.indices.append(indexValue.toInt());
        }

        state.twoDimGraphs.append(std::move(session));
    }

    const QJsonArray oneDimArray = object.value(QLatin1String("one_dim")).toArray();
    for (const QJsonValue& value : oneDimArray) {
        const QJsonObject windowObject = value.toObject();
        OneDimWindowState window;

        const QJsonArray valuesArray = windowObject.value(QLatin1String("values")).toArray();
        window.graphs.reserve(valuesArray.size());
        for (const QJsonValue& graphValue : valuesArray) {
            const QJsonObject graphObject = graphValue.toObject();
            VideoSession1Dim graph;
            graph.fieldIndex = graphObject.value(QLatin1String("nField")).toInt();
            graph.polar = toBoolFromNumber(graphObject.value(QLatin1String("bPolar")));
            graph.minValue = static_cast<float>(graphObject.value(QLatin1String("fFmin")).toDouble());
            graph.maxValue = static_cast<float>(graphObject.value(QLatin1String("fFmax")).toDouble());
            graph.argumentIndex = graphObject.value(QLatin1String("nArg1")).toInt();
            graph.xMin = graphObject.value(QLatin1String("nX1min")).toInt();
            graph.xMax = graphObject.value(QLatin1String("nX1max")).toInt();
            graph.style = graphObject.value(QLatin1String("Style")).toInt();
            graph.gridLines = toBoolFromNumber(graphObject.value(QLatin1String("bGridL")));
            graph.logarithmic = toBoolFromNumber(graphObject.value(QLatin1String("bLog")));
            graph.name = graphObject.value(QLatin1String("sName")).toString();
            graph.projectIndex = graphObject.value(QLatin1String("projectIndex")).toInt(-1);

            const QJsonObject colorObject = graphObject.value(QLatin1String("color")).toObject();
            graph.color = colorFromJson(colorObject, graph.color);

            const QJsonArray graphIndices = graphObject.value(QLatin1String("index")).toArray();
            graph.indices.reserve(graphIndices.size());
            for (const QJsonValue& indexValue : graphIndices) {
                graph.indices.append(indexValue.toInt());
            }

            window.graphs.append(std::move(graph));
        }

        const QJsonObject meta = windowObject.value(QLatin1String("meta")).toObject();
        window.x1Min = static_cast<float>(meta.value(QLatin1String("fX1min")).toDouble());
        window.x1Max = static_cast<float>(meta.value(QLatin1String("fX1max")).toDouble());
        window.x2Min = static_cast<float>(meta.value(QLatin1String("fX2min")).toDouble());
        window.x2Max = static_cast<float>(meta.value(QLatin1String("fX2max")).toDouble());
        window.axisStatus = meta.value(QLatin1String("AxeStatus")).toInt();

        state.oneDimGraphs.append(std::move(window));
    }

    return state;
}

QJsonObject sessionStateToJson(const VideoSaveState& state)
{
    QJsonObject root;

    QJsonArray projectsArray;
    for (const QString& project : state.projects) {
        projectsArray.append(project);
    }
    root.insert(QLatin1String("projects"), projectsArray);

    QJsonArray twoDimArray;
    for (const VideoSession2Dim& session : state.twoDimGraphs) {
        QJsonObject obj;
        obj.insert(QLatin1String("m_nField"), session.fieldIndex);
        obj.insert(QLatin1String("m_fFmin"), session.minValue);
        obj.insert(QLatin1String("m_fFmax"), session.maxValue);
        obj.insert(QLatin1String("m_bPolar"), session.polar ? 1 : 0);
        obj.insert(QLatin1String("m_bLog"), session.logarithmic ? 1 : 0);
        obj.insert(QLatin1String("m_nArg1"), session.argument1);
        obj.insert(QLatin1String("m_nX1min"), session.x1Min);
        obj.insert(QLatin1String("m_nX1max"), session.x1Max);
        obj.insert(QLatin1String("m_nArg2"), session.argument2);
        obj.insert(QLatin1String("m_nX2min"), session.x2Min);
        obj.insert(QLatin1String("m_nX2max"), session.x2Max);
        obj.insert(QLatin1String("m_nGrids"), session.gridCount);
        obj.insert(QLatin1String("m_bGridLines"), session.gridLines ? 1 : 0);
        obj.insert(QLatin1String("m_bGridAuto"), session.autoGrid ? 1 : 0);
        obj.insert(QLatin1String("m_Palette"), session.palette);
        obj.insert(QLatin1String("m_layers"), session.layers);
        obj.insert(QLatin1String("m_cloud"), session.cloud);
        obj.insert(QLatin1String("m_lines"), session.isolines);
        obj.insert(QLatin1String("projectIndex"), session.projectIndex);

        QJsonArray indexArray;
        for (int index : session.indices) {
            indexArray.append(index);
        }
        obj.insert(QLatin1String("index"), indexArray);

        twoDimArray.append(obj);
    }
    root.insert(QLatin1String("two_dim"), twoDimArray);

    QJsonArray oneDimArray;
    for (const OneDimWindowState& window : state.oneDimGraphs) {
        QJsonObject windowObject;
        QJsonArray graphsArray;
        for (const VideoSession1Dim& graph : window.graphs) {
            QJsonObject graphObject;
            graphObject.insert(QLatin1String("nField"), graph.fieldIndex);
            graphObject.insert(QLatin1String("bPolar"), graph.polar ? 1 : 0);
            graphObject.insert(QLatin1String("fFmin"), graph.minValue);
            graphObject.insert(QLatin1String("fFmax"), graph.maxValue);
            graphObject.insert(QLatin1String("nArg1"), graph.argumentIndex);
            graphObject.insert(QLatin1String("nX1min"), graph.xMin);
            graphObject.insert(QLatin1String("nX1max"), graph.xMax);
            graphObject.insert(QLatin1String("Style"), graph.style);
            graphObject.insert(QLatin1String("bGridL"), graph.gridLines ? 1 : 0);
            graphObject.insert(QLatin1String("bLog"), graph.logarithmic ? 1 : 0);
            graphObject.insert(QLatin1String("sName"), graph.name);
            graphObject.insert(QLatin1String("projectIndex"), graph.projectIndex);
            graphObject.insert(QLatin1String("color"), colorToJson(graph.color));

            QJsonArray indexArray;
            for (int index : graph.indices) {
                indexArray.append(index);
            }
            graphObject.insert(QLatin1String("index"), indexArray);

            graphsArray.append(graphObject);
        }
        windowObject.insert(QLatin1String("values"), graphsArray);

        QJsonObject meta;
        meta.insert(QLatin1String("fX1min"), window.x1Min);
        meta.insert(QLatin1String("fX1max"), window.x1Max);
        meta.insert(QLatin1String("fX2min"), window.x2Min);
        meta.insert(QLatin1String("fX2max"), window.x2Max);
        meta.insert(QLatin1String("AxeStatus"), window.axisStatus);
        windowObject.insert(QLatin1String("meta"), meta);

        oneDimArray.append(windowObject);
    }
    root.insert(QLatin1String("one_dim"), oneDimArray);

    root.insert(QLatin1String("version"), QLatin1String(kSessionVersion));
    return root;
}

VideoSaveState readVideoSave(QIODevice* device)
{
    if (!device) {
        throw std::invalid_argument("Device pointer is null.");
    }
    const QByteArray bytes = device->readAll();
    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        throw std::runtime_error(QStringLiteral("Failed to parse session file: %1")
                                     .arg(parseError.errorString())
                                     .toStdString());
    }
    if (!document.isObject()) {
        throw std::runtime_error("Session file does not contain a JSON object.");
    }
    return sessionStateFromJson(document.object());
}

void writeVideoSave(const VideoSaveState& state, QIODevice* device)
{
    if (!device) {
        throw std::invalid_argument("Device pointer is null.");
    }
    const QJsonObject root = sessionStateToJson(state);
    const QJsonDocument document(root);
    const QByteArray serialized = document.toJson(QJsonDocument::Indented);
    if (device->write(serialized) != serialized.size()) {
        throw std::runtime_error("Failed to write session data.");
    }
}

}  // namespace video

