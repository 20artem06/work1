#include "graph1dconfigdialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>

#include <algorithm>

#include "core/project.h"
#include "core/math_utils.h"

namespace {
QString seriesDisplayName(const video::VideoSession1Dim &series, int index)
{
    if (!series.name.isEmpty()) {
        return series.name;
    }
    return QObject::tr("Series %1").arg(index + 1);
}

int findFirstOneDimField(const video::Project *project)
{
    if (!project) {
        return -1;
    }
    for (int i = 0; i < project->fields().size(); ++i) {
        if (project->fields().at(i).axes.size() >= 1) {
            return i;
        }
    }
    return -1;
}

QColor defaultSeriesColor(int index)
{
    static const QColor palette[] = {
        QColor(0, 102, 204),
        QColor(204, 51, 51),
        QColor(0, 153, 102),
        QColor(204, 102, 0),
        QColor(102, 0, 153),
    };
    return palette[index % (sizeof(palette) / sizeof(palette[0]))];
}
}

Graph1DConfigDialog::Graph1DConfigDialog(const std::vector<video::Project *> &projects, QWidget *parent)
    : QDialog(parent)
    , m_projects(projects)
{
    setWindowTitle(tr("Configure 1D graph"));
    setModal(true);
    m_windowState.axisStatus = 1; // auto by default
    m_windowState.x1Min = 0.0f;
    m_windowState.x1Max = 0.0f;
    m_windowState.x2Min = 0.0f;
    m_windowState.x2Max = 0.0f;
    buildUi();
}

