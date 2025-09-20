#include "mainwindow.h"
#include "ui_MainWindow.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QRect>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QStringList>

#include <algorithm>
#include <exception>

#include "core/project.h"
#include "core/session.h"

#include "ui/deleteprojectdialog.h"
#include "ui/graph1dwindow.h"
#include "ui/graph2dwindow.h"
#include "ui/graphselectiondialog.h"
#include "ui/onediagraphdialog.h"
#include "ui/twodiagraphdialog.h"

namespace {
QString displayNameForProject(const video::Project &project, const QString &fallback)
{
    if (!project.name().isEmpty()) {
        return project.name();
    }
    return fallback.isEmpty() ? QObject::tr("Project") : fallback;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupInitialState();
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupInitialState()
{
    if (ui->mainToolBar) {
        ui->actionToolbar->setChecked(ui->mainToolBar->isVisible());
    }
    if (ui->statusbar) {
        ui->actionStatusBar->setChecked(ui->statusbar->isVisible());
    }
    if (ui->projectDock) {
        ui->actionProjectDock->setChecked(ui->projectDock->isVisible());
    }

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupConnections()
{
    connect(ui->actionOpenProject, &QAction::triggered, this, &MainWindow::openProject);
    connect(ui->actionCloseProject, &QAction::triggered, this, &MainWindow::closeProject);
    connect(ui->actionCloseAllProjects, &QAction::triggered, this, &MainWindow::closeAllProjects);
    connect(ui->actionLoadSession, &QAction::triggered, this, &MainWindow::loadSession);
    connect(ui->actionSaveSession, &QAction::triggered, this, &MainWindow::saveSession);
    connect(ui->actionSaveMovie, &QAction::triggered, this, &MainWindow::saveMovie);
    connect(ui->actionSaveData, &QAction::triggered, this, &MainWindow::saveData);
    connect(ui->actionGraphNew, &QAction::triggered, this, &MainWindow::createGraph);
    connect(ui->actionGraphModify, &QAction::triggered, this, &MainWindow::modifyGraph);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exitApplication);

    connect(ui->actionToolbar, &QAction::toggled, this, &MainWindow::toggleToolbar);
    connect(ui->actionStatusBar, &QAction::toggled, this, &MainWindow::toggleStatusBar);
    connect(ui->actionProjectDock, &QAction::toggled, this, &MainWindow::toggleProjectDock);

    connect(ui->actionWindowCascade, &QAction::triggered, this, &MainWindow::cascadeWindows);
    connect(ui->actionWindowTileH, &QAction::triggered, this, &MainWindow::tileWindowsHorizontally);
    connect(ui->actionWindowTileV, &QAction::triggered, this, &MainWindow::tileWindowsVertically);
    connect(ui->actionWindowArrange, &QAction::triggered, this, &MainWindow::arrangeIcons);
    connect(ui->actionWindowCloseAll, &QAction::triggered, this, &MainWindow::closeAllSubWindows);

    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    connect(ui->projectList, &QListWidget::itemDoubleClicked, this, &MainWindow::handleProjectDoubleClick);
    if (ui->projectDock) {
        connect(ui->projectDock, &QDockWidget::visibilityChanged, ui->actionProjectDock, &QAction::setChecked);
    }
}

void MainWindow::openProject()
{
    const QString filter = tr("Project description (*.prj);;All files (*.*)");
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open project"), QString(), filter);
    if (filePath.isEmpty()) {
        return;
    }

    openProjectFromPath(filePath);
}

void MainWindow::openProjectFromPath(const QString &path, bool showStatusMessage)
{
    if (path.isEmpty()) {
        return;
    }

    QFileInfo info(path);
    const QString canonical = info.exists() ? info.canonicalFilePath() : path;

    for (const OpenProject &entry : m_projects) {
        QFileInfo existing(entry.sourcePath);
        if (!canonical.isEmpty() && !existing.canonicalFilePath().isEmpty() && existing.canonicalFilePath() == canonical) {
            QMessageBox::information(this, tr("Project already open"),
                                     tr("The project '%1' is already loaded.").arg(displayNameForProject(*entry.project, existing.fileName())));
            return;
        }
    }

    try {
        video::Project project = video::Project::loadFromDescription(path);
        auto openProject = OpenProject{std::make_unique<video::Project>(std::move(project)), canonical};
        m_projects.push_back(std::move(openProject));
        updateProjectList();
        if (showStatusMessage) {
            statusBar()->showMessage(tr("Loaded project: %1").arg(QDir::toNativeSeparators(path)), 5000);
        }
    } catch (const std::exception &ex) {
        QMessageBox::critical(this, tr("Open project"),
                              tr("Unable to load project:\n%1").arg(QString::fromUtf8(ex.what())));
    }
}

void MainWindow::updateProjectList()
{
    ui->projectList->clear();

    for (int index = 0; index < static_cast<int>(m_projects.size()); ++index) {
        const OpenProject &entry = m_projects[index];
        QString name = entry.project ? entry.project->name() : QString();
        if (name.isEmpty()) {
            name = QFileInfo(entry.sourcePath).baseName();
        }
        if (name.isEmpty()) {
            name = tr("Project %1").arg(index + 1);
        }

        auto *item = new QListWidgetItem(name, ui->projectList);
        item->setData(Qt::UserRole, index);
        item->setToolTip(entry.sourcePath);
    }
}

std::vector<video::Project*> MainWindow::projectPointers() const
{
    std::vector<video::Project*> result;
    result.reserve(m_projects.size());
    for (const OpenProject &entry : m_projects) {
        result.push_back(entry.project.get());
    }
    return result;
}

int MainWindow::indexForProject(video::Project *project) const
{
    for (int i = 0; i < static_cast<int>(m_projects.size()); ++i) {
        if (m_projects[i].project.get() == project) {
            return i;
        }
    }
    return -1;
}

void MainWindow::closeProject()
{
    if (m_projects.empty()) {
        QMessageBox::information(this, tr("Close project"), tr("There are no open projects."));
        return;
    }

    QStringList names;
    names.reserve(static_cast<int>(m_projects.size()));
    for (const OpenProject &entry : m_projects) {
        names.append(displayNameForProject(*entry.project, QFileInfo(entry.sourcePath).baseName()));
    }

    DeleteProjectDialog dialog(names, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (dialog.deleteAll()) {
        clearProjects();
        statusBar()->showMessage(tr("All projects closed"), 5000);
        return;
    }

    const int index = dialog.selectedProjectIndex();
    if (index >= 0 && index < static_cast<int>(m_projects.size())) {
        removeProjectAt(index);
    }
}

void MainWindow::removeProjectAt(int index)
{
    if (index < 0 || index >= static_cast<int>(m_projects.size())) {
        return;
    }

    video::Project *project = m_projects[index].project.get();
    removeWindowsForProject(project);
    m_projects.erase(m_projects.begin() + index);
    updateProjectList();
}

void MainWindow::clearProjects()
{
    ui->mdiArea->closeAllSubWindows();
    m_projects.clear();
    updateProjectList();
}

void MainWindow::removeWindowsForProject(video::Project *project)
{
    if (!project) {
        return;
    }

    const auto subWindows = ui->mdiArea->subWindowList();
    for (QMdiSubWindow *subWindow : subWindows) {
        QWidget *widget = subWindow->widget();
        if (auto *graph1D = qobject_cast<Graph1DWindow *>(widget)) {
            if (graph1D->project() == project) {
                subWindow->close();
            }
        } else if (auto *graph2D = qobject_cast<Graph2DWindow *>(widget)) {
            if (graph2D->project() == project) {
                subWindow->close();
            }
        }
    }
}

void MainWindow::closeAllProjects()
{
    if (m_projects.empty()) {
        return;
    }
    clearProjects();
    statusBar()->showMessage(tr("All projects closed"), 5000);
}

void MainWindow::createGraph()
{
    if (m_projects.empty()) {
        QMessageBox::information(this, tr("Create graph"), tr("Load a project before creating graphs."));
        return;
    }

    GraphSelectionDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (dialog.selectedType() == GraphSelectionDialog::GraphType::OneDimensional) {
        createOneDimensionalGraph();
    } else {
        createTwoDimensionalGraph();
    }
}

void MainWindow::createOneDimensionalGraph()
{
    auto projects = projectPointers();
    OneDimGraphDialog dialog(projects, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto result = dialog.result();
    if (!result.project || result.projectIndex < 0 || result.projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }

    Graph1DWindow::Config config;
    config.project = result.project;
    config.projectIndex = result.projectIndex;
    config.fieldIndex = result.fieldIndex;
    config.timeIndex = result.timeIndex;
    config.logarithmic = result.logarithmic;
    config.gridLines = result.gridLines;

    auto *widget = new Graph1DWindow(config);
    ui->mdiArea->addSubWindow(widget);
    widget->show();
}

void MainWindow::createTwoDimensionalGraph()
{
    auto projects = projectPointers();
    TwoDimGraphDialog dialog(projects, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto result = dialog.result();
    if (!result.project || result.projectIndex < 0 || result.projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }

    Graph2DWindow::Config config;
    config.project = result.project;
    config.projectIndex = result.projectIndex;
    config.fieldIndex = result.fieldIndex;
    config.timeIndex = result.timeIndex;
    config.logarithmic = result.logarithmic;
    config.gridLines = result.gridLines;

    auto *widget = new Graph2DWindow(config);
    ui->mdiArea->addSubWindow(widget);
    widget->show();
}

void MainWindow::modifyGraph()
{
    QMessageBox::information(this, tr("Modify graph"), tr("Graph editing is not yet implemented. Close the window and create a new graph."));
}

void MainWindow::loadSession()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Load session"), QString(), tr("Video sessions (*.json);;All files (*.*)"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Load session"), tr("Unable to open session file."));
        return;
    }

    video::VideoSaveState state;
    try {
        state = video::readVideoSave(&file);
    } catch (const std::exception &ex) {
        QMessageBox::critical(this, tr("Load session"), tr("Failed to parse session file:\n%1").arg(QString::fromUtf8(ex.what())));
        return;
    }

    clearProjects();

    const QDir baseDir = QFileInfo(filePath).absoluteDir();
    for (const QString &projectPath : state.projects) {
        QString resolved = projectPath;
        if (!QFileInfo(projectPath).isAbsolute()) {
            resolved = baseDir.filePath(projectPath);
        }
        openProjectFromPath(resolved, false);
    }

    restoreOneDimGraphs(state);
    restoreTwoDimGraphs(state);

    statusBar()->showMessage(tr("Session loaded"), 5000);
}

void MainWindow::restoreOneDimGraphs(const video::VideoSaveState &state)
{
    for (const video::OneDimWindowState &windowState : state.oneDimGraphs) {
        for (const video::VideoSession1Dim &graphState : windowState.graphs) {
            if (graphState.projectIndex < 0 || graphState.projectIndex >= static_cast<int>(m_projects.size())) {
                continue;
            }
            video::Project *project = m_projects[graphState.projectIndex].project.get();
            if (!project) {
                continue;
            }
            if (graphState.fieldIndex < 0 || graphState.fieldIndex >= project->fields().size()) {
                continue;
            }

            Graph1DWindow::Config config;
            config.project = project;
            config.projectIndex = graphState.projectIndex;
            config.fieldIndex = graphState.fieldIndex;
            config.timeIndex = graphState.xMin;
            config.logarithmic = graphState.logarithmic;
            config.gridLines = graphState.gridLines;
            config.color = graphState.color;

            auto *widget = new Graph1DWindow(config);
            if (!graphState.name.isEmpty()) {
                widget->setWindowTitle(graphState.name);
            }
            ui->mdiArea->addSubWindow(widget);
            widget->show();
        }
    }
}

void MainWindow::restoreTwoDimGraphs(const video::VideoSaveState &state)
{
    for (const video::VideoSession2Dim &graphState : state.twoDimGraphs) {
        if (graphState.projectIndex < 0 || graphState.projectIndex >= static_cast<int>(m_projects.size())) {
            continue;
        }
        video::Project *project = m_projects[graphState.projectIndex].project.get();
        if (!project) {
            continue;
        }
        if (graphState.fieldIndex < 0 || graphState.fieldIndex >= project->fields().size()) {
            continue;
        }

        Graph2DWindow::Config config;
        config.project = project;
        config.projectIndex = graphState.projectIndex;
        config.fieldIndex = graphState.fieldIndex;
        config.timeIndex = graphState.x1Min;
        config.logarithmic = graphState.logarithmic;
        config.gridLines = graphState.gridLines;

        auto *widget = new Graph2DWindow(config);
        ui->mdiArea->addSubWindow(widget);
        widget->show();
    }
}

void MainWindow::saveSession()
{
    if (m_projects.empty()) {
        QMessageBox::information(this, tr("Save session"), tr("There are no open projects to save."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, tr("Save session"), QString(), tr("Video sessions (*.json);;All files (*.*)"));
    if (filePath.isEmpty()) {
        return;
    }
    if (!filePath.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
        filePath += QStringLiteral(".json");
    }

    video::VideoSaveState state;
    const QDir baseDir = QFileInfo(filePath).absoluteDir();
    for (const OpenProject &entry : m_projects) {
        QString path = entry.sourcePath;
        if (!path.isEmpty()) {
            path = baseDir.relativeFilePath(path);
        }
        state.projects.append(path);
    }

    const auto subWindows = ui->mdiArea->subWindowList();
    for (QMdiSubWindow *subWindow : subWindows) {
        QWidget *widget = subWindow->widget();
        if (auto *graph1D = qobject_cast<Graph1DWindow *>(widget)) {
            video::VideoSession1Dim graph = graph1D->sessionState();
            video::OneDimWindowState windowState;
            windowState.graphs.append(graph);
            state.oneDimGraphs.append(windowState);
        } else if (auto *graph2D = qobject_cast<Graph2DWindow *>(widget)) {
            video::VideoSession2Dim graph = graph2D->sessionState();
            state.twoDimGraphs.append(graph);
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::critical(this, tr("Save session"), tr("Unable to open file for writing."));
        return;
    }

    try {
        video::writeVideoSave(state, &file);
    } catch (const std::exception &ex) {
        QMessageBox::critical(this, tr("Save session"), tr("Failed to write session file:\n%1").arg(QString::fromUtf8(ex.what())));
        return;
    }

    statusBar()->showMessage(tr("Session saved"), 5000);
}

void MainWindow::saveMovie()
{
    QMessageBox::information(this, tr("Save movie"), tr("AVI export is not implemented in this Qt version."));
}

void MainWindow::saveData()
{
    QMessageBox::information(this, tr("Save data"), tr("Data export is not implemented in this Qt version."));
}

void MainWindow::toggleToolbar(bool checked)
{
    if (ui->mainToolBar) {
        ui->mainToolBar->setVisible(checked);
    }
}

void MainWindow::toggleStatusBar(bool checked)
{
    if (ui->statusbar) {
        ui->statusbar->setVisible(checked);
    }
}

void MainWindow::toggleProjectDock(bool checked)
{
    if (ui->projectDock) {
        ui->projectDock->setVisible(checked);
    }
}

void MainWindow::cascadeWindows()
{
    ui->mdiArea->cascadeSubWindows();
}

void MainWindow::tileWindowsHorizontally()
{
    const auto windows = ui->mdiArea->subWindowList();
    if (windows.isEmpty()) {
        return;
    }

    const QRect area = ui->mdiArea->viewport()->rect();
    const int count = windows.size();
    const int height = std::max(1, area.height() / count);

    int y = area.top();
    for (QMdiSubWindow *window : windows) {
        window->setGeometry(area.left(), y, area.width(), height);
        y += height;
    }
}

void MainWindow::tileWindowsVertically()
{
    const auto windows = ui->mdiArea->subWindowList();
    if (windows.isEmpty()) {
        return;
    }

    const QRect area = ui->mdiArea->viewport()->rect();
    const int count = windows.size();
    const int width = std::max(1, area.width() / count);

    int x = area.left();
    for (QMdiSubWindow *window : windows) {
        window->setGeometry(x, area.top(), width, area.height());
        x += width;
    }
}

void MainWindow::arrangeIcons()
{
    ui->mdiArea->arrangeIcons();
}

void MainWindow::closeAllSubWindows()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("About Video"),
                       tr("<h3>Video Analysis</h3><p>Qt port of the original VIDEO application.</p>"));
}

void MainWindow::handleProjectDoubleClick(QListWidgetItem *item)
{
    Q_UNUSED(item);
    createGraph();
}

void MainWindow::exitApplication()
{
    close();
}

