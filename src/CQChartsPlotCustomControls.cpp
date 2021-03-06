#include <CQChartsPlotCustomControls.h>
#include <CQChartsPlot.h>
#include <CQChartsPlotType.h>
#include <CQChartsPlotParameterEdit.h>
#include <CQChartsColumnControlGroup.h>
#include <CQChartsColumnCombo.h>
#include <CQChartsColorEdit.h>
#include <CQChartsColorRangeSlider.h>
#include <CQChartsPaletteNameEdit.h>
#include <CQChartsMapKey.h>
#include <CQChartsWidgetUtil.h>
#include <CQCharts.h>

#include <CQIconButton.h>
#include <CQGroupBox.h>
#include <CQUtil.h>

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

CQChartsPlotCustomControls::
CQChartsPlotCustomControls(CQCharts *charts, const QString &plotType) :
 QFrame(nullptr), charts_(charts), plotType_(plotType)
{
  setObjectName("customControls");

  layout_ = CQUtil::makeLayout<QVBoxLayout>(this, 2, 2);

  //---

  titleFrame_ = CQUtil::makeWidget<QFrame>("titleFrame");
  auto *titleLayout = CQUtil::makeLayout<QHBoxLayout>(titleFrame_, 0, 2);

  layout_->addWidget(titleFrame_);

  //--

  titleLabel_ = CQUtil::makeLabelWidget<QLabel>("", "title");

  titleLayout->addWidget(titleLabel_);

  //---

  numericIcon_ = CQUtil::makeWidget<CQIconButton>("numericIcon");

  numericIcon_->setIcon("NUMERIC_COLUMNS");
  numericIcon_->setCheckable(true);
  numericIcon_->setChecked(isNumericOnly());
  numericIcon_->setToolTip("Restrict to columns with numeric values");

  numericIcon_->setVisible(false);

  titleLayout->addWidget(numericIcon_);
}

void
CQChartsPlotCustomControls::
addLayoutStretch()
{
  layout_->addStretch(1);
}

void
CQChartsPlotCustomControls::
addColumnWidgets(const QStringList &columnNames, FrameData &frameData)
{
  auto *plotType = this->plotType();
  assert(plotType);

  bool isNumeric = false;

  for (const auto &name : columnNames) {
    const auto *parameter = plotType->getParameter(name);
    assert(parameter);

    if (parameter->isNumeric())
      isNumeric = true;

    if      (parameter->type() == CQChartsPlotParameter::Type::COLUMN) {
      auto *columnEdit = new CQChartsColumnParameterEdit(parameter, /*isBasic*/true);

      addFrameWidget(frameData, parameter->desc(), columnEdit);

      columnEdits_.push_back(columnEdit);
    }
    else if (parameter->type() == CQChartsPlotParameter::Type::COLUMN_LIST) {
      auto *columnsEdit = new CQChartsColumnsParameterEdit(parameter, /*isBasic*/true);

      addFrameWidget(frameData, parameter->desc(), columnsEdit);

      columnsEdits_.push_back(columnsEdit);
    }
    else
      assert(false);
  }

  if (isNumeric)
    numericIcon_->setVisible(true);
}

void
CQChartsPlotCustomControls::
showColumnWidgets(const QStringList &columnNames)
{
  QWidget *w = nullptr;

  if      (! columnEdits_.empty())
    w = columnEdits_[0];
  else if (! columnsEdits_.empty())
    w = columnsEdits_[0];

  assert(w);

  auto *parent = w->parentWidget();

  auto *layout = qobject_cast<QGridLayout *>(parent->layout());
  assert(layout);

  for (int r = 0; r < layout->rowCount(); ++r) {
    auto *item1 = layout->itemAtPosition(r, 1);

    auto *edit = (item1 ? item1->widget() : nullptr);
    if (! edit) continue;

    auto *ce1 = qobject_cast<CQChartsColumnParameterEdit  *>(edit);
    auto *ce2 = qobject_cast<CQChartsColumnsParameterEdit *>(edit);
    if (! ce1 && ! ce2) continue;

    auto *item0 = layout->itemAtPosition(r, 0);
    auto *label = (item0 ? item0->widget() : nullptr);

    auto *parameter = (ce1 ? ce1->parameter() : ce2->parameter());

    bool visible = (columnNames.indexOf(parameter->name()) >= 0);

    if (label)
      label->setVisible(visible);

    edit->setVisible(visible);
  }
}

