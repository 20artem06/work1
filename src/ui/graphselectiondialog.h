#pragma once

#include <QDialog>

class QRadioButton;
class QDialogButtonBox;

class GraphSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    enum class GraphType {
        OneDimensional,
        TwoDimensional
    };

    explicit GraphSelectionDialog(QWidget *parent = nullptr);

    GraphType selectedType() const;

private:
    QRadioButton *m_oneDimensionalButton = nullptr;
    QRadioButton *m_twoDimensionalButton = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};