void Graph1DConfigDialog::buildUi()
{
    auto *layout = new QVBoxLayout(this);

    auto *contentLayout = new QHBoxLayout();
    layout->addLayout(contentLayout);

    auto *leftLayout = new QVBoxLayout();
    contentLayout->addLayout(leftLayout);

    m_seriesList = new QListWidget(this);
    m_seriesList->setSelectionMode(QAbstractItemView::SingleSelection);
    leftLayout->addWidget(new QLabel(tr("Series:"), this));
    leftLayout->addWidget(m_seriesList);

    auto *buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton(tr("Add"), this);
    m_removeButton = new QPushButton(tr("Remove"), this);
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_removeButton);
    leftLayout->addLayout(buttonLayout);

    auto *seriesLayout = new QFormLayout();
    contentLayout->addLayout(seriesLayout, /*stretch*/ 1);

    m_seriesWidgets.projectCombo = new QComboBox(this);
    for (qsizetype i = 0; i < static_cast<qsizetype>(m_projects.size()); ++i) {
        const video::Project *project = m_projects[i];
        if (project) {
            m_seriesWidgets.projectCombo->addItem(project->name(), QVariant::fromValue(static_cast<int>(i)));
        }
    }
    connect(m_seriesWidgets.projectCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph1DConfigDialog::projectChanged);
    seriesLayout->addRow(tr("Project"), m_seriesWidgets.projectCombo);

    m_seriesWidgets.fieldCombo = new QComboBox(this);
    connect(m_seriesWidgets.fieldCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph1DConfigDialog::fieldChanged);
    seriesLayout->addRow(tr("Field"), m_seriesWidgets.fieldCombo);

    m_seriesWidgets.argumentCombo = new QComboBox(this);
    connect(m_seriesWidgets.argumentCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph1DConfigDialog::argumentIndexChanged);
    seriesLayout->addRow(tr("Axis"), m_seriesWidgets.argumentCombo);

    m_seriesWidgets.timeSpin = new QSpinBox(this);
    m_seriesWidgets.timeSpin->setMinimum(0);
    connect(m_seriesWidgets.timeSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph1DConfigDialog::timeIndexChanged);
    seriesLayout->addRow(tr("Time sample"), m_seriesWidgets.timeSpin);

    m_seriesWidgets.startSpin = new QSpinBox(this);
    m_seriesWidgets.startSpin->setMinimum(0);
    connect(m_seriesWidgets.startSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph1DConfigDialog::startIndexChanged);
    seriesLayout->addRow(tr("Start index"), m_seriesWidgets.startSpin);

    m_seriesWidgets.endSpin = new QSpinBox(this);
    m_seriesWidgets.endSpin->setMinimum(0);
    connect(m_seriesWidgets.endSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph1DConfigDialog::endIndexChanged);
    seriesLayout->addRow(tr("End index"), m_seriesWidgets.endSpin);

    m_seriesWidgets.colorButton = new QPushButton(tr("Color"), this);
    connect(m_seriesWidgets.colorButton, &QPushButton::clicked, this, &Graph1DConfigDialog::chooseColor);
    seriesLayout->addRow(tr("Color"), m_seriesWidgets.colorButton);

    m_seriesWidgets.styleCombo = new QComboBox(this);
    for (int i = 0; i < static_cast<int>(video::kPenStyles.size()); ++i) {
        m_seriesWidgets.styleCombo->addItem(tr("Style %1").arg(i + 1), i);
    }
    connect(m_seriesWidgets.styleCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph1DConfigDialog::styleChanged);
    seriesLayout->addRow(tr("Pen style"), m_seriesWidgets.styleCombo);

    m_seriesWidgets.nameEdit = new QLineEdit(this);
    connect(m_seriesWidgets.nameEdit, &QLineEdit::textChanged, this, &Graph1DConfigDialog::nameEdited);
    seriesLayout->addRow(tr("Display name"), m_seriesWidgets.nameEdit);

    m_seriesWidgets.logCheck = new QCheckBox(tr("Logarithmic scale"), this);
    connect(m_seriesWidgets.logCheck, &QCheckBox::toggled, this, &Graph1DConfigDialog::toggleLogarithmic);
    seriesLayout->addRow(QString(), m_seriesWidgets.logCheck);

    m_gridCheck = new QCheckBox(tr("Show grid lines"), this);
    connect(m_gridCheck, &QCheckBox::toggled, this, &Graph1DConfigDialog::toggleGridLines);
    layout->addWidget(m_gridCheck);

    auto *axisBox = new QGroupBox(tr("Axis options"), this);
    auto *axisLayout = new QFormLayout(axisBox);
    m_axisModeCombo = new QComboBox(axisBox);
    m_axisModeCombo->addItem(tr("Fixed ranges"), 0);
    m_axisModeCombo->addItem(tr("Auto fit"), 1);
    m_axisModeCombo->addItem(tr("Copy from first series"), 2);
    connect(m_axisModeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph1DConfigDialog::axisModeChanged);
    axisLayout->addRow(tr("Mode"), m_axisModeCombo);

    m_xMinSpin = new QDoubleSpinBox(axisBox);
    m_xMinSpin->setDecimals(4);
    m_xMinSpin->setRange(-1e9, 1e9);
    connect(m_xMinSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Graph1DConfigDialog::xAxisMinChanged);
    axisLayout->addRow(tr("X min"), m_xMinSpin);

    m_xMaxSpin = new QDoubleSpinBox(axisBox);
    m_xMaxSpin->setDecimals(4);
    m_xMaxSpin->setRange(-1e9, 1e9);
    connect(m_xMaxSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Graph1DConfigDialog::xAxisMaxChanged);
    axisLayout->addRow(tr("X max"), m_xMaxSpin);

    m_yMinSpin = new QDoubleSpinBox(axisBox);
    m_yMinSpin->setDecimals(4);
    m_yMinSpin->setRange(-1e9, 1e9);
    connect(m_yMinSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Graph1DConfigDialog::yAxisMinChanged);
    axisLayout->addRow(tr("Y min"), m_yMinSpin);

    m_yMaxSpin = new QDoubleSpinBox(axisBox);
    m_yMaxSpin->setDecimals(4);
    m_yMaxSpin->setRange(-1e9, 1e9);
    connect(m_yMaxSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Graph1DConfigDialog::yAxisMaxChanged);
    axisLayout->addRow(tr("Y max"), m_yMaxSpin);

    const int autoIndex = m_axisModeCombo->findData(1);
    if (autoIndex >= 0) {
        m_axisModeCombo->setCurrentIndex(autoIndex);
        axisModeChanged(autoIndex);
    }

    layout->addWidget(axisBox);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttonBox);

    connect(m_seriesList, &QListWidget::currentRowChanged, this, &Graph1DConfigDialog::currentSeriesChanged);
    connect(m_addButton, &QPushButton::clicked, this, &Graph1DConfigDialog::addSeries);
    connect(m_removeButton, &QPushButton::clicked, this, &Graph1DConfigDialog::removeSeries);
}

