#pragma once

#include <QDialog>

namespace video {
class Project;
}

class QComboBox;
class QSpinBox;
class QCheckBox;
class QLabel;
class QDialogButtonBox;

class TwoDimGraphDialog : public QDialog
{
    Q_OBJECT

public:
    struct Result {
        video::Project *project = nullptr;
        int projectIndex = -1;
        int fieldIndex = -1;
        int timeIndex = 0;
        bool logarithmic = false;
        bool gridLines = false;
    };

    TwoDimGraphDialog(const std::vector<video::Project*> &projects, QWidget *parent = nullptr);

    Result result() const { return m_result; }

private slots:
    void updateFieldsForProject(int index);
    void acceptDialog();
    void updateFieldInfo(int index);

private:
    const std::vector<video::Project*> m_projects;

    QComboBox *m_projectCombo = nullptr;
    QComboBox *m_fieldCombo = nullptr;
    QSpinBox *m_timeIndexSpin = nullptr;
    QCheckBox *m_logarithmic = nullptr;
    QCheckBox *m_gridLines = nullptr;
    QLabel *m_fieldInfo = nullptr;
    QDialogButtonBox *m_buttons = nullptr;

    Result m_result;
};

