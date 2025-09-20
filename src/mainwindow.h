#pragma once

#include <memory>
#include <vector>

#include <QMainWindow>

namespace video {
class Project;
struct VideoSaveState;
}

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QListWidgetItem;
class QMdiSubWindow;
class Graph1DWindow;
class Graph2DWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void openProject();
    void closeProject();
    void closeAllProjects();
    void loadSession();
    void saveSession();
    void saveMovie();
    void saveData();
    void createGraph();
    void modifyGraph();
    void exitApplication();

    void toggleToolbar(bool checked);
    void toggleStatusBar(bool checked);
    void toggleProjectDock(bool checked);

    void cascadeWindows();
    void tileWindowsHorizontally();
    void tileWindowsVertically();
    void arrangeIcons();
    void closeAllSubWindows();

    void showAboutDialog();

    void handleProjectDoubleClick(QListWidgetItem *item);

private:
    struct OpenProject {
        std::unique_ptr<video::Project> project;
        QString sourcePath;
    };

    void setupInitialState();
    void setupConnections();
    void updateProjectList();
    void openProjectFromPath(const QString &path, bool showStatusMessage = true);
    void removeProjectAt(int index);
    void clearProjects();

    std::vector<video::Project*> projectPointers() const;
    int indexForProject(video::Project *project) const;

    void createOneDimensionalGraph();
    void createTwoDimensionalGraph();

    void restoreOneDimGraphs(const video::VideoSaveState &state);
    void restoreTwoDimGraphs(const video::VideoSaveState &state);

    void removeWindowsForProject(video::Project *project);

    Ui::MainWindow *ui = nullptr;
    std::vector<OpenProject> m_projects;
};