int Graph1DConfigDialog::ensureValidProjectIndex() const
{
    for (int i = 0; i < static_cast<int>(m_projects.size()); ++i) {
        const video::Project *project = m_projects[i];
        if (project && findFirstOneDimField(project) >= 0) {
            return i;
        }
    }
    return -1;
}

void Graph1DConfigDialog::addSeries()
{
    if (m_projects.empty()) {
        QMessageBox::warning(this, tr("Cannot add series"), tr("No projects are loaded."));
        return;
    }

    const int projectIndex = ensureValidProjectIndex();
    if (projectIndex < 0) {
        QMessageBox::warning(this, tr("Cannot add series"), tr("None of the open projects contain 1D fields."));
        return;
    }

    video::VideoSession1Dim series;
    series.projectIndex = projectIndex;
    series.project = m_projects[projectIndex];
    series.fieldIndex = findFirstOneDimField(series.project);
    series.argumentIndex = 0;
    series.xMin = 0;
    series.xMax = 0;
    series.style = 0;
    series.gridLines = m_gridCheck->isChecked();
    series.logarithmic = false;
    series.indices = {0};
    series.color = defaultSeriesColor(m_series.size());
    series.name.clear();

    if (series.project && series.fieldIndex >= 0) {
        const auto &field = series.project->fields().at(series.fieldIndex);
        if (!field.axes.isEmpty()) {
            const int pointCount = field.axes.at(series.argumentIndex).points.size();
            series.xMax = pointCount > 0 ? pointCount - 1 : 0;
        }
    }

    m_series.append(series);
    refreshSeriesList();
    m_seriesList->setCurrentRow(m_series.size() - 1);
}

void Graph1DConfigDialog::removeSeries()
{
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    m_series.removeAt(row);
    refreshSeriesList();
    if (!m_series.isEmpty()) {
        m_seriesList->setCurrentRow(std::min(row, m_series.size() - 1));
    }
    m_removeButton->setEnabled(!m_series.isEmpty());
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!m_series.isEmpty());
}

void Graph1DConfigDialog::refreshSeriesList()
{
    m_seriesList->clear();
    for (int i = 0; i < m_series.size(); ++i) {
        const QString name = seriesDisplayName(m_series.at(i), i);
        m_seriesList->addItem(name);
    }
    m_removeButton->setEnabled(!m_series.isEmpty());
    if (m_buttonBox && m_buttonBox->button(QDialogButtonBox::Ok)) {
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!m_series.isEmpty());
    }
}

void Graph1DConfigDialog::currentSeriesChanged(int row)
{
    if (row < 0 || row >= m_series.size()) {
        m_seriesWidgets.projectCombo->setEnabled(false);
        m_seriesWidgets.fieldCombo->setEnabled(false);
        m_seriesWidgets.argumentCombo->setEnabled(false);
        m_seriesWidgets.timeSpin->setEnabled(false);
        m_seriesWidgets.startSpin->setEnabled(false);
        m_seriesWidgets.endSpin->setEnabled(false);
        m_seriesWidgets.colorButton->setEnabled(false);
        m_seriesWidgets.styleCombo->setEnabled(false);
        m_seriesWidgets.nameEdit->setEnabled(false);
        m_seriesWidgets.logCheck->setEnabled(false);
        return;
    }

    m_seriesWidgets.projectCombo->setEnabled(true);
    m_seriesWidgets.fieldCombo->setEnabled(true);
    m_seriesWidgets.argumentCombo->setEnabled(true);
    m_seriesWidgets.timeSpin->setEnabled(true);
    m_seriesWidgets.startSpin->setEnabled(true);
    m_seriesWidgets.endSpin->setEnabled(true);
    m_seriesWidgets.colorButton->setEnabled(true);
    m_seriesWidgets.styleCombo->setEnabled(true);
    m_seriesWidgets.nameEdit->setEnabled(true);
    m_seriesWidgets.logCheck->setEnabled(true);

    loadSeriesIntoEditor(row);
}