void
CQChartsPlotCustomControls::
addColorColumnWidgets(const QString &title)
{
  // color group
  auto colorControlGroupData = createColumnControlGroup("colorControlGroup", title);

  colorControlGroup_ = colorControlGroupData.group;

  //---

  // color widgets
  colorEdit_        = CQUtil::makeWidget<CQChartsColorLineEdit>("colorEdit");
  colorColumnCombo_ = CQUtil::makeWidget<CQChartsColumnCombo>("colorColumnCombo");
  colorRange_       = CQUtil::makeWidget<CQChartsColorRangeSlider>("colorRange");
  colorPaletteEdit_ = CQUtil::makeWidget<CQChartsPaletteNameEdit>("colorPaletteEdit");

  addFrameWidget(colorControlGroupData.fixedFrame , "Color"  , colorEdit_);
  addFrameWidget(colorControlGroupData.columnFrame, "Column" , colorColumnCombo_);
  addFrameWidget(colorControlGroupData.columnFrame, "Range"  , colorRange_);
  addFrameWidget(colorControlGroupData.columnFrame, "Palette", colorPaletteEdit_);

  addFrameRowStretch(colorControlGroupData.fixedFrame );
  addFrameRowStretch(colorControlGroupData.columnFrame);

  //---

  colorMapKeyWidget_ = new CQChartsMapKeyWidget;

  colorMapKeyWidget_->setFixedSize(16, 16);

  colorControlGroupData.columnControls->layout()->addWidget(colorMapKeyWidget_);

  connect(colorControlGroupData.group, SIGNAL(showKey(bool)),
          this, SLOT(showColorKeySlot(bool)));
}

void
CQChartsPlotCustomControls::
connectSlots(bool b)
{
  if (b == connected_)
    return;

  connected_ = b;

  if (plot_)
    CQChartsWidgetUtil::connectDisconnect(b,
      plot_, SIGNAL(colorDetailsChanged()), this, SLOT(colorDetailsSlot()));

  if (colorEdit_) {
    CQChartsWidgetUtil::connectDisconnect(b,
      colorControlGroup_, SIGNAL(groupChanged()), this, SLOT(colorGroupChanged()));
    CQChartsWidgetUtil::connectDisconnect(b,
      colorEdit_, SIGNAL(colorChanged()), this, SLOT(colorSlot()));
    CQChartsWidgetUtil::connectDisconnect(b,
      colorColumnCombo_, SIGNAL(columnChanged()), this, SLOT(colorColumnSlot()));
    CQChartsWidgetUtil::connectDisconnect(b,
      colorRange_, SIGNAL(sliderRangeChanged(double, double)), this, SLOT(colorRangeSlot()));
    CQChartsWidgetUtil::connectDisconnect(b,
      colorPaletteEdit_, SIGNAL(nameChanged()), this, SLOT(colorPaletteSlot()));
  }

  for (auto *columnEdit : columnEdits_)
    CQChartsWidgetUtil::connectDisconnect(b,
      columnEdit, SIGNAL(columnChanged()), this, SLOT(columnSlot()));

  for (auto *columnsEdit : columnsEdits_)
    CQChartsWidgetUtil::connectDisconnect(b,
      columnsEdit, SIGNAL(columnsChanged()), this, SLOT(columnsSlot()));

  if (numericIcon_)
    CQChartsWidgetUtil::connectDisconnect(b,
      numericIcon_, SIGNAL(clicked(bool)), this, SLOT(numericOnlySlot(bool)));
}

