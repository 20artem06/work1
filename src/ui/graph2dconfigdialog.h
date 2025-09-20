#pragma once

#include <QDialog>
#include <vector>

#include "core/session.h"

namespace video {
class Project;
}

class QComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;
class QDialogButtonBox;

class Graph2DConfigDialog : public QDialog
{
    Q_OBJECT

public:
    struct Result {
        video::VideoSession2Dim session;
    };

    Graph2DConfigDialog(const std::vector<video::Project *> &projects, QWidget *parent = nullptr);

    void setInitialState(const video::VideoSession2Dim &state);
    Result result() const;

protected:
    void accept() override;

private slots:
    void projectChanged(int index);
    void fieldChanged(int index);
    void axis1Changed(int index);
    void axis2Changed(int index);
    void updateAxisBounds();
    void timeIndexChanged(int value);
    void startXChanged(int value);
    void endXChanged(int value);
    void startYChanged(int value);
    void endYChanged(int value);
    void chooseMinColor();
    void chooseMaxColor();
    void paletteChanged(int index);

private:
    void buildUi();
    void populateFieldCombo(int projectComboIndex);
    void populateAxisCombos();
    void syncSession();

    const std::vector<video::Project *> m_projects;
    video::VideoSession2Dim m_session;

    QComboBox *m_projectCombo = nullptr;
    QComboBox *m_fieldCombo = nullptr;
    QComboBox *m_axis1Combo = nullptr;
    QComboBox *m_axis2Combo = nullptr;
    QSpinBox *m_timeSpin = nullptr;
    QSpinBox *m_startXSpin = nullptr;
    QSpinBox *m_endXSpin = nullptr;
    QSpinBox *m_startYSpin = nullptr;
    QSpinBox *m_endYSpin = nullptr;
    QSpinBox *m_gridCountSpin = nullptr;
    QSpinBox *m_layersSpin = nullptr;
    QSpinBox *m_cloudSpin = nullptr;
    QSpinBox *m_isoSpin = nullptr;
    QPushButton *m_colorMinButton = nullptr;
    QPushButton *m_colorMaxButton = nullptr;
    QComboBox *m_paletteCombo = nullptr;
    QCheckBox *m_gridCheck = nullptr;
    QCheckBox *m_autoGridCheck = nullptr;
    QCheckBox *m_logCheck = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;

    bool m_updating = false;
};

