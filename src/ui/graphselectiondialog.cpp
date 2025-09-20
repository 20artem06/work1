#include "graphselectiondialog.h"

#include <QDialogButtonBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>

GraphSelectionDialog::GraphSelectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select graph type"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);

    auto *label = new QLabel(tr("Choose the type of graph you want to create:"), this);
    label->setWordWrap(true);
    layout->addWidget(label);

    m_oneDimensionalButton = new QRadioButton(tr("1D graph"), this);
    m_twoDimensionalButton = new QRadioButton(tr("2D graph"), this);
    m_oneDimensionalButton->setChecked(true);

    layout->addWidget(m_oneDimensionalButton);
    layout->addWidget(m_twoDimensionalButton);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &GraphSelectionDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &GraphSelectionDialog::reject);
    layout->addWidget(m_buttonBox);
}

GraphSelectionDialog::GraphType GraphSelectionDialog::selectedType() const
{
    return m_twoDimensionalButton->isChecked() ? GraphType::TwoDimensional : GraphType::OneDimensional;
}

