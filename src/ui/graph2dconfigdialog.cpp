#include "graph2dconfigdialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>

#include <algorithm>

#include "core/project.h"

namespace {
QString axisLabel(const video::Field &field, int axis)
{
    if (axis < 0 || axis >= field.axes.size()) {
        return QObject::tr("Axis");
    }
    const video::Axis &ax = field.axes.at(axis);
    if (!ax.name.isEmpty()) {
        return ax.name;
    }
    return QObject::tr("Axis %1").arg(axis + 1);
}

void applyColorToButton(QPushButton *button, const QColor &color)
{
    if (!button) {
        return;
    }
    button->setText(color.name(QColor::HexArgb));
    QPalette pal = button->palette();
    pal.setColor(QPalette::Button, color);
    button->setPalette(pal);
    button->setAutoFillBackground(true);
}
}

Graph2DConfigDialog::Graph2DConfigDialog(const std::vector<video::Project *> &projects, QWidget *parent)
    : QDialog(parent)
    , m_projects(projects)
{
    setWindowTitle(tr("Configure 2D graph"));
    setModal(true);
    m_session.palette = 0;
    m_session.gridCount = 10;
    m_session.gridLines = true;
    m_session.autoGrid = false;
    m_session.logarithmic = false;
    m_session.colorMin = QColor(0, 0, 255);
    m_session.colorMax = QColor(255, 0, 0);
    m_session.indices = {0};
    buildUi();
}

