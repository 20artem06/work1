#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include "core/project.h"

class ProjectTest : public QObject
{
    Q_OBJECT

private slots:
    void loadProject();
    void readSlices();

private:
    QString writeProjectFiles(QTemporaryDir &dir);
};

QString ProjectTest::writeProjectFiles(QTemporaryDir &dir)
{
    const QString prjPath = dir.filePath("sample.prj");
    const QString dataPath = dir.filePath("data.bin");

    QFile prjFile(prjPath);
    QVERIFY(prjFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&prjFile);
    out << "data.bin\n";
    out << "Time\n";
    out << "s\n";
    out << "1\n";

    out << "Temperature\n";
    out << "C\n";
    out << "1\n";
    out << "Depth\n";
    out << "m\n";
    out << "-3\n";
    out << "0.0\n1.0\n2.0\n";

    prjFile.close();

    QFile dataFile(dataPath);
    QVERIFY(dataFile.open(QIODevice::WriteOnly));
    QDataStream dataOut(&dataFile);
    dataOut.setByteOrder(QDataStream::LittleEndian);
    dataOut.setFloatingPointPrecision(QDataStream::SinglePrecision);

    const QVector<float> sample1{10.0f, 12.0f, 14.0f};
    const QVector<float> sample2{11.0f, 13.0f, 15.0f};
    dataOut << 0.0f;
    for (float value : sample1) {
        dataOut << value;
    }
    dataOut << 5.0f;
    for (float value : sample2) {
        dataOut << value;
    }
    dataFile.close();

    return prjPath;
}

void ProjectTest::loadProject()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString prjPath = writeProjectFiles(dir);

    video::Project project = video::Project::loadFromDescription(prjPath);
    QCOMPARE(project.fields().size(), 1);
    QCOMPARE(project.timeValues().size(), 2);
    QCOMPARE(project.fields().first().sampleCount(), 3);
    QCOMPARE(project.fields().first().minimum, 10.0f);
    QCOMPARE(project.fields().first().maximum, 15.0f);
    QCOMPARE(project.fields().first().axes.first().points.size(), 3);
    QCOMPARE(project.fields().first().axes.first().points.first(), 0.0f);
    QCOMPARE(project.fields().first().axes.first().points.last(), 2.0f);
}

void ProjectTest::readSlices()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString prjPath = writeProjectFiles(dir);
    video::Project project = video::Project::loadFromDescription(prjPath);

    QVector<float> buffer;
    project.readPoint(0, 0, 1, 3, buffer, false);
    QCOMPARE(buffer.size(), 3);
    QCOMPARE(buffer[0], 10.0f);
    QCOMPARE(buffer[2], 14.0f);

    project.readPoint(0, 3, 1, 3, buffer, false);
    QCOMPARE(buffer[0], 11.0f);
    QCOMPARE(buffer[2], 15.0f);
}

QTEST_MAIN(ProjectTest)
#include "test_project.moc"