void
CQChartsPlotCustomControls::
setPlot(CQChartsPlot *plot)
{
  if (plot_) {
    disconnect(plot_, SIGNAL(plotDrawn()), this, SLOT(plotDrawnSlot()));
    disconnect(plot_, SIGNAL(colorDetailsChanged()), this, SLOT(colorDetailsSlot()));
  }

  plot_ = plot;

  if (plot_) {
    connect(plot_, SIGNAL(plotDrawn()), this, SLOT(plotDrawnSlot()));
    connect(plot_, SIGNAL(colorDetailsChanged()), this, SLOT(colorDetailsSlot()));
  }
}

void
CQChartsPlotCustomControls::
setNumericOnly(bool b)
{
  if (b != numericOnly_) {
    numericOnly_ = b;

    connectSlots(false);

    if (numericIcon_)
      numericIcon_->setChecked(isNumericOnly());

    updateNumericOnly();

    emit numericOnlyChanged();

    connectSlots(true);
  }
}

void
CQChartsPlotCustomControls::
setShowTitle(bool b)
{
  showTitle_ = b;

  titleFrame_->setVisible(showTitle_);

  // TODO: also hides numeric check
}

void
CQChartsPlotCustomControls::
colorGroupChanged()
{
  if (colorControlGroup_->isFixed())
    plot()->setColorColumn(CQChartsColumn());
}

void
CQChartsPlotCustomControls::
plotDrawnSlot()
{
  handlePlotDrawn();
}

void
CQChartsPlotCustomControls::
handlePlotDrawn()
{
  updateColorKeyVisible();
}

void
CQChartsPlotCustomControls::
updateColorKeyVisible()
{
  if (colorMapKeyWidget_) {
    auto *colorMapKey = plot()->colorMapKey();

    auto hasColorColumn = plot()->colorColumn().isValid();

    bool hasColorMapKey = (hasColorColumn && colorMapKey && ! colorMapKey->isNative());

    colorMapKeyWidget_->setVisible(hasColorMapKey && colorControlGroup_->isKeyVisible());

    colorMapKeyWidget_->setKey(colorMapKey);
  }
}

void
CQChartsPlotCustomControls::
colorDetailsSlot()
{
  // plot color details changed
  updateWidgets();
}

void
CQChartsPlotCustomControls::
columnSlot()
{
  auto *columnEdit = qobject_cast<CQChartsColumnParameterEdit *>(sender());
  auto *parameter  = columnEdit->parameter();

  plot()->setNamedColumn(parameter->name(), columnEdit->getColumn());
}

void
CQChartsPlotCustomControls::
columnsSlot()
{
  auto *columnsEdit = qobject_cast<CQChartsColumnsParameterEdit *>(sender());
  auto *parameter   = columnsEdit->parameter();

  plot()->setNamedColumns(parameter->name(), columnsEdit->columns());
}

void
CQChartsPlotCustomControls::
numericOnlySlot(bool state)
{
  setNumericOnly(state);
}

CQChartsPlotType *
CQChartsPlotCustomControls::
plotType() const
{
  return charts_->plotType(plotType_);
}

CQChartsPlotCustomControls::ColumnControlGroupData
CQChartsPlotCustomControls::
createColumnControlGroup(const QString &name, const QString &title)
{
  ColumnControlGroupData columnControlGroupData;

  // control group
  columnControlGroupData.group = CQUtil::makeWidget<CQChartsColumnControlGroup>(name);

  columnControlGroupData.group->setTitle(title);

  layout_->addWidget(columnControlGroupData.group);

  columnControlGroupData.fixedFrame     = createFrame("fixedFrame");
  columnControlGroupData.columnFrame    = createFrame("columnFrame");
  columnControlGroupData.columnControls = CQUtil::makeWidget<QFrame>("columnControls");

  auto *columnControlsLayout =
    CQUtil::makeLayout<QHBoxLayout>(columnControlGroupData.columnControls, 0, 0);

  columnControlsLayout->addWidget(columnControlGroupData.columnFrame.frame);

  columnControlGroupData.group->addFixedWidget (columnControlGroupData.fixedFrame.frame);
  columnControlGroupData.group->addColumnWidget(columnControlGroupData.columnControls  );

  return columnControlGroupData;
}

