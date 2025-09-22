#pragma once

#include <QDialog>

class QListWidget;
class QCheckBox;
class QDialogButtonBox;

class DeleteProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteProjectDialog(const QStringList &projects, QWidget *parent = nullptr);

    bool deleteAll() const;
    int selectedProjectIndex() const;

private slots:
    void onDeleteAllToggled(bool checked);

private:
    QListWidget *m_projectList = nullptr;
    QCheckBox *m_deleteAllCheck = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
};