void Graph1DConfigDialog::loadSeriesIntoEditor(int index)
{
    if (index < 0 || index >= m_series.size()) {
        return;
    }
    m_updatingUi = true;
    const video::VideoSession1Dim &series = m_series.at(index);

    int projectComboIndex = 0;
    for (int i = 0; i < m_seriesWidgets.projectCombo->count(); ++i) {
        if (m_seriesWidgets.projectCombo->itemData(i).toInt() == series.projectIndex) {
            projectComboIndex = i;
            break;
        }
    }
    m_seriesWidgets.projectCombo->setCurrentIndex(projectComboIndex);
    updateFieldComboForSeries(index, projectComboIndex);

    int fieldComboIndex = 0;
    for (int i = 0; i < m_seriesWidgets.fieldCombo->count(); ++i) {
        if (m_seriesWidgets.fieldCombo->itemData(i).toInt() == series.fieldIndex) {
            fieldComboIndex = i;
            break;
        }
    }
    m_seriesWidgets.fieldCombo->setCurrentIndex(fieldComboIndex);
    updateArgumentCombo(index);

    int argumentComboIndex = 0;
    for (int i = 0; i < m_seriesWidgets.argumentCombo->count(); ++i) {
        if (m_seriesWidgets.argumentCombo->itemData(i).toInt() == series.argumentIndex) {
            argumentComboIndex = i;
            break;
        }
    }
    m_seriesWidgets.argumentCombo->setCurrentIndex(argumentComboIndex);
    updateIndexSpinBounds(index);

    const int timeIndex = series.indices.isEmpty() ? 0 : series.indices.first();
    m_seriesWidgets.timeSpin->setValue(timeIndex);
    m_seriesWidgets.startSpin->setValue(series.xMin);
    m_seriesWidgets.endSpin->setValue(series.xMax);

    m_seriesWidgets.colorButton->setText(series.color.name(QColor::HexArgb));
    QPalette pal = m_seriesWidgets.colorButton->palette();
    pal.setColor(QPalette::Button, series.color);
    m_seriesWidgets.colorButton->setPalette(pal);
    m_seriesWidgets.colorButton->setAutoFillBackground(true);

    m_seriesWidgets.styleCombo->setCurrentIndex(series.style);
    m_seriesWidgets.nameEdit->setText(series.name);
    m_seriesWidgets.logCheck->setChecked(series.logarithmic);

    m_updatingUi = false;
}

void Graph1DConfigDialog::applyEditorToSeries(int index)
{
    if (m_updatingUi || index < 0 || index >= m_series.size()) {
        return;
    }

    video::VideoSession1Dim &series = m_series[index];
    const int projectComboIndex = m_seriesWidgets.projectCombo->currentIndex();
    const int projectIndex = m_seriesWidgets.projectCombo->itemData(projectComboIndex).toInt();
    series.projectIndex = projectIndex;
    if (projectIndex >= 0 && projectIndex < static_cast<int>(m_projects.size())) {
        series.project = m_projects[projectIndex];
    } else {
        series.project = nullptr;
    }

    const int fieldIndex = m_seriesWidgets.fieldCombo->currentData().toInt();
    series.fieldIndex = fieldIndex;
    series.argumentIndex = m_seriesWidgets.argumentCombo->currentData().toInt();
    const int timeIndex = m_seriesWidgets.timeSpin->value();
    if (series.indices.isEmpty()) {
        series.indices.append(timeIndex);
    } else {
        series.indices[0] = timeIndex;
    }
    series.xMin = m_seriesWidgets.startSpin->value();
    series.xMax = m_seriesWidgets.endSpin->value();
    series.style = m_seriesWidgets.styleCombo->currentData().toInt();
    series.name = m_seriesWidgets.nameEdit->text();
    series.logarithmic = m_seriesWidgets.logCheck->isChecked();
    series.gridLines = m_gridCheck->isChecked();
}

void Graph1DConfigDialog::projectChanged(int index)
{
    if (m_updatingUi) {
        return;
    }
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    updateFieldComboForSeries(row, index);
    applyEditorToSeries(row);
    refreshSeriesList();
}

void Graph1DConfigDialog::fieldChanged(int index)
{
    Q_UNUSED(index);
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    updateArgumentCombo(row);
    updateIndexSpinBounds(row);
    applyEditorToSeries(row);
    refreshSeriesList();
}

void Graph1DConfigDialog::argumentIndexChanged(int index)
{
    Q_UNUSED(index);
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    updateIndexSpinBounds(row);
    applyEditorToSeries(row);
}

