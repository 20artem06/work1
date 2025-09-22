#include <QtTest/QtTest>

#include <QJsonObject>

#include "core/session.h"

class SessionTest : public QObject
{
    Q_OBJECT

private slots:
    void roundTrip();
};

void SessionTest::roundTrip()
{
    video::VideoSaveState original;
    original.projects << "first.prj" << "second.prj";

    video::VideoSession1Dim series;
    series.fieldIndex = 1;
    series.xMin = 2;
    series.xMax = 5;
    series.argumentIndex = 0;
    series.projectIndex = 0;
    series.gridLines = true;
    series.logarithmic = false;
    series.color = QColor(10, 20, 30);
    series.indices = {3};
    series.name = "Sample";

    video::OneDimWindowState window;
    window.graphs.append(series);
    window.x1Min = -1.0f;
    window.x1Max = 2.5f;
    window.axisStatus = 1;
    original.oneDimGraphs.append(window);

    video::VideoSession2Dim grid;
    grid.fieldIndex = 2;
    grid.projectIndex = 1;
    grid.argument1 = 0;
    grid.argument2 = 1;
    grid.x1Min = 4;
    grid.x1Max = 6;
    grid.x2Min = 1;
    grid.x2Max = 3;
    grid.palette = 1;
    grid.gridLines = true;
    grid.autoGrid = false;
    grid.indices = {2};
    original.twoDimGraphs.append(grid);

    const QJsonObject json = video::sessionStateToJson(original);
    const video::VideoSaveState restored = video::sessionStateFromJson(json);

    QCOMPARE(restored.projects, original.projects);
    QCOMPARE(restored.oneDimGraphs.size(), original.oneDimGraphs.size());
    QCOMPARE(restored.twoDimGraphs.size(), original.twoDimGraphs.size());

    const video::OneDimWindowState restoredWindow = restored.oneDimGraphs.first();
    QCOMPARE(restoredWindow.graphs.first().fieldIndex, series.fieldIndex);
    QCOMPARE(restoredWindow.graphs.first().xMin, series.xMin);
    QCOMPARE(restoredWindow.graphs.first().xMax, series.xMax);
    QCOMPARE(restoredWindow.graphs.first().color, series.color);
    QCOMPARE(restoredWindow.graphs.first().indices.first(), series.indices.first());
    QCOMPARE(restoredWindow.x1Min, window.x1Min);
    QCOMPARE(restoredWindow.axisStatus, window.axisStatus);

    const video::VideoSession2Dim restoredGrid = restored.twoDimGraphs.first();
    QCOMPARE(restoredGrid.fieldIndex, grid.fieldIndex);
    QCOMPARE(restoredGrid.projectIndex, grid.projectIndex);
    QCOMPARE(restoredGrid.palette, grid.palette);
    QCOMPARE(restoredGrid.indices.first(), grid.indices.first());
}

QTEST_MAIN(SessionTest)
#include "test_session.moc"
