#include <CQChartsBoxPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsTip.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsAnalyzeModelData.h>
#include <CQChartsUtil.h>
#include <CQChartsVariant.h>
#include <CQChartsRand.h>
#include <CQCharts.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsPaintDevice.h>
#include <CQChartsHtml.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

#include <QMenu>

CQChartsBoxPlotType::
CQChartsBoxPlotType()
{
}

void
CQChartsBoxPlotType::
addParameters()
{
  startParameterGroup("Box Plot");

  //---

  auto primaryGroup = startParameterGroup("Raw Values");

  addColumnsParameter("value", "Value", "valueColumns").
    setNumeric().setRequired().setTip("Value column(s)");
  addColumnParameter ("name", "Name", "nameColumn").
    setString().setTip("Name column");
  addColumnParameter ("set", "Set", "setColumn").
    setTip("Set Values");

  endParameterGroup();

  //---

  auto secondaryGroup = startParameterGroup("Calculated Values");

  addColumnParameter("x"          , "X"           , "xColumn"          ).
    setNumeric().setTip("X Value");
  addColumnParameter("min"        , "Min"         , "minColumn"        ).
    setNumeric().setTip("Min Value");
  addColumnParameter("lowerMedian", "Lower Median", "lowerMedianColumn").
    setNumeric().setTip("Lower Median Value");
  addColumnParameter("median"     , "Median"      , "medianColumn"     ).
    setNumeric().setTip("Median Value");
  addColumnParameter("upperMedian", "Upper Median", "upperMedianColumn").
    setNumeric().setTip("Upper Median Value");
  addColumnParameter("max"        , "Max"         , "maxColumn"        ).
    setNumeric().setTip("Max Value");
  addColumnParameter("outliers"   , "Outliers"    , "outliersColumn"   ).
    setTip("Outlier Values");

  endParameterGroup();

  //---

  primaryGroup  ->setType(CQChartsPlotParameterGroup::Type::PRIMARY  );
  secondaryGroup->setType(CQChartsPlotParameterGroup::Type::SECONDARY);

  primaryGroup  ->setOtherGroupId(secondaryGroup->groupId());
  secondaryGroup->setOtherGroupId(primaryGroup  ->groupId());

  //---

  addBoolParameter("normalized", "Normalized", "normalized").
    setBasic().setTip("Normalize data ranges");

  addBoolParameter("horizontal", "Horizontal", "horizontal").setTip("Draw bars horizontal");
  addBoolParameter("notched"   , "Notched"   , "notched"   ).setTip("Draw notch on bar");

  addBoolParameter("colorBySet", "Color by Set", "colorBySet").setTip("Color by value set");

  addEnumParameter("pointsType", "Points Type", "pointsType").
   addNameValue("NONE"   , int(CQChartsBoxPlot::PointsType::NONE   )).
   addNameValue("JITTER" , int(CQChartsBoxPlot::PointsType::JITTER )).
   addNameValue("STACKED", int(CQChartsBoxPlot::PointsType::STACKED)).
   setTip("Show data points type");

  addBoolParameter("violin"  , "Violin"   , "violin"  ).setTip("Draw distribution outline");
  addBoolParameter("errorBar", "Error Bar", "errorBar").setTip("Error bar");

  //---

  endParameterGroup();

  //---

  CQChartsGroupPlotType::addParameters();
}

QString
CQChartsBoxPlotType::
description() const
{
  auto B   = [](const QString &str) { return CQChartsHtml::Str::bold(str); };
  auto LI  = [](const QString &str) { return CQChartsHtml::Str(str); };
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Box Plot").
    h3("Summary").
     p("Draws box and whiskers for the min, max, median and outlier values of the set "
       "of y values for rows with identical x values.").
    h3("Columns").
     p("Values can be supplied using:").
     ul({ LI("Raw Values with X and Y values in " + B("value") + " and " + B("set") + " columns."),
          LI("Calculated Values in the " + B("x") + ", " + B("min") + ", " +
             B("lowerMedian") + ", " + B("median") + ", " + B("upperMedian") + ", " +
             B("max") + " and " + B("outliers") + " columns.") }).
     p("The x value name can be supplied using this " + B("name") + " column.").
    h3("Options").
     p("The outliers values can be shown or hidden..").
     p("Multiple boxes (for each unique x value) can be connected by their y value.").
     p("The box can be drawn vertically or horizontally.").
     p("The y values can be normals to the range 0-1 so whiskers with different y ranges "
       "can be compared.").
     p("The box can be notched to show the confidence interval around the median.").
     p("The individual points can be displayed (if supplied) using jiier or stacked points.").
     p("The box can be drawn with a violin shape using the distribution curve.").
     p("The box can be drawn as an error bar.").
    h3("Customization").
     p("The box, outlier and data points can be styled (fill and stroke)").
    h3("Limitations").
     p("The plot does not support logarithmic x values.").
    h3("Example").
     p(IMG("images/boxplot.png"));
}

void
CQChartsBoxPlotType::
analyzeModel(CQChartsModelData *modelData, CQChartsAnalyzeModelData &analyzeModelData)
{
  auto *details = modelData->details();
  if (! details) return;

  CQChartsColumns columns;

  int nc = details->numColumns();

  for (int i = 0; i < nc; ++i) {
    auto columnDetails = details->columnDetails(CQChartsColumn(i));

    if (columnDetails && columnDetails->isNumeric())
      columns.addColumn(columnDetails->column());
  }

  analyzeModelData.parameterNameColumns["value"] = columns;

  if (columns.count() > 1)
    analyzeModelData.parameterNameBool["normalized"] = true;
}

CQChartsPlot *
CQChartsBoxPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsBoxPlot(view, model);
}

//---

CQChartsBoxPlot::
CQChartsBoxPlot(CQChartsView *view, const ModelP &model) :
 CQChartsGroupPlot(view, view->charts()->plotType("box"), model),
 CQChartsObjBoxShapeData    <CQChartsBoxPlot>(this),
 CQChartsObjTextData        <CQChartsBoxPlot>(this),
 CQChartsObjWhiskerLineData <CQChartsBoxPlot>(this),
 CQChartsObjOutlierPointData<CQChartsBoxPlot>(this),
 CQChartsObjJitterPointData <CQChartsBoxPlot>(this)
{
  NoUpdate noUpdate(this);

  setBoxFillColor(CQChartsColor(CQChartsColor::Type::PALETTE));

  setBoxStroked(true);
  setBoxFilled (true);

  setOutlierSymbolType(CQChartsSymbol::Type::CIRCLE);
  setOutlierSymbolSize(CQChartsLength("4px"));
  setOutlierSymbolFilled(true);
  setOutlierSymbolFillColor(CQChartsColor(CQChartsColor::Type::PALETTE));

  setJitterSymbolType(CQChartsSymbol::Type::CIRCLE);
  setJitterSymbolSize(CQChartsLength("4px"));
  setJitterSymbolFilled(true);
  setJitterSymbolFillColor(CQChartsColor(CQChartsColor::Type::PALETTE));

  addAxes();

  addKey();

  addTitle();
}

CQChartsBoxPlot::
~CQChartsBoxPlot()
{
  clearRawWhiskers();
}

//------