void Graph1DConfigDialog::timeIndexChanged(int value)
{
    Q_UNUSED(value);
    const int row = m_seriesList->currentRow();
    applyEditorToSeries(row);
}

void Graph1DConfigDialog::startIndexChanged(int value)
{
    if (m_updatingUi) {
        return;
    }
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    if (value > m_seriesWidgets.endSpin->value()) {
        m_seriesWidgets.endSpin->setValue(value);
    }
    applyEditorToSeries(row);
}

void Graph1DConfigDialog::endIndexChanged(int value)
{
    if (m_updatingUi) {
        return;
    }
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    if (value < m_seriesWidgets.startSpin->value()) {
        m_seriesWidgets.startSpin->setValue(value);
    }
    applyEditorToSeries(row);
}

void Graph1DConfigDialog::chooseColor()
{
    const int row = m_seriesList->currentRow();
    if (row < 0 || row >= m_series.size()) {
        return;
    }
    const QColor chosen = QColorDialog::getColor(m_series[row].color, this, tr("Select color"));
    if (!chosen.isValid()) {
        return;
    }
    m_series[row].color = chosen;
    m_seriesWidgets.colorButton->setText(chosen.name(QColor::HexArgb));
    QPalette pal = m_seriesWidgets.colorButton->palette();
    pal.setColor(QPalette::Button, chosen);
    m_seriesWidgets.colorButton->setPalette(pal);
    m_seriesWidgets.colorButton->setAutoFillBackground(true);
    refreshSeriesList();
}

void Graph1DConfigDialog::styleChanged(int index)
{
    Q_UNUSED(index);
    const int row = m_seriesList->currentRow();
    applyEditorToSeries(row);
}

void Graph1DConfigDialog::nameEdited(const QString &name)
{
    Q_UNUSED(name);
    const int row = m_seriesList->currentRow();
    applyEditorToSeries(row);
    refreshSeriesList();
}

void Graph1DConfigDialog::toggleLogarithmic(bool checked)
{
    Q_UNUSED(checked);
    const int row = m_seriesList->currentRow();
    applyEditorToSeries(row);
}

void Graph1DConfigDialog::toggleGridLines(bool checked)
{
    for (auto &series : m_series) {
        series.gridLines = checked;
    }
}

void Graph1DConfigDialog::axisModeChanged(int index)
{
    m_windowState.axisStatus = m_axisModeCombo->itemData(index).toInt();
    const bool enableRanges = (m_windowState.axisStatus == 0);
    m_xMinSpin->setEnabled(enableRanges);
    m_xMaxSpin->setEnabled(enableRanges);
    m_yMinSpin->setEnabled(enableRanges);
    m_yMaxSpin->setEnabled(enableRanges);
}

void Graph1DConfigDialog::xAxisMinChanged(double value)
{
    m_windowState.x1Min = static_cast<float>(value);
}

void Graph1DConfigDialog::xAxisMaxChanged(double value)
{
    m_windowState.x1Max = static_cast<float>(value);
}

void Graph1DConfigDialog::yAxisMinChanged(double value)
{
    m_windowState.x2Min = static_cast<float>(value);
}

void Graph1DConfigDialog::yAxisMaxChanged(double value)
{
    m_windowState.x2Max = static_cast<float>(value);
}

void Graph1DConfigDialog::updateFieldComboForSeries(int seriesIndex, int projectComboIndex)
{
    if (seriesIndex < 0 || seriesIndex >= m_series.size()) {
        return;
    }

    const int projectIndex = m_seriesWidgets.projectCombo->itemData(projectComboIndex).toInt();
    m_seriesWidgets.fieldCombo->clear();

    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }

    const video::Project *project = m_projects[projectIndex];
    if (!project) {
        return;
    }

    for (int i = 0; i < project->fields().size(); ++i) {
        const video::Field &field = project->fields().at(i);
        if (field.axes.size() >= 1) {
            QString name = field.name;
            if (name.isEmpty()) {
                name = tr("Field %1").arg(i + 1);
            }
            m_seriesWidgets.fieldCombo->addItem(name, i);
        }
    }

    if (m_seriesWidgets.fieldCombo->count() == 0) {
        m_seriesWidgets.fieldCombo->addItem(tr("No 1D-compatible fields"), -1);
        m_seriesWidgets.fieldCombo->setEnabled(false);
    } else {
        m_seriesWidgets.fieldCombo->setEnabled(true);
    }
}

