#pragma once

#include <QDialog>
#include <QVector>
#include <vector>

#include "core/session.h"

namespace video {
class Project;
}

class QListWidget;
class QPushButton;
class QComboBox;
class QSpinBox;
class QLineEdit;
class QDialogButtonBox;
class QCheckBox;
class QDoubleSpinBox;

class Graph1DConfigDialog : public QDialog
{
    Q_OBJECT

public:
    struct Result {
        video::OneDimWindowState windowState;
    };

    Graph1DConfigDialog(const std::vector<video::Project *> &projects, QWidget *parent = nullptr);

    void setInitialState(const video::OneDimWindowState &state);
    Result result() const;

protected:
    void accept() override;

private slots:
    void addSeries();
    void removeSeries();
    void currentSeriesChanged(int row);
    void projectChanged(int index);
    void fieldChanged(int index);
    void timeIndexChanged(int value);
    void argumentIndexChanged(int index);
    void startIndexChanged(int value);
    void endIndexChanged(int value);
    void chooseColor();
    void styleChanged(int index);
    void nameEdited(const QString &name);
    void toggleLogarithmic(bool checked);
    void toggleGridLines(bool checked);
    void axisModeChanged(int index);
    void xAxisMinChanged(double value);
    void xAxisMaxChanged(double value);
    void yAxisMinChanged(double value);
    void yAxisMaxChanged(double value);

private:
    struct SeriesWidgets {
        QComboBox *projectCombo = nullptr;
        QComboBox *fieldCombo = nullptr;
        QComboBox *argumentCombo = nullptr;
        QSpinBox *timeSpin = nullptr;
        QSpinBox *startSpin = nullptr;
        QSpinBox *endSpin = nullptr;
        QPushButton *colorButton = nullptr;
        QComboBox *styleCombo = nullptr;
        QLineEdit *nameEdit = nullptr;
        QCheckBox *logCheck = nullptr;
    };

    void buildUi();
    void refreshSeriesList();
    void loadSeriesIntoEditor(int index);
    void applyEditorToSeries(int index);
    void updateFieldComboForSeries(int seriesIndex, int projectComboIndex);
    void updateArgumentCombo(int seriesIndex);
    void updateIndexSpinBounds(int seriesIndex);
    int ensureValidProjectIndex() const;

    const std::vector<video::Project *> m_projects;
    QVector<video::VideoSession1Dim> m_series;
    video::OneDimWindowState m_windowState;

    QListWidget *m_seriesList = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_removeButton = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    QCheckBox *m_gridCheck = nullptr;
    QComboBox *m_axisModeCombo = nullptr;
    QDoubleSpinBox *m_xMinSpin = nullptr;
    QDoubleSpinBox *m_xMaxSpin = nullptr;
    QDoubleSpinBox *m_yMinSpin = nullptr;
    QDoubleSpinBox *m_yMaxSpin = nullptr;

    SeriesWidgets m_seriesWidgets;
    bool m_updatingUi = false;
};

