#include "twodiagraphdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QStringList>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QVBoxLayout>

#include "core/project.h"

namespace {
QString buildDescription(const video::Field &field)
{
    QStringList axesDescriptions;
    for (const auto &axis : field.axes) {
        axesDescriptions.append(QStringLiteral("%1 (%2 pts)").arg(axis.name).arg(axis.points.size()));
    }
    return axesDescriptions.join(QStringLiteral(", "));
}
}

TwoDimGraphDialog::TwoDimGraphDialog(const std::vector<video::Project*> &projects, QWidget *parent)
    : QDialog(parent)
    , m_projects(projects)
{
    setWindowTitle(tr("New 2D graph"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    m_projectCombo = new QComboBox(this);
    for (qsizetype i = 0; i < static_cast<qsizetype>(projects.size()); ++i) {
        if (projects[i]) {
            m_projectCombo->addItem(projects[i]->name(), QVariant::fromValue(static_cast<int>(i)));
        }
    }
    connect(m_projectCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &TwoDimGraphDialog::updateFieldsForProject);
    formLayout->addRow(tr("Project:"), m_projectCombo);

    m_fieldCombo = new QComboBox(this);
    connect(m_fieldCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &TwoDimGraphDialog::updateFieldInfo);
    formLayout->addRow(tr("Field:"), m_fieldCombo);

    m_timeIndexSpin = new QSpinBox(this);
    m_timeIndexSpin->setMinimum(0);
    formLayout->addRow(tr("Time sample:"), m_timeIndexSpin);

    m_fieldInfo = new QLabel(this);
    m_fieldInfo->setWordWrap(true);
    m_fieldInfo->setMinimumWidth(240);
    formLayout->addRow(tr("Structure:"), m_fieldInfo);

    layout->addLayout(formLayout);

    m_logarithmic = new QCheckBox(tr("Logarithmic scale"), this);
    m_gridLines = new QCheckBox(tr("Show grid lines"), this);

    layout->addWidget(m_logarithmic);
    layout->addWidget(m_gridLines);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &TwoDimGraphDialog::acceptDialog);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &TwoDimGraphDialog::reject);
    layout->addWidget(m_buttons);

    if (!projects.empty()) {
        m_projectCombo->setCurrentIndex(0);
    }
}

void TwoDimGraphDialog::updateFieldsForProject(int)
{
    m_fieldCombo->clear();
    m_fieldInfo->clear();

    const int projectIndex = m_projectCombo->currentData().toInt();
    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }

    const video::Project *project = m_projects[projectIndex];
    if (!project) {
        return;
    }

    int suitableFields = 0;
    for (int fieldIndex = 0; fieldIndex < project->fields().size(); ++fieldIndex) {
        const video::Field &field = project->fields()[fieldIndex];
        if (field.axes.size() >= 2) {
            const QString name = field.name.isEmpty() ? tr("Field %1").arg(fieldIndex + 1) : field.name;
            m_fieldCombo->addItem(name, fieldIndex);
            ++suitableFields;
        }
    }

    if (suitableFields == 0) {
        m_fieldCombo->addItem(tr("No 2D-compatible fields"), -1);
        m_fieldCombo->setEnabled(false);
    } else {
        m_fieldCombo->setEnabled(true);
        m_fieldCombo->setCurrentIndex(0);
    }

    const int timeSamples = project->timeValues().size();
    m_timeIndexSpin->setMaximum(timeSamples > 0 ? timeSamples - 1 : 0);
    m_timeIndexSpin->setEnabled(timeSamples > 1);
}

void TwoDimGraphDialog::updateFieldInfo(int index)
{
    if (index < 0) {
        m_fieldInfo->clear();
        return;
    }

    const int projectIndex = m_projectCombo->currentData().toInt();
    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        m_fieldInfo->clear();
        return;
    }

    const video::Project *project = m_projects[projectIndex];
    const int fieldIndex = m_fieldCombo->itemData(index).toInt();
    if (!project || fieldIndex < 0 || fieldIndex >= project->fields().size()) {
        m_fieldInfo->clear();
        return;
    }

    const video::Field &field = project->fields()[fieldIndex];
    m_fieldInfo->setText(buildDescription(field));
}

void TwoDimGraphDialog::acceptDialog()
{
    if (!m_fieldCombo->isEnabled() || m_fieldCombo->count() == 0) {
        QMessageBox::warning(this, tr("Cannot create graph"), tr("Selected project does not contain 2D fields."));
        return;
    }

    const int projectIndex = m_projectCombo->currentData().toInt();
    const int fieldIndex = m_fieldCombo->currentData().toInt();
    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size()) || fieldIndex < 0) {
        QMessageBox::warning(this, tr("Cannot create graph"), tr("Please select a valid field."));
        return;
    }

    m_result.project = m_projects[projectIndex];
    m_result.projectIndex = projectIndex;
    m_result.fieldIndex = fieldIndex;
    m_result.timeIndex = m_timeIndexSpin->value();
    m_result.logarithmic = m_logarithmic->isChecked();
    m_result.gridLines = m_gridLines->isChecked();

    accept();
}