CQChartsPlotCustomControls::FrameData
CQChartsPlotCustomControls::
createGroupFrame(const QString &name, const QString &objName, bool stretch)
{
  auto frameData = createFrame(objName, stretch);

  frameData.groupBox = CQUtil::makeLabelWidget<CQGroupBox>(name, "groupBox");

  auto *groupLayout = CQUtil::makeLayout<QVBoxLayout>(frameData.groupBox, 0, 0);

  frameData.groupBox->setTitleScale(0.85);
  frameData.groupBox->setTitleColored(true);

  groupLayout->addWidget(frameData.frame);

  layout_->addWidget(frameData.groupBox);

  return frameData;
}

CQChartsPlotCustomControls::FrameData
CQChartsPlotCustomControls::
createFrame(const QString &objName, bool stretch)
{
  FrameData frameData;

  frameData.frame  = CQUtil::makeWidget<QFrame>(objName);
  frameData.layout = CQUtil::makeLayout<QGridLayout>(frameData.frame, 2, 2);

  if (stretch)
    frameData.layout->setColumnStretch(1, 1);

  return frameData;
}

void
CQChartsPlotCustomControls::
addFrameWidget(FrameData &frameData, const QString &label, QWidget *w, bool nextRow)
{
  frameData.layout->addWidget(new QLabel(label), frameData.row, frameData.col++);
  frameData.layout->addWidget(w                , frameData.row, frameData.col++);

  if (nextRow) {
    frameData.col = 0;

    ++frameData.row;
  }
}

void
CQChartsPlotCustomControls::
addFrameWidget(FrameData &frameData, QWidget *w, bool nextRow)
{
  frameData.layout->addWidget(w, frameData.row, frameData.col, 1, 2);

  frameData.col += 2;

  if (nextRow) {
    frameData.col = 0;

    ++frameData.row;
  }
}

void
CQChartsPlotCustomControls::
addFrameSpacer(FrameData &frameData, bool nextRow)
{
  auto *spacer = CQUtil::makeLabelWidget<QLabel>(" ", "spacer");

  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  frameData.layout->addWidget(spacer, frameData.row, frameData.col++);

  if (nextRow) {
    frameData.col = 0;

    ++frameData.row;
  }
}

void
CQChartsPlotCustomControls::
addFrameColWidget(FrameData &frameData, QWidget *w, bool nextRow)
{
  frameData.layout->addWidget(w, frameData.row, frameData.col++);

  if (nextRow) {
    addFrameRowStretch(frameData);

    frameData.col = 0;

    ++frameData.row;
  }
}

void
CQChartsPlotCustomControls::
addFrameRowStretch(FrameData &frameData)
{
  frameData.layout->setRowStretch(frameData.row, frameData.col);
}

void
CQChartsPlotCustomControls::
setFrameWidgetVisible(QWidget *w, bool visible)
{
  auto *pw = w->parentWidget();

  auto *layout = qobject_cast<QGridLayout *>(pw->layout());
  if (! layout) return;

  // find widget row
  int row = -1;

  for (int i = 0; i < layout->count(); ++i) {
    auto *item = layout->itemAt(i);

    if (item->widget() == w) {
      int c, rs, cs;

      layout->getItemPosition(i, &row, &c, &rs, &cs);

      break;
    }
  }

  // set visible for all items on same row
  for (int i = 0; i < layout->count(); ++i) {
    auto *item = layout->itemAt(i);
    if (! item->widget()) continue;

    int r, c, rs, cs;

    layout->getItemPosition(i, &r, &c, &rs, &cs);

    if (r == row)
      item->widget()->setVisible(visible);
  }
}

void
CQChartsPlotCustomControls::
showColorKeySlot(bool)
{
  updateColorKeyVisible();
}

