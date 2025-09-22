#include "deleteprojectdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

DeleteProjectDialog::DeleteProjectDialog(const QStringList &projects, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Close project"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);

    auto *label = new QLabel(tr("Select a project to close."), this);
    layout->addWidget(label);

    m_projectList = new QListWidget(this);
    m_projectList->addItems(projects);
    if (!projects.isEmpty()) {
        m_projectList->setCurrentRow(0);
    }
    layout->addWidget(m_projectList);

    m_deleteAllCheck = new QCheckBox(tr("Close all projects"), this);
    connect(m_deleteAllCheck, &QCheckBox::toggled, this, &DeleteProjectDialog::onDeleteAllToggled);
    layout->addWidget(m_deleteAllCheck);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &DeleteProjectDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &DeleteProjectDialog::reject);
    layout->addWidget(m_buttons);
}

bool DeleteProjectDialog::deleteAll() const
{
    return m_deleteAllCheck->isChecked();
}

int DeleteProjectDialog::selectedProjectIndex() const
{
    return m_projectList->currentRow();
}

void DeleteProjectDialog::onDeleteAllToggled(bool checked)
{
    m_projectList->setEnabled(!checked);
}