void Graph2DConfigDialog::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();
    layout->addLayout(form);

    m_projectCombo = new QComboBox(this);
    for (qsizetype i = 0; i < static_cast<qsizetype>(m_projects.size()); ++i) {
        const video::Project *project = m_projects[i];
        if (project) {
            m_projectCombo->addItem(project->name(), QVariant::fromValue(static_cast<int>(i)));
        }
    }
    connect(m_projectCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph2DConfigDialog::projectChanged);
    form->addRow(tr("Project"), m_projectCombo);

    m_fieldCombo = new QComboBox(this);
    connect(m_fieldCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph2DConfigDialog::fieldChanged);
    form->addRow(tr("Field"), m_fieldCombo);

    auto *axisBox = new QWidget(this);
    auto *axisLayout = new QHBoxLayout(axisBox);
    axisLayout->setContentsMargins(0, 0, 0, 0);
    m_axis1Combo = new QComboBox(axisBox);
    m_axis2Combo = new QComboBox(axisBox);
    connect(m_axis1Combo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph2DConfigDialog::axis1Changed);
    connect(m_axis2Combo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph2DConfigDialog::axis2Changed);
    axisLayout->addWidget(new QLabel(tr("X:"), axisBox));
    axisLayout->addWidget(m_axis1Combo);
    axisLayout->addSpacing(12);
    axisLayout->addWidget(new QLabel(tr("Y:"), axisBox));
    axisLayout->addWidget(m_axis2Combo);
    form->addRow(tr("Axes"), axisBox);

    m_timeSpin = new QSpinBox(this);
    m_timeSpin->setMinimum(0);
    connect(m_timeSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::timeIndexChanged);
    form->addRow(tr("Time sample"), m_timeSpin);

    auto *xRangeWidget = new QWidget(this);
    auto *xRangeLayout = new QHBoxLayout(xRangeWidget);
    xRangeLayout->setContentsMargins(0, 0, 0, 0);
    m_startXSpin = new QSpinBox(xRangeWidget);
    m_endXSpin = new QSpinBox(xRangeWidget);
    m_startXSpin->setMinimum(0);
    m_endXSpin->setMinimum(0);
    connect(m_startXSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::startXChanged);
    connect(m_endXSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::endXChanged);
    xRangeLayout->addWidget(new QLabel(tr("From"), xRangeWidget));
    xRangeLayout->addWidget(m_startXSpin);
    xRangeLayout->addWidget(new QLabel(tr("to"), xRangeWidget));
    xRangeLayout->addWidget(m_endXSpin);
    form->addRow(tr("X index"), xRangeWidget);

    auto *yRangeWidget = new QWidget(this);
    auto *yRangeLayout = new QHBoxLayout(yRangeWidget);
    yRangeLayout->setContentsMargins(0, 0, 0, 0);
    m_startYSpin = new QSpinBox(yRangeWidget);
    m_endYSpin = new QSpinBox(yRangeWidget);
    m_startYSpin->setMinimum(0);
    m_endYSpin->setMinimum(0);
    connect(m_startYSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::startYChanged);
    connect(m_endYSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::endYChanged);
    yRangeLayout->addWidget(new QLabel(tr("From"), yRangeWidget));
    yRangeLayout->addWidget(m_startYSpin);
    yRangeLayout->addWidget(new QLabel(tr("to"), yRangeWidget));
    yRangeLayout->addWidget(m_endYSpin);
    form->addRow(tr("Y index"), yRangeWidget);

    m_paletteCombo = new QComboBox(this);
    m_paletteCombo->addItem(tr("Rainbow"), 0);
    m_paletteCombo->addItem(tr("Warm"), 1);
    m_paletteCombo->addItem(tr("Greyscale"), 2);
    connect(m_paletteCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &Graph2DConfigDialog::paletteChanged);
    form->addRow(tr("Palette"), m_paletteCombo);

    m_colorMinButton = new QPushButton(this);
    m_colorMaxButton = new QPushButton(this);
    connect(m_colorMinButton, &QPushButton::clicked, this, &Graph2DConfigDialog::chooseMinColor);
    connect(m_colorMaxButton, &QPushButton::clicked, this, &Graph2DConfigDialog::chooseMaxColor);
    form->addRow(tr("Low color"), m_colorMinButton);
    form->addRow(tr("High color"), m_colorMaxButton);

    auto *optionsBox = new QGroupBox(tr("Display options"), this);
    auto *optionsLayout = new QFormLayout(optionsBox);
    m_gridCheck = new QCheckBox(tr("Show grid"), optionsBox);
    m_gridCheck->setChecked(true);
    connect(m_gridCheck, &QCheckBox::toggled, this, &Graph2DConfigDialog::updateAxisBounds);
    m_autoGridCheck = new QCheckBox(tr("Auto grid density"), optionsBox);
    m_logCheck = new QCheckBox(tr("Logarithmic scale"), optionsBox);
    optionsLayout->addRow(m_gridCheck);
    optionsLayout->addRow(m_autoGridCheck);
    optionsLayout->addRow(m_logCheck);

    m_gridCountSpin = new QSpinBox(optionsBox);
    m_gridCountSpin->setRange(1, 200);
    m_gridCountSpin->setValue(10);
    optionsLayout->addRow(tr("Grid count"), m_gridCountSpin);

    m_layersSpin = new QSpinBox(optionsBox);
    m_layersSpin->setRange(0, video::kMaxLayers);
    optionsLayout->addRow(tr("Layers"), m_layersSpin);

    m_cloudSpin = new QSpinBox(optionsBox);
    m_cloudSpin->setRange(0, 1000);
    optionsLayout->addRow(tr("Cloud points"), m_cloudSpin);

    m_isoSpin = new QSpinBox(optionsBox);
    m_isoSpin->setRange(0, 1000);
    optionsLayout->addRow(tr("Isolines"), m_isoSpin);

    layout->addWidget(optionsBox);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttonBox);

    applyColorToButton(m_colorMinButton, m_session.colorMin);
    applyColorToButton(m_colorMaxButton, m_session.colorMax);

    connect(m_gridCheck, &QCheckBox::toggled, this, &Graph2DConfigDialog::syncSession);
    connect(m_autoGridCheck, &QCheckBox::toggled, this, &Graph2DConfigDialog::syncSession);
    connect(m_logCheck, &QCheckBox::toggled, this, &Graph2DConfigDialog::syncSession);
    connect(m_gridCountSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::syncSession);
    connect(m_layersSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::syncSession);
    connect(m_cloudSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::syncSession);
    connect(m_isoSpin, qOverload<int>(&QSpinBox::valueChanged), this, &Graph2DConfigDialog::syncSession);

    if (m_projectCombo->count() > 0) {
        populateFieldCombo(m_projectCombo->currentIndex());
        syncSession();
    }
}

void Graph2DConfigDialog::projectChanged(int index)
{
    if (m_updating) {
        return;
    }
    populateFieldCombo(index);
    syncSession();
}

void Graph2DConfigDialog::fieldChanged(int index)
{
    Q_UNUSED(index);
    if (m_updating) {
        return;
    }
    populateAxisCombos();
    updateAxisBounds();
    syncSession();
}

void Graph2DConfigDialog::axis1Changed(int index)
{
    Q_UNUSED(index);
    if (m_updating) {
        return;
    }
    updateAxisBounds();
    syncSession();
}

void Graph2DConfigDialog::axis2Changed(int index)
{
    Q_UNUSED(index);
    if (m_updating) {
        return;
    }
    updateAxisBounds();
    syncSession();
}

