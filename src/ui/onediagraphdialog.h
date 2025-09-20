#pragma once

#include <QDialog>
#include <QVector>

namespace video {
class Project;
}

class QComboBox;
class QSpinBox;
class QCheckBox;
class QDialogButtonBox;
class QLabel;

class OneDimGraphDialog : public QDialog
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

    OneDimGraphDialog(const std::vector<video::Project*> &projects, QWidget *parent = nullptr);

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
    QCheckBox *m_logScaleCheck = nullptr;
    QCheckBox *m_gridCheck = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
    QLabel *m_fieldInfo = nullptr;

    Result m_result;
};