CQChartsBoolParameterEdit *
CQChartsPlotCustomControls::
createBoolEdit(const QString &name, bool choice)
{
  auto *plotType = this->plotType();
  assert(plotType);

  const auto *parameter = plotType->getParameter(name);
  assert(parameter && parameter->type() == CQChartsPlotParameter::Type::BOOLEAN);

  const auto *bparameter = dynamic_cast<const CQChartsBoolParameter *>(parameter);
  assert(bparameter);

  return new CQChartsBoolParameterEdit(bparameter, choice);
}

CQChartsEnumParameterEdit *
CQChartsPlotCustomControls::
createEnumEdit(const QString &name)
{
  auto *plotType = this->plotType();
  assert(plotType);

  const auto *parameter = plotType->getParameter(name);
  assert(parameter && parameter->type() == CQChartsPlotParameter::Type::ENUM);

  const auto *eparameter = dynamic_cast<const CQChartsEnumParameter *>(parameter);
  assert(eparameter);

  return new CQChartsEnumParameterEdit(eparameter);
}

void
CQChartsPlotCustomControls::
updateWidgets()
{
  connectSlots(false);

  //---

  auto titleDesc = plot()->type()->desc();
  auto titleStr  = plot()->titleStr();

  if (titleStr == "")
    titleStr = plot()->calcName();

  titleLabel_->setText(QString("<b>%1: %2</b>").arg(titleDesc).arg(titleStr));

  //----

  if (colorEdit_) {
    auto hasColorColumn = plot()->colorColumn().isValid();
    auto isNative       = (plot()->colorMapType() == CQChartsPlot::ColumnType::COLOR);

    colorEdit_       ->setEnabled(! hasColorColumn);
    colorRange_      ->setEnabled(hasColorColumn && ! isNative);
    colorPaletteEdit_->setEnabled(hasColorColumn && ! isNative);

    colorEdit_->setColor(getColorValue());

    colorColumnCombo_->setModelColumn(plot()->getModelData(), plot()->colorColumn());

    auto paletteName = plot()->colorMapPalette();

    colorRange_->setPlot(plot());
    colorRange_->setPaletteName(paletteName);

    colorPaletteEdit_->setChartsPaletteName(plot()->charts(), paletteName);

    if (hasColorColumn)
      colorControlGroup_->setColumn();
  }

  //---

  for (auto *columnEdit : columnEdits_) {
    auto *parameter = columnEdit->parameter();

    columnEdit->setModelData(plot()->getModelData());
    columnEdit->setColumn   (plot()->getNamedColumn(parameter->name()));

    if (parameter->isNumeric())
      columnEdit->setNumericOnly(isNumericOnly());
  }

  for (auto *columnsEdit : columnsEdits_) {
    auto *parameter = columnsEdit->parameter();

    columnsEdit->setModelData(plot()->getModelData());
    columnsEdit->setColumns  (plot()->getNamedColumns(parameter->name()));

    if (parameter->isNumeric())
      columnsEdit->setNumericOnly(isNumericOnly());
  }

  //---

  updateNumericOnly();

  //---

  connectSlots(true);
}

void
CQChartsPlotCustomControls::
updateNumericOnly()
{
  for (auto *columnEdit : columnEdits_) {
    auto *parameter = columnEdit->parameter();

    if (parameter->isNumeric())
      columnEdit->setNumericOnly(isNumericOnly());
  }

  for (auto *columnsEdit : columnsEdits_) {
    auto *parameter = columnsEdit->parameter();

    if (parameter->isNumeric())
      columnsEdit->setNumericOnly(isNumericOnly());
  }
}

void
CQChartsPlotCustomControls::
colorSlot()
{
  setColorValue(colorEdit_->color());
}

void
CQChartsPlotCustomControls::
colorColumnSlot()
{
  plot()->setColorColumn(colorColumnCombo_->getColumn());
}

void
CQChartsPlotCustomControls::
colorRangeSlot()
{
  connectSlots(false);

  plot()->setColorMapMin(colorRange_->sliderMin());
  plot()->setColorMapMax(colorRange_->sliderMax());

  updateWidgets();

  connectSlots(true);
}

void
CQChartsPlotCustomControls::
colorPaletteSlot()
{
  plot()->setColorMapPalette(colorPaletteEdit_->paletteName());
}