void Graph2DConfigDialog::updateAxisBounds()
{
    const int projectIndex = m_projectCombo->currentData().toInt();
    const int fieldIndex = m_fieldCombo->currentData().toInt();
    const int axisX = m_axis1Combo->currentData().toInt();
    const int axisY = m_axis2Combo->currentData().toInt();

    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }
    const video::Project *project = m_projects[projectIndex];
    if (!project || fieldIndex < 0 || fieldIndex >= project->fields().size()) {
        return;
    }
    const video::Field &field = project->fields().at(fieldIndex);

    if (axisX >= 0 && axisX < field.axes.size()) {
        const int count = field.axes.at(axisX).points.size();
        m_startXSpin->setMaximum(std::max(0, count - 1));
        m_endXSpin->setMaximum(std::max(0, count - 1));
        m_startXSpin->setValue(std::min(m_startXSpin->value(), m_startXSpin->maximum()));
        m_endXSpin->setValue(std::min(m_endXSpin->value(), m_endXSpin->maximum()));
    }
    if (axisY >= 0 && axisY < field.axes.size()) {
        const int count = field.axes.at(axisY).points.size();
        m_startYSpin->setMaximum(std::max(0, count - 1));
        m_endYSpin->setMaximum(std::max(0, count - 1));
        m_startYSpin->setValue(std::min(m_startYSpin->value(), m_startYSpin->maximum()));
        m_endYSpin->setValue(std::min(m_endYSpin->value(), m_endYSpin->maximum()));
    }

    const int timeSamples = project->timeValues().size();
    m_timeSpin->setMaximum(std::max(0, timeSamples - 1));
    m_timeSpin->setValue(std::min(m_timeSpin->value(), m_timeSpin->maximum()));
}

void Graph2DConfigDialog::timeIndexChanged(int value)
{
    Q_UNUSED(value);
    syncSession();
}

void Graph2DConfigDialog::startXChanged(int value)
{
    if (value > m_endXSpin->value()) {
        m_endXSpin->setValue(value);
    }
    syncSession();
}

void Graph2DConfigDialog::endXChanged(int value)
{
    if (value < m_startXSpin->value()) {
        m_startXSpin->setValue(value);
    }
    syncSession();
}

void Graph2DConfigDialog::startYChanged(int value)
{
    if (value > m_endYSpin->value()) {
        m_endYSpin->setValue(value);
    }
    syncSession();
}

void Graph2DConfigDialog::endYChanged(int value)
{
    if (value < m_startYSpin->value()) {
        m_startYSpin->setValue(value);
    }
    syncSession();
}

void Graph2DConfigDialog::chooseMinColor()
{
    const QColor color = QColorDialog::getColor(m_session.colorMin, this, tr("Select low color"));
    if (!color.isValid()) {
        return;
    }
    m_session.colorMin = color;
    applyColorToButton(m_colorMinButton, color);
}

void Graph2DConfigDialog::chooseMaxColor()
{
    const QColor color = QColorDialog::getColor(m_session.colorMax, this, tr("Select high color"));
    if (!color.isValid()) {
        return;
    }
    m_session.colorMax = color;
    applyColorToButton(m_colorMaxButton, color);
}

void Graph2DConfigDialog::paletteChanged(int index)
{
    Q_UNUSED(index);
    syncSession();
}

void Graph2DConfigDialog::populateFieldCombo(int projectComboIndex)
{
    const int projectIndex = m_projectCombo->itemData(projectComboIndex).toInt();
    m_fieldCombo->clear();

    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }
    const video::Project *project = m_projects[projectIndex];
    if (!project) {
        return;
    }

    for (int i = 0; i < project->fields().size(); ++i) {
        const video::Field &field = project->fields().at(i);
        if (field.axes.size() >= 2) {
            QString name = field.name;
            if (name.isEmpty()) {
                name = tr("Field %1").arg(i + 1);
            }
            m_fieldCombo->addItem(name, i);
        }
    }
    if (m_fieldCombo->count() == 0) {
        m_fieldCombo->addItem(tr("No 2D-compatible fields"), -1);
        m_fieldCombo->setEnabled(false);
    } else {
        m_fieldCombo->setEnabled(true);
    }
    populateAxisCombos();
    updateAxisBounds();
}

void Graph2DConfigDialog::populateAxisCombos()
{
    const int projectIndex = m_projectCombo->currentData().toInt();
    const int fieldIndex = m_fieldCombo->currentData().toInt();
    m_axis1Combo->clear();
    m_axis2Combo->clear();

    if (projectIndex < 0 || projectIndex >= static_cast<int>(m_projects.size())) {
        return;
    }
    const video::Project *project = m_projects[projectIndex];
    if (!project || fieldIndex < 0 || fieldIndex >= project->fields().size()) {
        return;
    }
    const video::Field &field = project->fields().at(fieldIndex);

    for (int axis = 0; axis < field.axes.size(); ++axis) {
        const QString label = axisLabel(field, axis);
        m_axis1Combo->addItem(label, axis);
        m_axis2Combo->addItem(label, axis);
    }

    if (m_axis1Combo->count() > 0) {
        m_axis1Combo->setCurrentIndex(0);
    }
    if (m_axis2Combo->count() > 1) {
        m_axis2Combo->setCurrentIndex(1);
    }
}