void
CQChartsBoxPlot::
setValueColumns(const CQChartsColumns &c)
{
  CQChartsUtil::testAndSet(valueColumns_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setSetColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(setColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setNameColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(nameColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
setXColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(xColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setMinColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(minColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setLowerMedianColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(lowerMedianColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setMedianColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(medianColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setUpperMedianColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(upperMedianColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setMaxColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(maxColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setOutliersColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(outliersColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
setShowOutliers(bool b)
{
  CQChartsUtil::testAndSet(showOutliers_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setConnected(bool b)
{
  CQChartsUtil::testAndSet(connected_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setBoxWidth(const CQChartsLength &l)
{
  CQChartsUtil::testAndSet(boxWidth_, l, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
setColorBySet(bool b)
{
  CQChartsUtil::testAndSet(colorBySet_, b, [&]() { resetSetHidden(); updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
setWhiskerRange(double r)
{
  CQChartsUtil::testAndSet(whiskerRange_, r, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setWhiskerExtent(double r)
{
  CQChartsUtil::testAndSet(whiskerExtent_, r, [&]() { drawObjs(); } );
}

//------

void
CQChartsBoxPlot::
setTextMargin(double r)
{
  CQChartsUtil::testAndSet(textMargin_, r, [&]() { drawObjs(); } );
}

//------

void
CQChartsBoxPlot::
setYMargin(double r)
{
  CQChartsUtil::testAndSet(ymargin_, r, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  addBaseProperties();

  // columns
  addProp("columns/raw", "valueColumns", "values", "Value columns");
  addProp("columns/raw", "nameColumn"  , "name"  , "Name column");
  addProp("columns/raw", "setColumn"   , "set"   , "Set column");

  addProp("columns/calculated", "xColumn"          , "x"          ,
          "Precalculated x column");
  addProp("columns/calculated", "minColumn"        , "min"        ,
          "Precalculated min column");
  addProp("columns/calculated", "lowerMedianColumn", "lowerMedian",
          "Precalculated lower median column");
  addProp("columns/calculated", "medianColumn"     , "median"     ,
          "Precalculated median column");
  addProp("columns/calculated", "upperMedianColumn", "upperMedian",
          "Precalculated upperx column");
  addProp("columns/calculated", "maxColumn"        , "max"        ,
          "Precalculated max column");
  addProp("columns/calculated", "outliersColumn"   , "outlier"    ,
          "Precalculated outliers column");

  addGroupingProperties();

  // options
  addProp("options", "connected" , "connected" , "Connect across multiple whiskers");
  addProp("options", "horizontal", "horizontal", "Draw bar horizontal");
  addProp("options", "normalized", "normalized", "Normalize bar ranges to 0-1");

  // margins
  addProp("margins", "ymargin", "ybar", "Margin above/below bar when normalized")->
    setMinValue(0.0).setMaxValue(0.5);

  // jitter
  addProp("points", "pointsType", "type", "Draw jitter or scatter points");

  addSymbolProperties("points/symbol", "jitter", "Points");

  // violin
  addProp("violin", "violin"     , "visible", "Display distribution for box as violin");
  addProp("violin", "violinWidth", "width"  , "Width of violin");
  addProp("violin", "violinBox"  , "box"    , "Draw box as well as violin");

  // error bar
  addProp("errorBar", "errorBar"    , "visible", "Draw error bars");
  addProp("errorBar", "errorBarType", "type"   , "Error bar type");

  // whisker box
  addProp("box", "whiskerRange", "range"  , "Whisker interquartile range factor")->
    setMinValue(1.0);
  addProp("box", "boxWidth"    , "width"  , "Box width");
  addProp("box", "notched"     , "notched", "Box notched at median");

  // whisker box fill
  addProp("box/fill", "boxFilled", "visible", "Box fill visible");

  addFillProperties("box/fill", "boxFill", "Box");

  // whisker box stroke
  addProp("box/stroke", "boxStroked"   , "visible"   , "Box stroke visible");
  addProp("box/stroke", "boxCornerSize", "cornerSize", "Box corner size");

  addLineProperties("box/stroke", "boxStroke", "Box");

  // whisker line
  addLineProperties("whisker/stroke", "whiskerLines", "Whisker");

  addProp("whisker", "whiskerExtent", "extent", "Box whisker line extent")->
    setMinValue(0.0).setMaxValue(1.0);

  // value labels
  addProp("labels", "textVisible", "visible", "Value labels visible");

  addTextProperties("labels/text", "text", "Value", CQChartsTextOptions::ValueType::CONTRAST);

  addProp("labels", "textMargin", "margin", "Value text margin in pixels")->setMinValue(0.0);

  // outlier
  addProp("outlier", "showOutliers", "visible", "Outlier points visible");

  addSymbolProperties("outlier/symbol", "outlier", "Outlier");

  // coloring
  addProp("coloring", "colorBySet", "", "Color by value set");
}

//---

void
CQChartsBoxPlot::
setHorizontal(bool b)
{
  CQChartsUtil::testAndSet(horizontal_, b, [&]() {
    CQChartsAxis::swap(xAxis(), yAxis());

    updateRangeAndObjs();
  } );
}

void
CQChartsBoxPlot::
setNormalized(bool b)
{
  CQChartsUtil::testAndSet(normalized_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setNotched(bool b)
{
  CQChartsUtil::testAndSet(notched_, b, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
setPointsJitter(bool b)
{
  setPointsType(b ? PointsType::JITTER : PointsType::NONE);
}

void
CQChartsBoxPlot::
setPointsStacked(bool b)
{
  setPointsType(b ? PointsType::STACKED : PointsType::NONE);
}

void
CQChartsBoxPlot::
setPointsType(const PointsType &pointsType)
{
  if (pointsType != pointsType_) {
    pointsType_ = pointsType;

    updateRangeAndObjs();
  }
}

//---

void
CQChartsBoxPlot::
setViolin(bool b)
{
  CQChartsUtil::testAndSet(violin_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setViolinWidth(const CQChartsLength &l)
{
  CQChartsUtil::testAndSet(violinWidth_, l, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setViolinBox(bool b)
{
  CQChartsUtil::testAndSet(violinBox_, b, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBoxPlot::
setErrorBar(bool b)
{
  CQChartsUtil::testAndSet(errorBar_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsBoxPlot::
setErrorBarType(const ErrorBarType &t)
{
  CQChartsUtil::testAndSet(errorBarType_, t, [&]() { updateRangeAndObjs(); } );
}

//---

bool
CQChartsBoxPlot::
isPreCalc() const
{
  return (minColumn        ().isValid() &&
          lowerMedianColumn().isValid() &&
          medianColumn     ().isValid() &&
          upperMedianColumn().isValid() &&
          maxColumn        ().isValid());
}

//---

CQChartsGeom::Range
CQChartsBoxPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsBoxPlot::calcRange");

  NoUpdate noUpdate(this);

  CQChartsGeom::Range dataRange;

  //---

  // x-axis must be integer, y-axis must be real
  auto *xAxis = mappedXAxis();
  auto *yAxis = mappedYAxis();

  xAxis->setValueType     (CQChartsAxisValueType::Type::INTEGER, /*notify*/false);
  xAxis->setMajorIncrement(1);

  yAxis->setValueType     (CQChartsAxisValueType::Type::REAL, /*notify*/false);
  yAxis->setMajorIncrement(0);

  //---

  if (! isPreCalc())
    dataRange = updateRawRange();
  else
    dataRange = updateCalcRange();

  return dataRange;
}

// calculate box plot from individual values
CQChartsGeom::Range
CQChartsBoxPlot::
updateRawRange() const
{
  auto *th = const_cast<CQChartsBoxPlot *>(this);

  CQChartsGeom::Range dataRange;

  //---

  // check columns
  bool columnsValid = true;

  th->clearErrors();

  // value columns required
  // name, group and set column optional

  if (! checkColumns(valueColumns(), "Values", /*required*/true))
    columnsValid = false;

  if (! checkColumn(nameColumn (), "Name" )) columnsValid = false;
  if (! checkColumn(groupColumn(), "Group")) columnsValid = false;

  if (! checkColumn(setColumn(), "Set", th->setType_))
    columnsValid = false;

  if (! columnsValid)
    return dataRange;

  //---

  auto *xAxis = mappedXAxis();
  auto *yAxis = mappedYAxis();

  th->forceNoYAxis_ = false;

  yAxis->setVisible(true);

  //---

  initGroupData(valueColumns(), nameColumn());

  //---

  updateRawWhiskers();

  //---

  RMinMax xrange;

  auto updateRange = [&](double x, double y) {
    if (! isHorizontal())
      dataRange.updateRange(x, y);
    else
      dataRange.updateRange(y, x);
  };

  //---

  xAxis->clearTickLabels();

  //---

  bool hasSets   = this->hasSets();
  bool hasGroups = this->hasGroups();

  //---

  int ig = 0;

  for (auto &groupIdWhiskers : groupWhiskers_) {
    int                  groupInd      = groupIdWhiskers.first;
    const SetWhiskerMap &setWhiskerMap = groupIdWhiskers.second;

    if (! isWhiskersGrouped() || ! isSetHidden(ig)) {
      int is = 0;

      for (auto &setWhiskers : setWhiskerMap) {
        if (isWhiskersGrouped() || ! isSetHidden(is)) {
          int  setId   = setWhiskers.first;
          auto whisker = setWhiskers.second;

          //---

          QString name;
          bool    allowNoName = false;

          if      (isGroupHeaders()) {
            name = whisker->name();
          }
          else if (hasGroups) {
            name = groupIndName(groupInd);
          }
          else if (hasSets) {
            name = this->setIdName(setId);
          }
          else {
            allowNoName = true;
          }

          whisker->setName(name);

          //---

          int x;

          if      (hasSets && isConnected())
            x = setId;
          else if (hasSets && ! hasGroups)
            x = setId;
          else
            x = ig;

          //---

          if (allowNoName || name.length())
            xAxis->setTickLabel(x, name);

          //---

          double min, max;

          if (isErrorBar()) {
            min = whisker->mean() - whisker->stddev();
            max = whisker->mean() + whisker->stddev();
          }
          else {
            if (isShowOutliers()) {
              min = whisker->vmin();
              max = whisker->vmax();
            }
            else {
              min = whisker->min();
              max = whisker->max();
            }
          }

          if (! isNormalized()) {
            updateRange(x - 0.5, min);
            updateRange(x + 0.5, max);
          }
          else {
            xrange.add(x - 0.5);
            xrange.add(x + 0.5);
          }

          //---

          if (isViolin()) {
            const CQChartsDensity &density = whisker->density();

            updateRange(x, density.xmin1());
            updateRange(x, density.xmax1());
          }
        }

        ++is;
      }
    }

    ++ig;
  }

  //---

  if (isNormalized()) {
    if (xrange.isSet()) {
      updateRange(xrange.min(), 0.0);
      updateRange(xrange.max(), 1.0);
    }
    else {
      updateRange(0.0, 0.0);
      updateRange(1.0, 1.0);
    }
  }

  //---

//xAxis->setColumn(setColumn());
  yAxis->setColumn(valueColumns().column());

  //---

  xAxis->setLabel(CQChartsOptString(groupSetColumnName("")));
  yAxis->setLabel(CQChartsOptString(valueColumnName   ("")));

  //---

  return dataRange;
}

QString
CQChartsBoxPlot::
groupSetColumnName(const QString &def) const
{
  if (numGroupWhiskers() <= 1)
    return "";

  bool ok;

  QString xname = xLabel();

  if (! xname.length() && groupColumn().isValid())
    xname = modelHHeaderString(groupColumn(), ok);

  if (! xname.length() && setColumn().isValid())
    xname = modelHHeaderString(setColumn(), ok);

  if (! xname.length())
    xname = def;

  return xname;
}

QString
CQChartsBoxPlot::
valueColumnName(const QString &def) const
{
  bool ok;

  QString yname = yLabel();

  if (valueColumns().count() == 1 && ! yname.length())
    yname = modelHHeaderString(valueColumns().column(), ok);

  if (! yname.length())
    yname = def;

  return yname;
}

QString
CQChartsBoxPlot::
groupColumnName(const QString &def) const
{
  bool ok;

  QString groupName;

  if (groupColumn().isValid())
    groupName = modelHHeaderString(groupColumn(), ok);

  if (! groupName.length())
    groupName = def;

  return groupName;
}

//---

bool
CQChartsBoxPlot::
hasSets() const
{
  for (const auto &groupIdWhiskers : groupWhiskers_) {
    const SetWhiskerMap &setWhiskerMap = groupIdWhiskers.second;

    if (setWhiskerMap.size() > 1)
      return true;
  }

  return false;
}

bool
CQChartsBoxPlot::
hasGroups() const
{
  return (groupWhiskers().size() > 1);
}

QString
CQChartsBoxPlot::
setIdName(int setId) const
{
  return setValueInd_.idName(setId);
}

// calculate box plot from pre-calculated values
CQChartsGeom::Range
CQChartsBoxPlot::
updateCalcRange() const
{
  auto *th = const_cast<CQChartsBoxPlot *>(this);

  CQChartsGeom::Range dataRange;

  //---

  // check columns
  bool columnsValid = true;

  th->clearErrors();

  // min, lowerMedian, median, upperMedia, max required (already checked)
  // x, outliers optional (value list)

  if (! checkColumn(xColumn(), "X", th->xType_))
    columnsValid = false;

  if (! checkColumn(minColumn        (), "Min"         )) columnsValid = false;
  if (! checkColumn(lowerMedianColumn(), "Lower Median")) columnsValid = false;
  if (! checkColumn(medianColumn     (), "Median"      )) columnsValid = false;
  if (! checkColumn(upperMedianColumn(), "Upper Median")) columnsValid = false;
  if (! checkColumn(maxColumn        (), "Max"         )) columnsValid = false;
  if (! checkColumn(outliersColumn   (), "Outliers"    )) columnsValid = false;
  if (! checkColumn(idColumn         (), "Id"          )) columnsValid = false;

  if (! columnsValid)
    return dataRange;

  //---

  th->xValueInd_.clear();

  //---

  auto *xAxis = mappedXAxis();
  auto *yAxis = mappedYAxis();

  th->forceNoYAxis_ = true;

  yAxis->setVisible(false);

  //---

  RMinMax xrange;

  auto updateRange = [&](double x, double y) {
    if (! isHorizontal())
      dataRange.updateRange(x, y);
    else
      dataRange.updateRange(y, x);
  };

  //---

  // process model data
  class BoxPlotVisitor : public ModelVisitor {
   public:
    using DataList = CQChartsBoxPlot::WhiskerDataList;

   public:
    BoxPlotVisitor(const CQChartsBoxPlot *plot, CQChartsGeom::Range &dataRange,
                   CQChartsGeom::RMinMax &xrange) :
     plot_(plot), dataRange_(dataRange), xrange_(xrange) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      plot_->addCalcRow(data, dataList_, dataRange_, xrange_);

      return State::OK;
    }

    const DataList &dataList() const { return dataList_; }

   private:
    const CQChartsBoxPlot* plot_ { nullptr };
    CQChartsGeom::Range&   dataRange_;
    CQChartsGeom::RMinMax& xrange_;
    DataList               dataList_;
  };

  BoxPlotVisitor boxPlotVisitor(this, dataRange, xrange);

  visitModel(boxPlotVisitor);

  th->whiskerDataList_ = boxPlotVisitor.dataList();

  //---

  if (isNormalized()) {
    if (xrange.isSet()) {
      updateRange(xrange.min(), 0.0);
      updateRange(xrange.max(), 1.0);
    }
    else {
      updateRange(0.0, 0.0);
      updateRange(1.0, 1.0);
    }
  }

  //---

  bool ok;

  QString xname = (xLabel().length() ? xLabel() : modelHHeaderString(xColumn(), ok));

  xAxis->setLabel(CQChartsOptString(xname));

  //---

  return dataRange;
}

void
CQChartsBoxPlot::
addCalcRow(const ModelVisitor::VisitData &vdata, WhiskerDataList &dataList,
           CQChartsGeom::Range &dataRange, CQChartsGeom::RMinMax &xrange) const
{
  auto updateRange = [&](double x, double y) {
    if (! isHorizontal())
      dataRange.updateRange(x, y);
    else
      dataRange.updateRange(y, x);
  };

  auto *th = const_cast<CQChartsBoxPlot *>(this);

  //---

  CQChartsBoxWhiskerData data;

  data.ind = vdata.parent;

  bool ok;

  //---

  if (xColumn().isValid()) {
    CQChartsModelIndex xInd(vdata.row, xColumn(), vdata.parent);

    // x column can be string or real
    if (xType_ == ColumnType::STRING) {
      QString xname = modelString(xInd, ok);

      if (! ok) {
        th->addDataError(xInd, "Invalid x value");
        return;
      }

      data.name = xname;
      data.x    = th->xValueInd_.calcId(data.name);
    }
    else {
      data.x = modelReal(xInd, ok);

      if (! ok) {
        th->addDataError(xInd, "Invalid x value");
        return;
      }
    }
  }
  else {
    data.x = vdata.row;
  }

  //---

  CQChartsModelIndex minInd        (vdata.row, minColumn        (), vdata.parent);
  CQChartsModelIndex lowerMedianInd(vdata.row, lowerMedianColumn(), vdata.parent);
  CQChartsModelIndex medianInd     (vdata.row, medianColumn     (), vdata.parent);
  CQChartsModelIndex upperMedianInd(vdata.row, upperMedianColumn(), vdata.parent);
  CQChartsModelIndex maxInd        (vdata.row, maxColumn        (), vdata.parent);

  data.statData.min         = modelReal(minInd        , ok);
  if (! ok) { th->addDataError(minInd        , "Invalid min value"); return; }
  data.statData.lowerMedian = modelReal(lowerMedianInd, ok);
  if (! ok) { th->addDataError(lowerMedianInd, "Invalid lower median value"); return; }
  data.statData.median      = modelReal(medianInd     , ok);
  if (! ok) { th->addDataError(medianInd     , "Invalid median value"); return; }
  data.statData.upperMedian = modelReal(upperMedianInd, ok);
  if (! ok) { th->addDataError(upperMedianInd, "Invalid upper median value"); return; }
  data.statData.max         = modelReal(maxInd        , ok);
  if (! ok) { th->addDataError(maxInd        , "Invalid max value"); return; }

  data.dataMin = data.statData.min;
  data.dataMax = data.statData.max;

  if (isShowOutliers() && outliersColumn().isValid()) {
    CQChartsModelIndex outliersInd(vdata.row, outliersColumn(), vdata.parent);

    data.outliers = modelReals(outliersInd, ok);

    if (! ok) {
      th->addDataError(outliersInd, "Invalid outlier real values");
      return;
    }

    for (auto &o : data.outliers) {
      data.dataMin = std::min(data.dataMin, o);
      data.dataMax = std::max(data.dataMax, o);
    }
  }

  if (! isNormalized()) {
    updateRange(data.x - 0.5, data.statData.min);
    updateRange(data.x + 0.5, data.statData.max);
  }
  else {
    xrange.add(data.x - 0.5);
    xrange.add(data.x + 0.5);
  }

  //---

  bool nameValid = true;

  if (! data.name.length()) {
    if (idColumn().isValid()) {
      CQChartsModelIndex idInd(vdata.row, idColumn(), vdata.parent);

      QString id = modelString(idInd, ok);

      if (ok)
        data.name = id;
      else
        th->addDataError(idInd, "Invalid id value");
    }
  }

  if (! data.name.length()) {
    data.name = modelVHeaderString(vdata.row, Qt::Vertical, ok); // ignore fail

    if (! data.name.length()) {
      data.name = QString("%1").arg(vdata.row);
      nameValid = false;
    }
  }

  if (nameValid)
    xAxis_->setTickLabel(int(data.x), data.name);

  //---

  dataList.push_back(data);
}

void
CQChartsBoxPlot::
updateRawWhiskers() const
{
  auto *th = const_cast<CQChartsBoxPlot *>(this);

  th->clearRawWhiskers();

  //---

  // set data type
  th->setValueInd_.clear();

  //---

  // process model data
  class BoxPlotVisitor : public ModelVisitor {
   public:
    BoxPlotVisitor(const CQChartsBoxPlot *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      plot_->addRawWhiskerRow(data);

      return State::OK;
    }

   private:
    const CQChartsBoxPlot *plot_ { nullptr };
  };

  BoxPlotVisitor boxPlotVisitor(this);

  visitModel(boxPlotVisitor);

  //---

  th->isWhiskersGrouped_ = (numGroupWhiskers() > 1);

  //---

  for (auto &groupIdWhiskers : groupWhiskers_) {
    const SetWhiskerMap &setWhiskerMap = groupIdWhiskers.second;

    for (auto &setWhiskers : setWhiskerMap)
      setWhiskers.second->init();
  }
}

void
CQChartsBoxPlot::
clearRawWhiskers()
{
  for (auto &groupWhisker : groupWhiskers_) {
    const SetWhiskerMap &setWhiskerMap = groupWhisker.second;

    for (auto &setWhisker : setWhiskerMap)
      delete setWhisker.second;
  }

  groupWhiskers_.clear();
}

void
CQChartsBoxPlot::
addRawWhiskerRow(const ModelVisitor::VisitData &vdata) const
{
  auto *th = const_cast<CQChartsBoxPlot *>(this);

  // get value set id
  int      setId = -1;
  QVariant setVal;

  if (setColumn().isValid()) {
    CQChartsModelIndex setInd(vdata.row, setColumn(), vdata.parent);

    bool ok1;

    setVal = modelHierValue(setInd, ok1);

    if (! ok1) {
      th->addDataError(setInd, "Invalid set value");
      return;
    }

    setId = th->setValueInd_.calcId(setVal, setType_);
  }

  //---

  for (const auto &valueColumn : valueColumns()) {
    CQChartsModelIndex ind(vdata.row, valueColumn, vdata.parent);

    // get group
    int groupInd = rowGroupInd(ind);

    //---

    // add value to set
    bool ok2;

    double value = modelReal(ind, ok2);

    if (! ok2) {
      th->addDataError(ind, "Invalid value real");
      continue;
    }

    if (CMathUtil::isNaN(value))
      continue;

    QModelIndex yind  = modelIndex(ind);
    QModelIndex yind1 = normalizeIndex(yind);

    CQChartsBoxPlotValue wv(value, yind1);

    auto pg = groupWhiskers_.find(groupInd);

    if (pg == groupWhiskers_.end()) {
      auto pg1 = th->groupWhiskers_.find(groupInd);

      if (pg1 == th->groupWhiskers_.end())
        pg1 = th->groupWhiskers_.insert(pg1,
          GroupSetWhiskerMap::value_type(groupInd, SetWhiskerMap()));

      pg = groupWhiskers_.find(groupInd);
    }

    const SetWhiskerMap &setWhiskerMap = (*pg).second;

    auto ps = setWhiskerMap.find(setId);

    if (ps == setWhiskerMap.end()) {
      SetWhiskerMap &setWhiskerMap1 = const_cast<SetWhiskerMap &>(setWhiskerMap);

      auto ps1 = setWhiskerMap1.find(setId);

      if (ps1 == setWhiskerMap1.end()) {
        auto *whisker = new CQChartsBoxPlotWhisker;

        whisker->setRange(whiskerRange());

        QString name;
        bool    ok = false;

        if      (isGroupHeaders()) {
          name = modelHHeaderString(valueColumn, ok);
        }
        else if (setColumn().isValid()) {
          ok = CQChartsVariant::toString(setVal, name);
        }

        if (ok && name.length())
          whisker->setName(name);

        ps1 = setWhiskerMap1.insert(ps1, SetWhiskerMap::value_type(setId, whisker));
      }

      ps = setWhiskerMap.find(setId);
    }

    (*ps).second->addValue(wv);
  }
}

CQChartsAxis *
CQChartsBoxPlot::
mappedXAxis() const
{
  return (! isHorizontal() ? xAxis() : yAxis());
}

CQChartsAxis *
CQChartsBoxPlot::
mappedYAxis() const
{
  return (! isHorizontal() ? yAxis() : xAxis());
}

CQChartsGeom::BBox
CQChartsBoxPlot::
calcAnnotationBBox() const
{
  CQPerfTrace trace("CQChartsBoxPlot::calcAnnotationBBox");

  CQChartsGeom::BBox bbox;

  for (const auto &plotObj : plotObjs_) {
    auto *boxObj = dynamic_cast<CQChartsBoxPlotWhiskerObj *>(plotObj);

    if (boxObj)
      bbox += boxObj->annotationBBox();
  }

  return bbox;
}

//------

bool
CQChartsBoxPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsBoxPlot::createObjs");

  NoUpdate noUpdate(this);

  //---

  bool rc;

  if (! isPreCalc())
    rc = initRawObjs(objs);
  else
    rc = initCalcObjs(objs);

  //---

  return rc;
}

bool
CQChartsBoxPlot::
initRawObjs(PlotObjs &objs) const
{
  bool hasSets   = this->hasSets();
  bool hasGroups = this->hasGroups();

  double bw2 = lengthPlotSize(boxWidth   (), ! isHorizontal())/2.0;
  double vw2 = lengthPlotSize(violinWidth(), ! isHorizontal())/2.0;

  //---

  int ig = 0;
  int ng = numGroupWhiskers();

  for (const auto &groupIdWhiskers : this->groupWhiskers()) {
    int                  groupInd      = groupIdWhiskers.first;
    const SetWhiskerMap &setWhiskerMap = groupIdWhiskers.second;

    if (! isConnected()) {
      int is = 0;
      int ns = setWhiskerMap.size();

      double sf = (ns > 1 ? 1.0/ns : 1.0);

      for (const auto &setWhiskers : setWhiskerMap) {
        bool hidden = (isWhiskersGrouped() ? isSetHidden(ig) : isSetHidden(is));
        if (hidden) { ++is; continue; }

        int  setId   = setWhiskers.first;
        auto whisker = setWhiskers.second;

        if (whisker->values().empty())
          continue;
#if 0
        if (whisker->lowerMedian() >= whisker->upperMedian())
          continue;
#endif

        //----

        double pos = ig;
        double sbw = (isViolin() ? vw2 : bw2);

        if      (hasSets && hasGroups) {
          pos += (is + 1.0)/(ns + 1.0) - 0.5;
          sbw *= sf;
        }
        else if (hasSets)
          pos = setId;

        //---

        // create whisker object
        CQChartsGeom::BBox rect;

        if (! isNormalized())
          rect = CQChartsGeom::makeDirBBox(/*flipped*/ isHorizontal(),
                   pos - sbw, whisker->lowerMedian(), pos + sbw, whisker->upperMedian());
        else
          rect = CQChartsGeom::makeDirBBox(/*flipped*/ isHorizontal(),
                   pos - sbw, 0.0, pos + sbw, 1.0);

        auto *boxObj = new CQChartsBoxPlotWhiskerObj(this, rect, setId, groupInd, whisker,
                                                     ColorInd(is, ns), ColorInd(ig, ng));

        objs.push_back(boxObj);

        //---

        if (isShowOutliers()) {
          double ymargin = this->ymargin();

          double osx, osy;

          plotSymbolSize(outlierSymbolSize(), osx, osy);

          for (auto &o : whisker->outliers()) {
            double ovalue = whisker->rvalue(o);

            CQChartsGeom::BBox rect;

            if (! isNormalized()) {
              if (! isHorizontal())
                rect = CQChartsGeom::BBox(pos - osx, ovalue - osy, pos + osx, ovalue + osy);
              else
                rect = CQChartsGeom::BBox(ovalue - osx, pos - osy, ovalue + osx, pos + osy);
            }
            else {
              double ovalue1 =
                CMathUtil::map(ovalue, whisker->vmin(), whisker->vmax(), ymargin, 1.0 - ymargin);

              if (! isHorizontal())
                rect = CQChartsGeom::BBox(pos - osx, ovalue1 - osy, pos + osx, ovalue1 + osy);
              else
                rect = CQChartsGeom::BBox(ovalue1 - osx, pos - osy, ovalue1 + osx, pos + osy);
            }

            auto *outlierObj = new CQChartsBoxPlotOutlierObj(this, rect, setId, groupInd, whisker,
                                                             ColorInd(is, ns), ColorInd(ig, ng), o);

            objs.push_back(outlierObj);
          }
        }

        //---

        // create jitter or stacked points
        if      (isPointsJitter()) {
          addJitterPoints(groupInd, setId, pos, whisker, ColorInd(is, ns), ColorInd(ig, ng), objs);
        }
        else if (isPointsStacked()) {
          addStackedPoints(groupInd, setId, pos, whisker, ColorInd(is, ns), ColorInd(ig, ng), objs);
        }

        //---

        ++is;
      }
    }
    else {
      bool hidden = (isWhiskersGrouped() ? isSetHidden(ig) : false);
      if (hidden) { continue; }

      auto rect = getDataRange();

      auto *connectedObj = new CQChartsBoxPlotConnectedObj(this, rect, groupInd, ColorInd(ig, ng));

      objs.push_back(connectedObj);
    }

    //---

    ++ig;
  }

  //---

  return true;
}

void
CQChartsBoxPlot::
addJitterPoints(int groupInd, int setId, double pos, const CQChartsBoxPlotWhisker *whisker,
                const ColorInd &is, const ColorInd &ig, PlotObjs &objs) const
{
  double vw2 = lengthPlotSize(violinWidth(), ! isHorizontal())/2.0;

  const CQChartsDensity &density = whisker->density();

  double ymin = density.ymin1();
  double ymax = density.ymax1();

  CQChartsRand::RealInRange rand(-vw2, vw2);

  int nv = whisker->numValues();

  for (int iv = 0; iv < nv; ++iv) {
    const CQChartsBoxPlotValue &value = whisker->value(iv);

    double d = rand.gen();

    double yv = density.yval(value)/(ymax - ymin);

    double x = pos + yv*d;
    double y = value.value;

    double y1 = (isNormalized() ? whisker->normalize(y, isShowOutliers()) : y);

    CQChartsGeom::Point pos;
    CQChartsGeom::BBox  rect;

    if (! isHorizontal()) {
      pos = CQChartsGeom::Point(x, y1);

      if (! isNormalized())
        rect = CQChartsGeom::BBox(x - 0.1, y1 - 0.1, x + 0.1, y1 + 0.1);
      else
        rect = CQChartsGeom::BBox(x - 0.01, y1 - 0.01, x + 0.01, y1 + 0.01);
    }
    else {
      pos = CQChartsGeom::Point(y1, x);

      if (! isNormalized())
        rect = CQChartsGeom::BBox(y1 - 0.1, x - 0.1, y1 + 0.1, x + 0.1);
      else
        rect = CQChartsGeom::BBox(y1 - 0.01, x - 0.01, y1 + 0.01, x + 0.01);
    }

    auto *pointObj = new CQChartsBoxPlotPointObj(this, rect, setId, groupInd, pos, value.ind,
                                                 is, ig, ColorInd(iv, nv));

    objs.push_back(pointObj);
  }
}

void
CQChartsBoxPlot::
addStackedPoints(int groupInd, int setId, double pos, const CQChartsBoxPlotWhisker *whisker,
                 const ColorInd &is, const ColorInd &ig, PlotObjs &objs) const
{
  using Rects    = std::vector<CQChartsGeom::BBox>;
  using PosRects = std::map<int,Rects>;

  PosRects posRects;

  auto placePosRect = [&](int pos, const CQChartsGeom::BBox &rect) {
    Rects &rects = posRects[pos];

    for (auto &r : rects) {
      if (r.intersect(rect))
        return false;
    }

    rects.push_back(rect);

    return true;
  };

  auto placeRect = [&](const CQChartsGeom::BBox &rect, CQChartsGeom::BBox &prect) {
    if (placePosRect(0, rect))
      return false;

    double w = rect.getWidth();

    int d = 1;

    while (true) {
      auto rect1 = rect;

      if (! isNormalized())
        rect1.moveBy(CQChartsGeom::Point(-d*w, 0.0));
      else
        rect1.moveBy(CQChartsGeom::Point(0.0, -d*w));

      if (placePosRect(-d, rect1)) {
        prect = rect1;
        return true;
      }

      auto rect2 = rect;

      if (! isNormalized())
        rect2.moveBy(CQChartsGeom::Point(d*w, 0.0));
      else
        rect2.moveBy(CQChartsGeom::Point(0.0, d*w));

      if (placePosRect(d, rect2)) {
        prect = rect2;
        return true;
      }

      ++d;
    }
  };

  int nv = whisker->numValues();

  for (int iv = 0; iv < nv; ++iv) {
    const CQChartsBoxPlotValue &value = whisker->value(iv);

    double x = pos;
    double y = value.value;

    double y1 = (isNormalized() ? whisker->normalize(y, isShowOutliers()) : y);

    double sx, sy;

    plotSymbolSize(jitterSymbolSize(), sx, sy);

    CQChartsGeom::Point pos;
    CQChartsGeom::BBox  rect;

    if (! isHorizontal())
      pos = CQChartsGeom::Point(x, y1);
    else
      pos = CQChartsGeom::Point(y1, x);

    rect = CQChartsGeom::BBox(pos.x - sx, pos.y - sy, pos.x + sx, pos.y + sy);

    CQChartsBoxPlotPointObj *pointObj = nullptr;

    CQChartsGeom::BBox prect;

    if (placeRect(rect, prect)) {
      auto ppos = pos;

      if (! isHorizontal())
        ppos.setX(prect.getXMid());
      else
        ppos.setY(prect.getYMid());

      pointObj = new CQChartsBoxPlotPointObj(this, prect, setId, groupInd, ppos, value.ind,
                                             is, ig, ColorInd(iv, nv));
    }
    else {
      pointObj = new CQChartsBoxPlotPointObj(this, rect, setId, groupInd, pos, value.ind,
                                             is, ig, ColorInd(iv, nv));
    }

    objs.push_back(pointObj);
  }
}

bool
CQChartsBoxPlot::
initCalcObjs(PlotObjs &objs) const
{
  int is = 0;
  int ns = whiskerDataList_.size();

  //int ipos = 0;

  double bw = 0.1;

  for (const auto &whiskerData : whiskerDataList_) {
    double pos = whiskerData.x;

    CQChartsGeom::BBox rect;

    if (! isNormalized())
      rect = CQChartsGeom::makeDirBBox(/*flipped*/isHorizontal(),
               pos - bw, whiskerData.statData.lowerMedian,
               pos + bw, whiskerData.statData.upperMedian);
    else
      rect = CQChartsGeom::makeDirBBox(/*flipped*/isHorizontal(),
               pos - bw, 0.0, pos + bw, 1.0);

    auto *boxObj = new CQChartsBoxPlotDataObj(this, rect, whiskerData, ColorInd(is, ns));

    objs.push_back(boxObj);

    //++ipos;

    //---

    if (isShowOutliers()) {
      double ymargin = this->ymargin();

      double osx, osy;

      plotSymbolSize(outlierSymbolSize(), osx, osy);

      int io = 0;

      for (auto &ovalue : whiskerData.outliers) {
        CQChartsGeom::BBox rect;

        if (! isNormalized()) {
          if (! isHorizontal())
            rect = CQChartsGeom::BBox(pos - osx, ovalue - osy, pos + osx, ovalue + osy);
          else
            rect = CQChartsGeom::BBox(ovalue - osx, pos - osy, ovalue + osx, pos + osy);
        }
        else {
          double ovalue1 =
            CMathUtil::map(ovalue,
                           whiskerData.statData.lowerMedian, whiskerData.statData.upperMedian,
                           ymargin, 1.0 - ymargin);

          if (! isHorizontal())
            rect = CQChartsGeom::BBox(pos - osx, ovalue1 - osy, pos + osx, ovalue1 + osy);
          else
            rect = CQChartsGeom::BBox(ovalue1 - osx, pos - osy, ovalue1 + osx, pos + osy);
        }

        auto *outlierObj = new CQChartsBoxPlotOutlierObj(this, rect, -1, -1, nullptr,
                                                         ColorInd(is, ns), ColorInd(), io);

        objs.push_back(outlierObj);

        ++io;
      }
    }

    //---

    ++is;
  }

  return true;
}

void
CQChartsBoxPlot::
addKeyItems(CQChartsPlotKey *key)
{
  int ng = numGroupWhiskers();

  // if has groups
  if      (ng > 1) {
    // if color by set add key item per set
    if (hasSets() && isColorBySet()) {
      auto pg = this->groupWhiskers().begin();

      int                  groupInd      = (*pg).first;
      const SetWhiskerMap &setWhiskerMap = (*pg).second;

      QString groupName = groupIndName(groupInd);

      int is = 0;
      int ns = setWhiskerMap.size();

      for (const auto &setWhiskers : setWhiskerMap) {
        int  setId   = setWhiskers.first;
      //auto whisker = setWhiskers.second;

        QString setName = setIdName(setId);

        ColorInd sc(is, ns), gc;

        auto *color = new CQChartsBoxKeyColor(this, sc, gc);
        auto *text  = new CQChartsBoxKeyText (this, setName, sc, gc);

        key->addItem(color, is, 0);
        key->addItem(text , is, 1);

        ++is;
      }
    }
    // if not color by set add key item per group
    else {
      int ig = 0;

      for (const auto &groupIdWhiskers : this->groupWhiskers()) {
        int groupInd = groupIdWhiskers.first;

        QString groupName = groupIndName(groupInd);

        ColorInd sc, gc(ig, ng);

        auto *color = new CQChartsBoxKeyColor(this, sc, gc);
        auto *text  = new CQChartsBoxKeyText (this, groupName, sc, gc);

        key->addItem(color, ig, 0);
        key->addItem(text , ig, 1);

        ++ig;
      }
    }
  }
  // if single group then add key per set
  else if (ng > 0) {
    auto pg = this->groupWhiskers().begin();

    int                  groupInd      = (*pg).first;
    const SetWhiskerMap &setWhiskerMap = (*pg).second;

    QString groupName = groupIndName(groupInd);

    int is = 0;
    int ns = setWhiskerMap.size();

    for (const auto &setWhiskers : setWhiskerMap) {
    //int  setId   = setWhiskers.first;
      auto whisker = setWhiskers.second;

      QString name = whisker->name();

      ColorInd sc(is, ns), gc;

      auto *color = new CQChartsBoxKeyColor(this, sc, gc);
      auto *text  = new CQChartsBoxKeyText (this, name, sc, gc);

      key->addItem(color, is, 0);
      key->addItem(text , is, 1);

      ++is;
    }
  }

  key->plot()->updateKeyPosition(/*force*/true);
}

bool
CQChartsBoxPlot::
probe(ProbeData &probeData) const
{
  const auto &dataRange = this->dataRange();

  if (! dataRange.isSet())
    return false;

  if (! isHorizontal()) {
    probeData.p.x =
      std::min(std::max(probeData.p.x, dataRange.xmin() + 0.5), dataRange.xmax() - 0.5);
    probeData.p.x = std::round(probeData.p.x);
  }
  else {
    probeData.p.y =
      std::min(std::max(probeData.p.y, dataRange.ymin() + 0.5), dataRange.ymax() - 0.5);
    probeData.p.y = std::round(probeData.p.y);
  }

  return true;
}

//------

bool
CQChartsBoxPlot::
addMenuItems(QMenu *menu)
{
  auto addMenuCheckedAction = [&](QMenu *menu, const QString &name, bool isSet, const char *slot) {
    auto *action = new QAction(name, menu);

    action->setCheckable(true);
    action->setChecked(isSet);

    connect(action, SIGNAL(triggered(bool)), this, slot);

    menu->addAction(action);

    return action;
  };

  auto addCheckedAction = [&](const QString &name, bool isSet, const char *slot) {
    return addMenuCheckedAction(menu, name, isSet, slot);
  };

  //---

  menu->addSeparator();

  (void) addCheckedAction("Horizontal", isHorizontal(), SLOT(setHorizontal(bool)));
  (void) addCheckedAction("Normalized", isNormalized(), SLOT(setNormalized(bool)));

  //---

  // following items only allowed if we have individual data points
  if (! isPreCalc()) {
    auto *pointsMenu = new QMenu("Points", menu);

    (void) addMenuCheckedAction(pointsMenu, "Jitter" , isPointsJitter(),
                                SLOT(setPointsJitter(bool)));
    (void) addMenuCheckedAction(pointsMenu, "Stacked", isPointsStacked(),
                                SLOT(setPointsStacked(bool)));

    menu->addMenu(pointsMenu);

    //---

    (void) addCheckedAction("Notched"  , isNotched() , SLOT(setNotched(bool)));
    (void) addCheckedAction("Violin"   , isViolin()  , SLOT(setViolin(bool)));
    (void) addCheckedAction("Error Bar", isErrorBar(), SLOT(setErrorBar(bool)));
  }

  return true;
}

bool
CQChartsBoxPlot::
hasXAxis() const
{
  if (isHorizontal() && forceNoYAxis_)
    return false;

  return CQChartsPlot::hasXAxis();
}

bool
CQChartsBoxPlot::
hasYAxis() const
{
  if (! isHorizontal() && forceNoYAxis_)
    return false;

  return CQChartsPlot::hasYAxis();
}

//------

CQChartsBoxPlotWhiskerObj::
CQChartsBoxPlotWhiskerObj(const CQChartsBoxPlot *plot, const CQChartsGeom::BBox &rect, int setId,
                          int groupInd, const CQChartsBoxPlotWhisker *whisker,
                          const ColorInd &is, const ColorInd &ig) :
 CQChartsBoxPlotObj(plot, rect, is, ig, ColorInd()), setId_(setId), groupInd_(groupInd),
 whisker_(whisker)
{
  assert(whisker_);

  setDetailHint(DetailHint::MAJOR);

  for (auto &value : whisker_->values())
    addModelInd(value.ind);
}

double
CQChartsBoxPlotWhiskerObj::
pos() const
{
  return rect_.getXYMid(! plot_->isHorizontal());
}

double
CQChartsBoxPlotWhiskerObj::
min() const
{
  return (whisker_ ? whisker_->min() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
lowerMedian() const
{
  return (whisker_ ? whisker_->lowerMedian() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
median() const
{
  return (whisker_ ? whisker_->median() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
upperMedian() const
{
  return (whisker_ ? whisker_->upperMedian() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
max() const
{
  return (whisker_ ? whisker_->max() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
mean() const
{
  return (whisker_ ? whisker_->mean() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
stddev() const
{
  return (whisker_ ? whisker_->stddev() : 0.0);
}

double
CQChartsBoxPlotWhiskerObj::
notch() const
{
  return (whisker_ ? whisker_->notch() : 0.0);
}

QString
CQChartsBoxPlotWhiskerObj::
calcId() const
{
  if (setId_ >= 0)
    return QString("%1:%2:%3").arg(typeName()).arg(setId_).arg(groupInd_);
  else
    return QString("%1:%2").arg(typeName()).arg(groupInd_);
}

QString
CQChartsBoxPlotWhiskerObj::
calcTipId() const
{
  QString setName, groupName, name;

  if (plot_->hasSets())
    setName = plot_->setIdName(setId_);

  if (plot_->hasGroups())
    groupName = plot_->groupIndName(groupInd_);

  if (! setName.length() && ! groupName.length())
    name = (whisker_ ? whisker_->name() : "");

  CQChartsTableTip tableTip;

  if (setName.length())
    tableTip.addTableRow("Set", setName);

  if (groupName.length())
    tableTip.addTableRow("Group", groupName);

  if (name.length())
    tableTip.addTableRow("Name", name);

  if (plot_->isErrorBar()) {
    tableTip.addTableRow("Mean"  , mean  ());
    tableTip.addTableRow("StdDev", stddev());
  }
  else {
    tableTip.addTableRow("Min"         , min        ());
    tableTip.addTableRow("Lower Median", lowerMedian());
    tableTip.addTableRow("Median"      , median     ());
    tableTip.addTableRow("Upper Median", upperMedian());
    tableTip.addTableRow("Max"         , max        ());
  }

  //---

  //plot()->addTipColumns(tableTip, node1_->ind());

  //---

  return tableTip.str();
}

//---

void
CQChartsBoxPlotWhiskerObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  QString path1 = path + "/" + propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "rect"    )->setDesc("Bounding box");
//model->addProperty(path1, this, "selected")->setDesc("Is selected");

  model->addProperty(path1, this, "pos"        )->setDesc("Position");
  model->addProperty(path1, this, "min"        )->setDesc("Minumum");
  model->addProperty(path1, this, "lowerMedian")->setDesc("Lower median");
  model->addProperty(path1, this, "median"     )->setDesc("Median");
  model->addProperty(path1, this, "upperMedian")->setDesc("Upper median");
  model->addProperty(path1, this, "max"        )->setDesc("Maximum");
  model->addProperty(path1, this, "mean"       )->setDesc("Mean");
  model->addProperty(path1, this, "stddev"     )->setDesc("Standard deviation");
  model->addProperty(path1, this, "notch"      )->setDesc("Notch");
}

//---

void
CQChartsBoxPlotWhiskerObj::
getSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, plot_->setColumn  ());
  addColumnSelectIndex(inds, plot_->groupColumn());
}

bool
CQChartsBoxPlotWhiskerObj::
inside(const CQChartsGeom::Point &p) const
{
  if (plot_->isViolin())
    return poly_.containsPoint(p, Qt::OddEvenFill);

  return CQChartsBoxPlotObj::inside(p);
}

void
CQChartsBoxPlotWhiskerObj::
draw(CQChartsPaintDevice *device)
{
  device->setColorNames();

  //---

  // get color index
  ColorInd colorInd = this->calcColorInd();

  if (plot_->hasSets() && plot_->isColorBySet())
    colorInd = is_;

  //---

  // set fill and stroke
  CQChartsPenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // set whisker fill and stroke
  CQChartsPenBrush whiskerPenBrush;

  plot_->setWhiskerLineDataPen(whiskerPenBrush.pen, colorInd);

  plot_->setBrush(whiskerPenBrush.brush, false);

  plot_->updateObjPenBrushState(this, whiskerPenBrush);

  //---

  Qt::Orientation orientation = (! plot_->isHorizontal() ? Qt::Vertical : Qt::Horizontal);

  //---

  double pos = this->pos();

  double ww = plot_->whiskerExtent();
  double bw = plot_->lengthPlotSize(plot_->boxWidth(), plot_->isHorizontal());

  CQStatData statData;

  statData.min         = remapPos(this->min());
  statData.lowerMedian = remapPos(this->lowerMedian());
  statData.median      = remapPos(this->median());
  statData.upperMedian = remapPos(this->upperMedian());
  statData.max         = remapPos(this->max());

  statData.lnotch = remapPos(this->median() - this->notch());
  statData.unotch = remapPos(this->median() + this->notch());

  //---

  bool drawBox       = true;
  bool drawBoxFilled = true;

  // draw violin
  if (plot_->isViolin()) {
    const CQChartsDensity &density = whisker_->density();

    double vw = plot_->lengthPlotSize(plot_->violinWidth(), plot_->isHorizontal())/2.0;

    CQChartsGeom::BBox rect =
      CQChartsGeom::makeDirBBox(/*flipped*/plot_->isHorizontal(),
                                pos - vw, statData.min, pos + vw, statData.max);

    CQChartsWhiskerOpts opts;

    opts.violin = true;

    auto poly1 = poly_;

    density.calcDistributionPoly(poly1, plot_, rect, orientation, opts);

    device->drawPolygon(poly1);

    drawBox       = plot_->isViolinBox();
    drawBoxFilled = false;
  }

  //---

  // draw error bar
  if (plot_->isErrorBar()) {
    device->setPen(whiskerPenBrush.pen);

    //---

    double mean = remapPos(this->mean());
    double dev1 = remapPos(this->mean() - this->stddev());
    double dev2 = remapPos(this->mean() + this->stddev());

    CQChartsGeom::BBox rect =
      CQChartsGeom::makeDirBBox(/*flipped*/plot_->isHorizontal(),
                                pos - bw/2.0, dev1, pos + bw/2.0, dev2);

    if      (plot_->errorBarType() == CQChartsBoxPlot::ErrorBarType::CROSS_BAR) {
      CQChartsDensity::drawCrossBar(plot_, device, rect, mean, orientation,
                                    plot_->boxCornerSize());
    }
    else if (plot_->errorBarType() == CQChartsBoxPlot::ErrorBarType::ERROR_BAR) {
      CQChartsSymbolData symbol;

      symbol.setType(CQChartsSymbol::Type::CIRCLE);
      symbol.setSize(plot_->outlierSymbolSize());

      CQChartsDensity::drawErrorBar(plot_, device, rect, mean, orientation,
                                    symbol);
    }
    else if (plot_->errorBarType() == CQChartsBoxPlot::ErrorBarType::POINT_RANGE) {
      // set fill and stroke
      CQChartsPenBrush symbolPenBrush;

      QColor boxColor    = plot_->interpBoxFillColor  (colorInd);
      QColor strokeColor = plot_->interpBoxStrokeColor(colorInd);

      plot_->setPenBrush(symbolPenBrush,
        CQChartsPenData  (true, strokeColor, plot_->boxStrokeAlpha(),
                          plot_->boxStrokeWidth(), plot_->boxStrokeDash()),
        CQChartsBrushData(true, boxColor, plot_->boxFillAlpha(), plot_->boxFillPattern()));

      plot_->updateObjPenBrushState(this, symbolPenBrush, CQChartsPlot::DrawType::SYMBOL);

      CQChartsDrawUtil::setPenBrush(device, symbolPenBrush);

      //---

      CQChartsSymbolData symbol;

      symbol.setType(CQChartsSymbol::Type::CIRCLE);
      symbol.setSize(plot_->outlierSymbolSize());

      CQChartsDensity::drawPointRange(plot_, device, rect, mean, orientation, symbol);
    }
    else if (plot_->errorBarType() == CQChartsBoxPlot::ErrorBarType::LINE_RANGE) {
      CQChartsDensity::drawLineRange(plot_, device, rect, orientation);
    }

    drawBox = false;
  }

  //---

  // draw notched box
  if (drawBox) {
    device->setPen(whiskerPenBrush.pen);

    //---

    if (! drawBoxFilled) {
      QColor boxColor = plot_->interpThemeColor(ColorInd());

      penBrush.brush.setColor(boxColor);

      device->setBrush(penBrush.brush);
    }

    //---

    bool median = true;

    std::vector<double> outliers;

    CQChartsBoxWhiskerUtil::drawWhiskerBar(plot_, device, statData, pos, orientation, ww, bw,
                                           plot_->boxCornerSize(), plot_->isNotched(),
                                           median, outliers);
  }

  //---

  if (plot_->isErrorBar()) {
  }
  else {
    double wd1 = ww/2.0;
    double wd2 = bw/2.0;

    if (! device->isInteractive() ||
        plot_->drawLayerType() == CQChartsLayer::Type::MID_PLOT) {
      auto posToPixel = [&](double pos, double value) {
        if (! plot_->isHorizontal())
          return plot_->windowToPixel(CQChartsGeom::Point(pos, value));
        else
          return plot_->windowToPixel(CQChartsGeom::Point(value, pos));
      };

      auto p1 = posToPixel(pos - wd1, statData.min        );
      auto p2 = posToPixel(pos - wd2, statData.lowerMedian);
      auto p3 = posToPixel(pos      , statData.median     );
      auto p4 = posToPixel(pos + wd2, statData.upperMedian);
      auto p5 = posToPixel(pos + wd1, statData.max        );

      // draw labels
      if (plot_->isTextVisible()) {
        plot_->view()->setPlotPainterFont(plot_, device, plot_->textFont());

        //---

        QPen pen;

        QColor tc = plot_->interpTextColor(colorInd);

        plot_->setPen(pen, true, tc, plot_->textAlpha());

        device->setPen(pen);

        //---

        bool hasRange = (fabs(this->max() - this->min()) > 1E-6);

        if (hasRange) {
          QString strl = QString("%1").arg(this->min        ());
          QString lstr = QString("%1").arg(this->lowerMedian());
          QString mstr = QString("%1").arg(this->median     ());
          QString ustr = QString("%1").arg(this->upperMedian());
          QString strh = QString("%1").arg(this->max        ());

          if (! plot_->isHorizontal()) {
            drawHText(device, p1.x, p5.x, p1.y, strl, /*onLeft*/true );
            drawHText(device, p2.x, p4.x, p2.y, lstr, /*onLeft*/false);
            drawHText(device, p2.x, p4.x, p3.y, mstr, /*onLeft*/true );
            drawHText(device, p2.x, p4.x, p4.y, ustr, /*onLeft*/false);
            drawHText(device, p1.x, p5.x, p5.y, strh, /*onLeft*/true );
          }
          else {
            drawVText(device, p1.y, p5.y, p1.x, strl, /*onBottom*/false);
            drawVText(device, p2.y, p4.y, p2.x, lstr, /*onBottom*/true );
            drawVText(device, p2.y, p4.y, p3.x, mstr, /*onBottom*/false);
            drawVText(device, p2.y, p4.y, p4.x, ustr, /*onBottom*/true );
            drawVText(device, p1.y, p5.y, p5.x, strh, /*onBottom*/false);
          }
        }
        else {
          QString strl = QString("%1").arg(this->min());

          if (! plot_->isHorizontal())
            drawHText(device, p1.x, p5.x, p1.y, strl, /*onLeft*/true);
          else
            drawVText(device, p1.y, p5.y, p1.x, strl, /*onBottom*/false);
        }
      }
    }
  }

  //---

  device->resetColorNames();
}

void
CQChartsBoxPlotWhiskerObj::
calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const
{
  // get color index
  ColorInd colorInd = this->calcColorInd();

  if (plot_->hasSets() && plot_->isColorBySet())
    colorInd = is_;

  //---

  // set fill and stroke
  QColor bc = plot_->interpBoxStrokeColor(colorInd);
  QColor fc = plot_->interpBoxFillColor  (colorInd);

  plot_->setPenBrush(penBrush,
    CQChartsPenData  (plot_->isBoxStroked(), bc, plot_->boxStrokeAlpha(),
                      plot_->boxStrokeWidth(), plot_->boxStrokeDash()),
    CQChartsBrushData(plot_->isBoxFilled(), fc, plot_->boxFillAlpha(), plot_->boxFillPattern()));

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

void
CQChartsBoxPlotWhiskerObj::
writeScriptData(CQChartsScriptPainter *device) const
{
  calcPenBrush(penBrush_, /*updateState*/ false);

  CQChartsPlotObj::writeScriptData(device);

  //---

  std::ostream &os = device->os();

  os << "\n";
  os << "  this.pos         = " << pos        () << ";\n";
  os << "  this.min         = " << min        () << ";\n";
  os << "  this.lowerMedian = " << lowerMedian() << ";\n";
  os << "  this.median      = " << median     () << ";\n";
  os << "  this.upperMedian = " << upperMedian() << ";\n";
  os << "  this.max         = " << max        () << ";\n";
  os << "  this.mean        = " << mean       () << ";\n";
  os << "  this.stddev      = " << stddev     () << ";\n";
  os << "  this.notch       = " << notch      () << ";\n";
}

CQChartsGeom::BBox
CQChartsBoxPlotWhiskerObj::
annotationBBox() const
{
  if (plot_->isErrorBar())
    return CQChartsGeom::BBox();

  //---

  double pos = this->pos();

  double ww = plot_->whiskerExtent();
  double bw = plot_->lengthPlotSize(plot_->boxWidth(), plot_->isHorizontal());

  double wd1 = ww/2.0;
  double wd2 = bw/2.0;

  auto posToRemapPixel = [&](double pos, double value) {
    if (! plot_->isHorizontal())
      return plot_->windowToPixel(CQChartsGeom::Point(pos, remapPos(value)));
    else
      return plot_->windowToPixel(CQChartsGeom::Point(remapPos(value), pos));
  };

  auto p1 = posToRemapPixel(pos - wd1, min        ());
  auto p2 = posToRemapPixel(pos - wd2, lowerMedian());
  auto p3 = posToRemapPixel(pos      , median     ());
  auto p4 = posToRemapPixel(pos + wd2, upperMedian());
  auto p5 = posToRemapPixel(pos + wd1, max        ());

  //---

  CQChartsGeom::BBox pbbox;

  if (plot_->isTextVisible()) {
    bool hasRange = (fabs(max() - min()) > 1E-6);

    if (hasRange) {
      QString strl = QString("%1").arg(min        ());
      QString lstr = QString("%1").arg(lowerMedian());
      QString mstr = QString("%1").arg(median     ());
      QString ustr = QString("%1").arg(upperMedian());
      QString strh = QString("%1").arg(max        ());

      if (! plot_->isHorizontal()) {
        addHBBox(pbbox, p1.x, p5.x, p1.y, strl, /*onLeft*/false);
        addHBBox(pbbox, p2.x, p4.x, p2.y, lstr, /*onLeft*/true );
        addHBBox(pbbox, p2.x, p4.x, p3.y, mstr, /*onLeft*/false);
        addHBBox(pbbox, p2.x, p4.x, p4.y, ustr, /*onLeft*/true );
        addHBBox(pbbox, p1.x, p5.x, p5.y, strh, /*onLeft*/false);
      }
      else {
        addVBBox(pbbox, p1.y, p5.y, p1.x, strl, /*onBottom*/true );
        addVBBox(pbbox, p2.y, p4.y, p2.x, lstr, /*onBottom*/false);
        addVBBox(pbbox, p2.y, p4.y, p3.x, mstr, /*onBottom*/true );
        addVBBox(pbbox, p2.y, p4.y, p4.x, ustr, /*onBottom*/false);
        addVBBox(pbbox, p1.y, p5.y, p5.x, strh, /*onBottom*/true );
      }
    }
    else {
      QString strl = QString("%1").arg(min());

      if (! plot_->isHorizontal())
        addHBBox(pbbox, p1.x, p5.x, p1.y, strl, /*onLeft*/false);
      else
        addVBBox(pbbox, p1.y, p5.y, p1.x, strl, /*onBottom*/true);
    }
  }
  else {
    pbbox += p1;
    pbbox += p2;
    pbbox += p3;
    pbbox += p4;
    pbbox += p5;
  }

  //---

  auto bbox = plot_->pixelToWindow(pbbox);

  return bbox;
}

double
CQChartsBoxPlotWhiskerObj::
remapPos(double y) const
{
  // remap to margin -> 1.0 - margin
  if (! plot_->isNormalized())
    return y;

  double ymargin = plot_->ymargin();

  if (plot_->isShowOutliers())
    return CMathUtil::map(y, whisker_->vmin(), whisker_->vmax(), ymargin, 1.0 - ymargin);
  else
    return CMathUtil::map(y, min(), max(), ymargin, 1.0 - ymargin);
}

//------

CQChartsBoxPlotOutlierObj::
CQChartsBoxPlotOutlierObj(const CQChartsBoxPlot *plot, const CQChartsGeom::BBox &rect, int setId,
                          int groupInd, const CQChartsBoxPlotWhisker *whisker,
                          const ColorInd &is, const ColorInd &ig, int io) :
 CQChartsBoxPlotObj(plot, rect, is, ig, ColorInd()), setId_(setId), groupInd_(groupInd),
 whisker_(whisker), io_(io)
{
  if (whisker_) {
    const CQChartsBoxPlotValue &ovalue = whisker_->value(io_);

    setModelInd(ovalue.ind);
  }
}

QString
CQChartsBoxPlotOutlierObj::
calcId() const
{
  return QString("%1:%2:%3:%4").arg(typeName()).arg(setId_).arg(groupInd_).arg(io_);
}

QString
CQChartsBoxPlotOutlierObj::
calcTipId() const
{
  QString setName, groupName, name;

  if (plot_->hasSets())
    setName = plot_->setIdName(setId_);

  if (plot_->hasGroups())
    groupName = plot_->groupIndName(groupInd_);

  if (whisker_ && ! setName.length() && ! groupName.length())
    name = whisker_->name();

  CQChartsTableTip tableTip;

  if (setName.length())
    tableTip.addTableRow("Set", setName);

  if (groupName.length())
    tableTip.addTableRow("Group", groupName);

  if (name.length())
    tableTip.addTableRow("Name", name);

  //---

  if (whisker_) {
    const CQChartsBoxPlotValue &ovalue = whisker_->value(io_);

    double rvalue = double(ovalue);

    tableTip.addTableRow("Value", rvalue);

    //---

    plot()->addTipColumns(tableTip, ovalue.ind);
  }

  //---

  return tableTip.str();
}

void
CQChartsBoxPlotOutlierObj::
getSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, plot_->setColumn  ());
  addColumnSelectIndex(inds, plot_->groupColumn());

  //---

  if (whisker_) {
    const CQChartsBoxPlotValue &ovalue = whisker_->value(io_);

    addSelectIndex(inds, ovalue.ind.row(), CQChartsColumn(ovalue.ind.column()),
                   ovalue.ind.parent());
  }
}

void
CQChartsBoxPlotOutlierObj::
draw(CQChartsPaintDevice *device)
{
  // get color index
  ColorInd colorInd = this->calcColorInd();

  if (plot_->hasSets() && plot_->isColorBySet())
    colorInd = is_;

  //---

  // set fill and stroke
  CQChartsPenBrush penBrush;

  plot_->setOutlierSymbolPenBrush(penBrush, colorInd);

  plot_->updateObjPenBrushState(this, penBrush, CQChartsPlot::DrawType::SYMBOL);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw symbol
  double ox = rect_.getXYMid(! plot_->isHorizontal());
  double oy = rect_.getXYMid(  plot_->isHorizontal());

  CQChartsGeom::Point pos(ox, oy);

  plot_->drawSymbol(device, pos, plot_->outlierSymbolType(),
                    plot_->outlierSymbolSize(), penBrush);
}

double
CQChartsBoxPlotOutlierObj::
remapPos(double y) const
{
  // remap to margin -> 1.0 - margin
  if (! whisker_ || ! plot_->isNormalized())
    return y;

  double ymargin = plot_->ymargin();

  return CMathUtil::map(y, whisker_->vmin(), whisker_->vmax(), ymargin, 1.0 - ymargin);
}

//------

CQChartsBoxPlotDataObj::
CQChartsBoxPlotDataObj(const CQChartsBoxPlot *plot, const CQChartsGeom::BBox &rect,
                       const CQChartsBoxWhiskerData &data, const ColorInd &is) :
 CQChartsBoxPlotObj(plot, rect, is, ColorInd(), ColorInd()), data_(data)
{
  setModelInd(data_.ind);
}

double
CQChartsBoxPlotDataObj::
pos() const
{
  return rect_.getXYMid(! plot_->isHorizontal());
}

QString
CQChartsBoxPlotDataObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(data_.name);
}

QString
CQChartsBoxPlotDataObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Name"        , data_.name                );
  tableTip.addTableRow("Min"         , data_.statData.min        );
  tableTip.addTableRow("Lower Median", data_.statData.lowerMedian);
  tableTip.addTableRow("Median"      , data_.statData.median     );
  tableTip.addTableRow("Upper Median", data_.statData.upperMedian);
  tableTip.addTableRow("Max"         , data_.statData.max        );

  //---

  plot()->addTipColumns(tableTip, data_.ind);

  //---

  return tableTip.str();
}

void
CQChartsBoxPlotDataObj::
getSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, plot_->xColumn          ());
  addColumnSelectIndex(inds, plot_->minColumn        ());
  addColumnSelectIndex(inds, plot_->lowerMedianColumn());
  addColumnSelectIndex(inds, plot_->medianColumn     ());
  addColumnSelectIndex(inds, plot_->upperMedianColumn());
  addColumnSelectIndex(inds, plot_->maxColumn        ());
  addColumnSelectIndex(inds, plot_->outliersColumn   ());
}

void
CQChartsBoxPlotDataObj::
draw(CQChartsPaintDevice *device)
{
  // set whisker fill and stroke
  CQChartsPenBrush whiskerPenBrush;

  plot_->setWhiskerLineDataPen(whiskerPenBrush.pen, ColorInd());

  plot_->setBrush(whiskerPenBrush.brush, false);

  plot_->updateObjPenBrushState(this, whiskerPenBrush);

  //---

  // set fill and stroke
  CQChartsPenBrush penBrush;

  QColor bc = plot_->interpBoxStrokeColor(ColorInd());
  QColor fc = plot_->interpBoxFillColor(ColorInd());

  plot_->setPenBrush(penBrush,
    CQChartsPenData  (plot_->isBoxStroked(), bc, plot_->boxStrokeAlpha(),
                      plot_->boxStrokeWidth(), plot_->boxStrokeDash()),
    CQChartsBrushData(plot_->isBoxFilled(), fc, plot_->boxFillAlpha(), plot_->boxFillPattern()));

  plot_->updateObjPenBrushState(this, penBrush);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  double pos = this->pos();

  double ww = plot_->whiskerExtent();
  double bw = plot_->lengthPlotSize(plot_->boxWidth(), plot_->isHorizontal());

  //---

  Qt::Orientation orientation = (! plot_->isHorizontal() ? Qt::Vertical : Qt::Horizontal);

  //---

  device->setPen(whiskerPenBrush.pen);

  CQStatData statData;

  statData.min         = remapPos(data_.statData.min);
  statData.lowerMedian = remapPos(data_.statData.lowerMedian);
  statData.median      = remapPos(data_.statData.median     );
  statData.upperMedian = remapPos(data_.statData.upperMedian);
  statData.max         = remapPos(data_.statData.max);

  bool notched = false;
  bool median  = true;

  std::vector<double> outliers;

  CQChartsBoxWhiskerUtil::drawWhiskerBar(plot_, device, statData, pos, orientation, ww, bw,
                                         plot_->boxCornerSize(), notched, median, outliers);

  //---

  // draw labels
  if (plot_->isTextVisible()) {
    double wd1 = ww/2.0;
    double wd2 = bw/2.0;

    auto posToRemapPixel = [&](double pos, double value) {
      if (! plot_->isHorizontal())
        return plot_->windowToPixel(CQChartsGeom::Point(pos, remapPos(value)));
      else
        return plot_->windowToPixel(CQChartsGeom::Point(remapPos(value), pos));
    };

    auto p1 = posToRemapPixel(pos - wd1, data_.statData.min        );
    auto p2 = posToRemapPixel(pos - wd2, data_.statData.lowerMedian);
    auto p3 = posToRemapPixel(pos      , data_.statData.median     );
    auto p4 = posToRemapPixel(pos + wd2, data_.statData.upperMedian);
    auto p5 = posToRemapPixel(pos + wd1, data_.statData.max        );

    //---

    plot_->view()->setPlotPainterFont(plot_, device, plot_->textFont());

    //---

    QPen pen;

    QColor tc = plot_->interpTextColor(ColorInd());

    plot_->setPen(pen, true, tc, plot_->textAlpha());

    device->setPen(pen);

    //---

    QString strl = QString("%1").arg(data_.statData.min        );
    QString lstr = QString("%1").arg(data_.statData.lowerMedian);
    QString mstr = QString("%1").arg(data_.statData.median     );
    QString ustr = QString("%1").arg(data_.statData.upperMedian);
    QString strh = QString("%1").arg(data_.statData.max        );

    if (! plot_->isHorizontal()) {
      drawHText(device, p1.x, p5.x, p1.y, strl, /*onLeft*/false);
      drawHText(device, p2.x, p4.x, p2.y, lstr, /*onLeft*/true );
      drawHText(device, p2.x, p4.x, p3.y, mstr, /*onLeft*/false);
      drawHText(device, p2.x, p4.x, p4.y, ustr, /*onLeft*/true );
      drawHText(device, p1.x, p5.x, p5.y, strh, /*onLeft*/false);
    }
    else {
      drawVText(device, p1.y, p5.y, p1.x, strl, /*onBottom*/true );
      drawVText(device, p2.y, p4.y, p2.x, lstr, /*onBottom*/false);
      drawVText(device, p2.y, p4.y, p3.x, mstr, /*onBottom*/true );
      drawVText(device, p2.y, p4.y, p4.x, ustr, /*onBottom*/false);
      drawVText(device, p1.y, p5.y, p5.x, strh, /*onBottom*/true );
    }
  }
}

CQChartsGeom::BBox
CQChartsBoxPlotDataObj::
annotationBBox() const
{
  double pos = this->pos();

  double ww = plot_->whiskerExtent();
  double bw = plot_->lengthPlotSize(plot_->boxWidth(), plot_->isHorizontal());

  double wd1 = ww/2.0;
  double wd2 = bw/2.0;

  auto posToRemapPixel = [&](double pos, double value) {
    if (! plot_->isHorizontal())
      return plot_->windowToPixel(CQChartsGeom::Point(pos, remapPos(value)));
    else
      return plot_->windowToPixel(CQChartsGeom::Point(remapPos(value), pos));
  };

  auto p1 = posToRemapPixel(pos - wd1, data_.statData.min        );
  auto p2 = posToRemapPixel(pos - wd2, data_.statData.lowerMedian);
  auto p3 = posToRemapPixel(pos      , data_.statData.median     );
  auto p4 = posToRemapPixel(pos + wd2, data_.statData.upperMedian);
  auto p5 = posToRemapPixel(pos + wd1, data_.statData.max        );

  //---

  CQChartsGeom::BBox pbbox;

  if (plot_->isTextVisible()) {
    QString strl = QString("%1").arg(data_.statData.min        );
    QString lstr = QString("%1").arg(data_.statData.lowerMedian);
    QString mstr = QString("%1").arg(data_.statData.median     );
    QString ustr = QString("%1").arg(data_.statData.upperMedian);
    QString strh = QString("%1").arg(data_.statData.max        );

    if (! plot_->isHorizontal()) {
      addHBBox(pbbox, p1.x, p5.x, p1.y, strl, /*onLeft*/false);
      addHBBox(pbbox, p2.x, p4.x, p2.y, lstr, /*onLeft*/true );
      addHBBox(pbbox, p2.x, p4.x, p3.y, mstr, /*onLeft*/false);
      addHBBox(pbbox, p2.x, p4.x, p4.y, ustr, /*onLeft*/true );
      addHBBox(pbbox, p1.x, p5.x, p5.y, strh, /*onLeft*/false);
    }
    else {
      addVBBox(pbbox, p1.y, p5.y, p1.x, strl, /*onBottom*/true );
      addVBBox(pbbox, p2.y, p4.y, p2.x, lstr, /*onBottom*/false);
      addVBBox(pbbox, p2.y, p4.y, p3.x, mstr, /*onBottom*/true );
      addVBBox(pbbox, p2.y, p4.y, p4.x, ustr, /*onBottom*/false);
      addVBBox(pbbox, p1.y, p5.y, p5.x, strh, /*onBottom*/true );
    }
  }
  else {
    pbbox += p1;
    pbbox += p2;
    pbbox += p3;
    pbbox += p4;
    pbbox += p5;
  }

  //---

  auto bbox = plot_->pixelToWindow(pbbox);

  return bbox;
}

double
CQChartsBoxPlotDataObj::
remapPos(double y) const
{
  // remap to margin -> 1.0 - margin
  if (! plot_->isNormalized())
    return y;

  double ymargin = plot_->ymargin();

  return CMathUtil::map(y, data_.dataMin, data_.dataMax, ymargin, 1.0 - ymargin);
}

//------

CQChartsBoxPlotConnectedObj::
CQChartsBoxPlotConnectedObj(const CQChartsBoxPlot *plot, const CQChartsGeom::BBox &rect,
                            int groupInd, const ColorInd &ig) :
 CQChartsPlotObj(const_cast<CQChartsBoxPlot *>(plot), rect, ColorInd(), ig, ColorInd()),
 plot_(plot), groupInd_(groupInd)
{
  initPolygon();
}

QString
CQChartsBoxPlotConnectedObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(ig_.i);
}

QString
CQChartsBoxPlotConnectedObj::
calcTipId() const
{
  QString groupName = plot_->groupIndName(groupInd_);

  const CQChartsBoxPlotConnectedObj::SetWhiskerMap &setWhiskerMap = this->setWhiskerMap();

  int ns = setWhiskerMap.size();

  CQChartsTableTip tableTip;

  tableTip.addTableRow("Group"   , groupName);
  tableTip.addTableRow("Num Sets", ns);

  //---

  //plot()->addTipColumns(tableTip, node1_->ind());

  //---

  return tableTip.str();
}

void
CQChartsBoxPlotConnectedObj::
initPolygon()
{
  CQChartsGeom::Polygon maxPoly, minPoly;

  const CQChartsBoxPlotConnectedObj::SetWhiskerMap &setWhiskerMap = this->setWhiskerMap();

  for (const auto &setWhiskers : setWhiskerMap) {
    int  setId   = setWhiskers.first;
    auto whisker = setWhiskers.second;

    double min    = whisker->min   ();
    double max    = whisker->max   ();
    double median = whisker->median();

    line_.addPoint(CQChartsGeom::Point(setId, median));

    maxPoly.addPoint(CQChartsGeom::Point(setId, max));
    minPoly.addPoint(CQChartsGeom::Point(setId, min));
  }

  //---

  int np = maxPoly.size();

  for (int i = 0; i < np; ++i)
    poly_.addPoint(maxPoly.point(i));

  for (int i = 0; i < np; ++i)
    poly_.addPoint(minPoly.point(np - 1 - i));
}

const CQChartsBoxPlotConnectedObj::SetWhiskerMap &
CQChartsBoxPlotConnectedObj::
setWhiskerMap() const
{
  static CQChartsBoxPlotConnectedObj::SetWhiskerMap dummy;

  int i = 0;

  for (const auto &groupIdWhiskers : plot_->groupWhiskers()) {
    if (i == ig_.i)
      return groupIdWhiskers.second;

    ++i;
  }

  assert(false);

  return dummy;
}

bool
CQChartsBoxPlotConnectedObj::
inside(const CQChartsGeom::Point &p) const
{
  return poly_.containsPoint(p, Qt::OddEvenFill);
}

void
CQChartsBoxPlotConnectedObj::
draw(CQChartsPaintDevice *device)
{
  // draw range polygon
  int np = poly_.size();

  if (np) {
    // set pen and brush
    CQChartsPenBrush pPenBrush;

    QColor bc = plot_->interpBoxStrokeColor(ig_);
    QColor fc = plot_->interpBoxFillColor  (ig_);

    plot_->setPenBrush(pPenBrush,
      CQChartsPenData  (plot_->isBoxStroked(), bc, plot_->boxStrokeAlpha(),
                        plot_->boxStrokeWidth(), plot_->boxStrokeDash()),
      CQChartsBrushData(plot_->isBoxFilled(), fc, plot_->boxFillAlpha(), plot_->boxFillPattern()));

    plot_->updateObjPenBrushState(this, pPenBrush);

    CQChartsDrawUtil::setPenBrush(device, pPenBrush);

    //---

    // draw poly
    QPainterPath path = CQChartsDrawUtil::polygonToPath(poly_, /*closed*/true);

    device->drawPath(path);
  }

  //---

  // set pen
  CQChartsPenBrush lPenBrush;

  QColor lineColor = plot_->interpBoxStrokeColor(ig_);

  plot_->setPen(lPenBrush.pen, true, lineColor, plot_->boxStrokeAlpha(),
                plot_->boxStrokeWidth(), plot_->boxStrokeDash());

  plot_->updateObjPenBrushState(this, lPenBrush);

  device->setPen(lPenBrush.pen);

  //---

  // draw connected line
  CQChartsGeom::Polygon line;

  for (int i = 0; i < line_.size(); ++i)
    line.addPoint(line_.point(i));

  device->drawPolyline(line);
}

//------

CQChartsBoxPlotObj::
CQChartsBoxPlotObj(const CQChartsBoxPlot *plot, const CQChartsGeom::BBox &rect,
                   const ColorInd &is, const ColorInd &ig, const ColorInd &iv) :
 CQChartsPlotObj(const_cast<CQChartsBoxPlot *>(plot), rect, is, ig, iv), plot_(plot)
{
}

void
CQChartsBoxPlotObj::
drawHText(CQChartsPaintDevice *device, double xl, double xr, double y,
          const QString &text, bool onLeft)
{
  double margin  = plot_->textMargin();
  bool   invertX = plot_->isInvertX();

  if (invertX)
    onLeft = ! onLeft;

  double x = ((onLeft && ! invertX) || (! onLeft && invertX) ? xl : xr);

  plot_->view()->setPlotPainterFont(plot_, device, plot_->textFont());

  QFontMetricsF fm(device->font());

  double yf = (fm.ascent() - fm.descent())/2.0;

  CQChartsGeom::Point tp;

  if (onLeft)
    tp = CQChartsGeom::Point(x - margin - fm.width(text), y + yf);
  else
    tp = CQChartsGeom::Point(x + margin, y + yf);

  // only support contrast
  CQChartsTextOptions options;

  options.angle         = CQChartsAngle(0);
  options.align         = Qt::AlignLeft | Qt::AlignBottom;
  options.contrast      = plot_->isTextContrast();
  options.contrastAlpha = plot_->textContrastAlpha();

  CQChartsDrawUtil::drawTextAtPoint(device, device->pixelToWindow(tp), text, options);
}

void
CQChartsBoxPlotObj::
drawVText(CQChartsPaintDevice *device, double yb, double yt, double x,
          const QString &text, bool onBottom)
{
  double margin  = plot_->textMargin();
  bool   invertY = plot_->isInvertY();

  if (invertY)
    onBottom = ! onBottom;

  double y = ((onBottom && ! invertY) || (! onBottom && invertY) ? yb : yt);

  plot_->view()->setPlotPainterFont(plot_, device, plot_->textFont());

  QFontMetricsF fm(device->font());

  double xf = fm.width(text)/2.0;
  double fa = fm.ascent ();
  double fd = fm.descent();

  CQChartsGeom::Point tp;

  if (onBottom)
    tp = CQChartsGeom::Point(x - xf, y + margin + fa);
  else
    tp = CQChartsGeom::Point(x - xf, y - margin - fd);

  // only support contrast
  CQChartsTextOptions options;

  options.angle         = CQChartsAngle(0);
  options.align         = Qt::AlignLeft | Qt::AlignBottom;
  options.contrast      = plot_->isTextContrast();
  options.contrastAlpha = plot_->textContrastAlpha();

  CQChartsDrawUtil::drawTextAtPoint(device, device->pixelToWindow(tp), text, options);
}

void
CQChartsBoxPlotObj::
addHBBox(CQChartsGeom::BBox &pbbox, double xl, double xr, double y,
         const QString &text, bool onLeft) const
{
  double margin  = plot_->textMargin();
  bool   invertX = plot_->isInvertX();

  if (invertX)
    onLeft = ! onLeft;

  double x = ((onLeft && ! invertX) || (! onLeft && invertX) ? xl : xr);

  QFont font = plot_->view()->plotFont(plot_, plot_->textFont());

  QFontMetricsF fm(font);

  double fa = fm.ascent ();
  double fd = fm.descent();
  double yf = (fa - fd)/2.0;

  double tx;

  if (onLeft)
    tx = x - margin - fm.width(text);
  else
    tx = x + margin + fm.width(text);

  pbbox += CQChartsGeom::Point(tx, y + yf - fa);
  pbbox += CQChartsGeom::Point(tx, y + yf + fd);
}

void
CQChartsBoxPlotObj::
addVBBox(CQChartsGeom::BBox &pbbox, double yb, double yt, double x,
         const QString &text, bool onBottom) const
{
  double margin  = plot_->textMargin();
  bool   invertY = plot_->isInvertY();

  if (invertY)
    onBottom = ! onBottom;

  double y = ((onBottom && ! invertY) || (! onBottom && invertY) ? yb : yt);

  QFont font = plot_->view()->plotFont(plot_, plot_->textFont());

  QFontMetricsF fm(font);

  double xf = fm.width(text)/2.0;
  double fa = fm.ascent ();
  double fd = fm.descent();

  double ty;

  if (onBottom)
    ty = y + margin + fa;
  else
    ty = y - margin - fd;

  pbbox += CQChartsGeom::Point(x - xf, ty);
  pbbox += CQChartsGeom::Point(x + xf, ty);
}

//------

CQChartsBoxPlotPointObj::
CQChartsBoxPlotPointObj(const CQChartsBoxPlot *plot, const CQChartsGeom::BBox &rect,
                        int setId, int groupInd, const CQChartsGeom::Point &p,
                        const QModelIndex &ind, const ColorInd &is, const ColorInd &ig,
                        const ColorInd &iv) :
 CQChartsPlotObj(const_cast<CQChartsBoxPlot *>(plot), rect, is, ig, iv), plot_(plot),
 setId_(setId), groupInd_(groupInd), p_(p)
{
  setModelInd(ind);
}

QString
CQChartsBoxPlotPointObj::
calcId() const
{
  return QString("%1:%2:%3:%4").arg(typeName()).arg(is_.i).arg(ig_.i).arg(iv_.i);
}

QString
CQChartsBoxPlotPointObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  QString setName   = plot_->setIdName   (setId_);
  QString groupName = plot_->groupIndName(groupInd_);

  tableTip.addTableRow("Set"  , setName);
  tableTip.addTableRow("Group", groupName);
  tableTip.addTableRow("Ind"  , iv_.i);

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

bool
CQChartsBoxPlotPointObj::
inside(const CQChartsGeom::Point &p) const
{
  auto p1 = plot_->windowToPixel(CQChartsGeom::Point(p_.x, p_.y));

  CQChartsGeom::BBox pbbox(p1.x - 4, p1.y - 4, p1.x + 4, p1.y + 4);

  auto pp = plot_->windowToPixel(p);

  return pbbox.inside(pp);
}

void
CQChartsBoxPlotPointObj::
getSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, CQChartsColumn(modelInd().column()));
}

void
CQChartsBoxPlotPointObj::
draw(CQChartsPaintDevice *device)
{
  CQChartsSymbol symbolType = plot_->jitterSymbolType();
  CQChartsLength symbolSize = plot_->jitterSymbolSize();

  //double sx, sy;
  //plot_->pixelSymbolSize(symbolSize, sx, sy);

  //---

  // get color index
  ColorInd colorInd = this->calcColorInd();

  if (plot_->hasSets() && plot_->isColorBySet())
    colorInd = is_;

  //---

  // calc stroke and brush
  CQChartsPenBrush penBrush;

  plot_->setJitterSymbolPenBrush(penBrush, colorInd);

  plot_->updateObjPenBrushState(this, penBrush, CQChartsPlot::DrawType::SYMBOL);

  //---

  // draw symbol
  auto pos = device->pixelToWindow(p_);

  plot_->drawSymbol(device, pos, symbolType, symbolSize, penBrush);
}

//------

CQChartsBoxKeyColor::
CQChartsBoxKeyColor(CQChartsBoxPlot *plot, const ColorInd &is, const ColorInd &ig) :
 CQChartsKeyColorBox(plot, is, ig, ColorInd())
{
}

bool
CQChartsBoxKeyColor::
selectPress(const CQChartsGeom::Point &, CQChartsSelMod)
{
  auto *plot = qobject_cast<CQChartsBoxPlot *>(plot_);

  ColorInd ic = (is_.n > 1 ? is_ : ig_);

  plot->setSetHidden(ic.i, ! plot->isSetHidden(ic.i));

  plot->updateRangeAndObjs();

  return true;
}

QBrush
CQChartsBoxKeyColor::
fillBrush() const
{
  QColor c = CQChartsKeyColorBox::fillBrush().color();

  auto *plot = qobject_cast<CQChartsBoxPlot *>(plot_);

  ColorInd ic = (is_.n > 1 ? is_ : ig_);

  if (plot->isSetHidden(ic.i))
    c = CQChartsUtil::blendColors(c, key_->interpBgColor(), key_->hiddenAlpha());

  return c;
}

double
CQChartsBoxKeyColor::
xColorValue(bool relative) const
{
  auto *boxObj = this->boxObj();

  return (boxObj ? boxObj->xColorValue(relative) : 0.0);
}

double
CQChartsBoxKeyColor::
yColorValue(bool relative) const
{
  auto *boxObj = this->boxObj();

  return (boxObj ? boxObj->yColorValue(relative) : 0.0);
}

CQChartsBoxPlotWhiskerObj *
CQChartsBoxKeyColor::
boxObj() const
{
  for (const auto &plotObj : plot_->plotObjects()) {
    auto *boxObj = dynamic_cast<CQChartsBoxPlotWhiskerObj *>(plotObj);
    if (! boxObj) continue;

    if (boxObj->is() == is_ && boxObj->ig() == ig_)
      return boxObj;
  }

  return nullptr;
}

//------

CQChartsBoxKeyText::
CQChartsBoxKeyText(CQChartsBoxPlot *plot, const QString &text,
                   const ColorInd &is, const ColorInd &ig) :
 CQChartsKeyText(plot, text, (is.n > 1 ? is : ig))
{
}

QColor
CQChartsBoxKeyText::
interpTextColor(const ColorInd &ind) const
{
  auto *plot = qobject_cast<CQChartsBoxPlot *>(plot_);

  QColor c = CQChartsKeyText::interpTextColor(ind);

  if (plot->isSetHidden(ic_.i))
    c = CQChartsUtil::blendColors(c, key_->interpBgColor(), key_->hiddenAlpha());

  return c;
}