void Graph1DConfigDialog::updateArgumentCombo(int seriesIndex)
{
    if (seriesIndex < 0 || seriesIndex >= m_series.size()) {
        return;
    }

    const int fieldIndex = m_seriesWidgets.fieldCombo->currentData().toInt();
    const int projectComboIndex = m_seriesWidgets.projectCombo->currentIndex();
    const int projectIndex = m_seriesWidgets.projectCombo->itemData(projectComboIndex).toInt();
    m_seriesWidgets.argumentCombo->clear();

    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }
    const video::Project *project = m_projects[projectIndex];
    if (!project || fieldIndex < 0 || fieldIndex >= project->fields().size()) {
        return;
    }

    const video::Field &field = project->fields().at(fieldIndex);
    for (int axis = 0; axis < field.axes.size(); ++axis) {
        QString axisName = field.axes.at(axis).name;
        if (axisName.isEmpty()) {
            axisName = tr("Axis %1").arg(axis + 1);
        }
        m_seriesWidgets.argumentCombo->addItem(axisName, axis);
    }
}

void Graph1DConfigDialog::updateIndexSpinBounds(int seriesIndex)
{
    if (seriesIndex < 0 || seriesIndex >= m_series.size()) {
        return;
    }

    const int projectComboIndex = m_seriesWidgets.projectCombo->currentIndex();
    const int projectIndex = m_seriesWidgets.projectCombo->itemData(projectComboIndex).toInt();
    const int fieldIndex = m_seriesWidgets.fieldCombo->currentData().toInt();
    const int argumentIndex = m_seriesWidgets.argumentCombo->currentData().toInt();

    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }
    const video::Project *project = m_projects[projectIndex];
    if (!project || fieldIndex < 0 || fieldIndex >= project->fields().size()) {
        return;
    }

    const video::Field &field = project->fields().at(fieldIndex);
    if (argumentIndex < 0 || argumentIndex >= field.axes.size()) {
        return;
    }

    const int pointCount = field.axes.at(argumentIndex).points.size();
    m_seriesWidgets.startSpin->setMaximum(std::max(0, pointCount - 1));
    m_seriesWidgets.endSpin->setMaximum(std::max(0, pointCount - 1));

    if (pointCount > 0) {
        video::VideoSession1Dim &series = m_series[seriesIndex];
        series.xMax = std::min(series.xMax, pointCount - 1);
        series.xMin = std::min(series.xMin, series.xMax);
        m_seriesWidgets.startSpin->setValue(series.xMin);
        m_seriesWidgets.endSpin->setValue(series.xMax);
    }

    const int timeSamples = project->timeValues().size();
    m_seriesWidgets.timeSpin->setMaximum(std::max(0, timeSamples - 1));
}

void Graph1DConfigDialog::setInitialState(const video::OneDimWindowState &state)
{
    m_windowState = state;
    m_series = state.graphs;
    refreshSeriesList();
    if (!m_series.isEmpty()) {
        m_seriesList->setCurrentRow(0);
    }
    m_gridCheck->setChecked(!m_series.isEmpty() ? m_series.first().gridLines : false);
    m_axisModeCombo->setCurrentIndex(m_axisModeCombo->findData(m_windowState.axisStatus));
    axisModeChanged(m_axisModeCombo->currentIndex());
    m_xMinSpin->setValue(m_windowState.x1Min);
    m_xMaxSpin->setValue(m_windowState.x1Max);
    m_yMinSpin->setValue(m_windowState.x2Min);
    m_yMaxSpin->setValue(m_windowState.x2Max);
}

Graph1DConfigDialog::Result Graph1DConfigDialog::result() const
{
    Graph1DConfigDialog::Result r;
    r.windowState = m_windowState;
    r.windowState.graphs = m_series;
    return r;
}

void Graph1DConfigDialog::accept()
{
    const int row = m_seriesList ? m_seriesList->currentRow() : -1;
    if (row >= 0) {
        const_cast<Graph1DConfigDialog *>(this)->applyEditorToSeries(row);
    }
    if (m_series.isEmpty()) {
        QMessageBox::warning(this, tr("Configure 1D graph"), tr("Add at least one series."));
        return;
    }
    QDialog::accept();
}