void Graph2DConfigDialog::syncSession()
{
    const int projectIndex = m_projectCombo->currentData().toInt();
    m_session.projectIndex = projectIndex;
    if (projectIndex >= 0 && projectIndex < static_cast<int>(m_projects.size())) {
        m_session.project = m_projects[projectIndex];
    } else {
        m_session.project = nullptr;
    }

    m_session.fieldIndex = m_fieldCombo->currentData().toInt();
    m_session.argument1 = m_axis1Combo->currentData().toInt();
    m_session.argument2 = m_axis2Combo->currentData().toInt();
    const int timeIndex = m_timeSpin->value();
    if (m_session.indices.isEmpty()) {
        m_session.indices.append(timeIndex);
    } else {
        m_session.indices[0] = timeIndex;
    }
    m_session.x1Min = m_startXSpin->value();
    m_session.x1Max = m_endXSpin->value();
    m_session.x2Min = m_startYSpin->value();
    m_session.x2Max = m_endYSpin->value();
    m_session.gridLines = m_gridCheck->isChecked();
    m_session.autoGrid = m_autoGridCheck->isChecked();
    m_session.logarithmic = m_logCheck->isChecked();
    m_session.gridCount = m_gridCountSpin->value();
    m_session.layers = m_layersSpin->value();
    m_session.cloud = m_cloudSpin->value();
    m_session.isolines = m_isoSpin->value();
    m_session.palette = m_paletteCombo->currentData().toInt();
}

void Graph2DConfigDialog::setInitialState(const video::VideoSession2Dim &state)
{
    m_session = state;
    m_updating = true;

    int projectComboIndex = 0;
    for (int i = 0; i < m_projectCombo->count(); ++i) {
        if (m_projectCombo->itemData(i).toInt() == state.projectIndex) {
            projectComboIndex = i;
            break;
        }
    }
    m_projectCombo->setCurrentIndex(projectComboIndex);
    populateFieldCombo(projectComboIndex);

    int fieldComboIndex = 0;
    for (int i = 0; i < m_fieldCombo->count(); ++i) {
        if (m_fieldCombo->itemData(i).toInt() == state.fieldIndex) {
            fieldComboIndex = i;
            break;
        }
    }
    m_fieldCombo->setCurrentIndex(fieldComboIndex);
    populateAxisCombos();

    int axis1Index = 0;
    for (int i = 0; i < m_axis1Combo->count(); ++i) {
        if (m_axis1Combo->itemData(i).toInt() == state.argument1) {
            axis1Index = i;
            break;
        }
    }
    m_axis1Combo->setCurrentIndex(axis1Index);

    int axis2Index = 0;
    for (int i = 0; i < m_axis2Combo->count(); ++i) {
        if (m_axis2Combo->itemData(i).toInt() == state.argument2) {
            axis2Index = i;
            break;
        }
    }
    m_axis2Combo->setCurrentIndex(axis2Index);

    m_timeSpin->setValue(state.indices.isEmpty() ? 0 : state.indices.first());
    m_startXSpin->setValue(state.x1Min);
    m_endXSpin->setValue(state.x1Max);
    m_startYSpin->setValue(state.x2Min);
    m_endYSpin->setValue(state.x2Max);
    m_gridCheck->setChecked(state.gridLines);
    m_autoGridCheck->setChecked(state.autoGrid);
    m_logCheck->setChecked(state.logarithmic);
    m_gridCountSpin->setValue(state.gridCount);
    m_layersSpin->setValue(state.layers);
    m_cloudSpin->setValue(state.cloud);
    m_isoSpin->setValue(state.isolines);
    m_paletteCombo->setCurrentIndex(m_paletteCombo->findData(state.palette));
    applyColorToButton(m_colorMinButton, state.colorMin);
    applyColorToButton(m_colorMaxButton, state.colorMax);

    updateAxisBounds();

    m_updating = false;
}

Graph2DConfigDialog::Result Graph2DConfigDialog::result() const
{
    Graph2DConfigDialog::Result res;
    res.session = m_session;
    return res;
}

void Graph2DConfigDialog::accept()
{
    if (m_fieldCombo->currentData().toInt() < 0) {
        QMessageBox::warning(this, tr("Configure 2D graph"), tr("Select a 2D-compatible field."));
        return;
    }
    syncSession();
    QDialog::accept();
}

