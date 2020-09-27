#include <CQCharts.h>
#include <CQChartsView.h>

// plot types
#include <CQChartsAdjacencyPlot.h>
#include <CQChartsBarChartPlot.h>
#include <CQChartsBoxPlot.h>
#include <CQChartsBubblePlot.h>
#include <CQChartsChordPlot.h>
#include <CQChartsCompositePlot.h>
#include <CQChartsContourPlot.h>
#include <CQChartsCorrelationPlot.h>
#include <CQChartsDelaunayPlot.h>
#include <CQChartsDendrogramPlot.h>
#include <CQChartsDistributionPlot.h>
#include <CQChartsEmptyPlot.h>
#include <CQChartsForceDirectedPlot.h>
#include <CQChartsGeometryPlot.h>
#include <CQChartsGraphPlot.h>
#include <CQChartsHierBubblePlot.h>
#include <CQChartsHierScatterPlot.h>
#include <CQChartsImagePlot.h>
#include <CQChartsParallelPlot.h>
#include <CQChartsPiePlot.h>
#include <CQChartsPivotPlot.h>
#include <CQChartsRadarPlot.h>
#include <CQChartsSankeyPlot.h>
#include <CQChartsScatterPlot.h>
#include <CQChartsScatterPlot3D.h>
#include <CQChartsSunburstPlot.h>
#include <CQChartsTablePlot.h>
#include <CQChartsTreeMapPlot.h>
#include <CQChartsWordCloudPlot.h>
#include <CQChartsXYPlot.h>

#include <CQChartsModelData.h>
#include <CQChartsColumnType.h>
#include <CQChartsFileType.h>
#include <CQChartsImage.h>
#include <CQChartsWidget.h>
#include <CQChartsModelIndex.h>

#include <CQChartsAlphaEdit.h>
#include <CQChartsAngleEdit.h>
#include <CQChartsArrowDataEdit.h>
#include <CQChartsAxisSideEdit.h>
#include <CQChartsAxisTickLabelPlacementEdit.h>
#include <CQChartsAxisValueTypeEdit.h>
#include <CQChartsBoxDataEdit.h>
#include <CQChartsColorEdit.h>
#include <CQChartsColumnEdit.h>
#include <CQChartsColumnsEdit.h>
#include <CQChartsColumnNumEdit.h>
#include <CQChartsFillDataEdit.h>
#include <CQChartsFillPatternEdit.h>
#include <CQChartsFillUnderEdit.h>
#include <CQChartsFontEdit.h>
#include <CQChartsGeomBBoxEdit.h>
#include <CQChartsGeomPointEdit.h>
#include <CQChartsKeyLocationEdit.h>
#include <CQChartsKeyPressBehaviorEdit.h>
#include <CQChartsLengthEdit.h>
#include <CQChartsLineDataEdit.h>
#include <CQChartsLineDashEdit.h>
#include <CQChartsMarginEdit.h>
#include <CQChartsPaletteNameEdit.h>
#include <CQChartsPolygonEdit.h>
#include <CQChartsPositionEdit.h>
#include <CQChartsRectEdit.h>
#include <CQChartsShapeDataEdit.h>
#include <CQChartsSidesEdit.h>
#include <CQChartsStrokeDataEdit.h>
#include <CQChartsSymbolDataEdit.h>
#include <CQChartsSymbolEdit.h>
#include <CQChartsTextBoxDataEdit.h>
#include <CQChartsTextDataEdit.h>
#include <CQChartsTitleLocationEdit.h>

#include <CQChartsOptIntEdit.h>
#include <CQChartsOptRealEdit.h>
#include <CQChartsOptStringEdit.h>
#include <CQChartsOptLengthEdit.h>
#include <CQChartsOptPositionEdit.h>
#include <CQChartsOptRectEdit.h>

#include <CQChartsPolygonList.h>
#include <CQChartsNamePair.h>
#include <CQChartsSides.h>
#include <CQChartsFillUnder.h>
#include <CQChartsWindow.h>
#include <CQChartsPath.h>
#include <CQChartsVariant.h>
#include <CQChartsPoints.h>
#include <CQChartsReals.h>
#include <CQChartsObjRef.h>

#include <CQChartsOptInt.h>
#include <CQChartsOptReal.h>
#include <CQChartsOptString.h>
#include <CQChartsOptLength.h>
#include <CQChartsOptPosition.h>
#include <CQChartsOptRect.h>

#include <CQChartsFont.h>
#include <CQChartsInterfaceTheme.h>
#include <CQChartsColorStops.h>
#include <CQChartsPaletteName.h>
#include <CQChartsHtml.h>

#include <CQChartsLoadModelDlg.h>
#include <CQChartsManageModelsDlg.h>
#include <CQChartsEditModelDlg.h>
#include <CQChartsCreatePlotDlg.h>

#include <CQChartsPlotControlWidgets.h>
#include <CQChartsModelViewHolder.h>
#include <CQChartsModelDetailsTable.h>
#include <CQChartsPlotPropertyEdit.h>
#include <CQChartsModelColumnDataControl.h>
#include <CQChartsModelExprControl.h>

#include <CQColorsPalette.h>

#include <CQPropertyView.h>
#include <CQPropertyViewItem.h>
#include <CQColors.h>
#include <CQColorsTheme.h>

#include <CQWidgetFactory.h>
#include <CQTclUtil.h>

#include <iostream>

QString
CQCharts::
description()
{
  auto LI  = [](const QString &str) { return CQChartsHtml::Str(str); };
//auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };
  auto IMG = [](const QString &) { return QString(); };

  return CQChartsHtml().
   h2("Introduction").
    p("CQCharts is a Qt based charting library to support the display of an interactive "
      "chart from a data model (QAbstractItemModel) with support for cross selection using "
      "the data model's selection model (QItemSelectionModel).").
    p("Charts are live in that changes to the data model automatically update "
      "the chart. This allows the programmer to build interactive charts where "
      "the user can select items from the model using a table or tree view and/or "
      "the displayed chart data.").
    p("The library comes with a test program which supports scripting of charts using the "
      "'tcl' programming language. A number of example plot scripts are available in the "
      "data directory.").
   h3("Chart Types").
    p("Chart types supported:").
    ul({
     LI("Adjacency : tabular connectivity table (needs special column value syntax "
        "for connections) " + IMG("images/adjacency.png")),
     LI("BarChart : 1d bar chart for column values " + IMG("images/barchart.png")),
     LI("Box : Box plot of aggregated values from value and group columns " +
        IMG("images/boxplot.png") + IMG("images/boxplot_connected.png")),
     LI("Bubble : Bubble plot where circle is size of column value" +
        IMG("images/bubbleplot.png")),
     LI("Chord : Circlular connectivity chart (needs special column value syntax for "
        "connections) " + IMG("images/chord_plot.png")),
     LI("Delaunay : Delaunay/Voronoi geometric connectivity chart" +
        IMG("images/delaunay.png")),
     LI("Distribution : Distribution count of range based column values" +
        IMG("images/distribution.png")),
     LI("ForceDirected : Force directed connectivity chart" +
        IMG("images/forcedirected.png")),
     LI("Geometry : General polygon geometry colored by value" +
        IMG("images/geometryplot.png")),
     LI("Graph : Graph plot of connected values" +
        IMG("images/graph.png")),
     LI("HierBubble : Hierarchical bubble plot where circle is size of column value" +
        IMG("images/hierbubble.png")),
     LI("Image : Image/regular grid plot (x, y, color)" +
        IMG("images/imageplot.png")),
     LI("Parallel : Parallel lines plot using multiple connected value sets" +
        IMG("images/parallelplot.png")),
     LI("Pie : Pie chart" +
        IMG("images/piechart.png")),
     LI("Radar : Radar (polygon pie chart)" +
        IMG("images/radar.png")),
     LI("Sankey : Sankey plot of connected values" +
        IMG("images/sankey.png")),
     LI("Scatter : Scatter plot of disparate x/y column values" +
        IMG("images/scatterplot.png")),
     LI("Sunburst : Hierarchical pie chart plot" +
        IMG("images/sunburst.png")),
     LI("TreeMap : Hierarchical tree map" +
        IMG("images/treemap.png")),
     LI("XY : x/y monotonic value plot" +
        IMG("images/xychart.png") + IMG("images/goal_scorers.png")) }).
   h3("Adjacency").
    p("Uses table cell coloring to show number of connections between two items.").
    p("Items names are displayed in the row and column headers and the number of connections "
      "is used to color the cell (row/column intersection).").
    p("Cell colors are derived from the blend of the colors for each item's group "
      "and the color intensity is calculated from the relative value.").
    p("The following values can be customized:").
     ul({
      LI("table background color"),
      LI("empty cell color"),
      LI("margin"),
      LI("cell border color, alpha and corner size"),
      LI("header text color and font")}).
    p("Input data model can contain either node data (name, id, group) and a list of "
      "connections (id, count) [old format] or a '/' separated connection data "
      "(from name/to name, value, count).").
   h3("BarChart").
    p("1d bar chart for column value or values.").
    p("Bars can be grouped using a group/category column.").
    p("Custom bar colors and data labels can be provided in additional columns.").
    p("Bars can be stacked next to each other or on top of each other and can be "
      "drawn vertically (default) or horizontally.").
   h3("Box").
    p("Box plot of aggregated values from value and group columns.").
    p("Input data is a set of y values for an associated common x value.").
    p("Data can also be additional grouped by an extra column.").
    p("Values can be displayed as a candlestick bar or connected into a single "
      "solid bar of the value range.").
   h3("Bubble").
    p("Bubble plot where circle is size of column value.").
    p("Circles are packed in minimum enclosing circle.").
   h3("Chord").
    p("Circlular connectivity chart.").
    p("Input data model can contain either node data (name, id, group) and a list of "
      "connections (id, count) (old format) or '/' separated connection data "
      "(from name/to name, value, count).").
   h3("Delaunay").
    p("Delaunay/Voronoi geometric connectivity chart").
   h3("Distribution").
    p("Distribution count of range based values").
   h3("ForceDirected").
    p("Force directed connectivity chart").
   h3("Geometry").
    p("General polygon geometry colored by value").
   h3("Graph").
    p("Graph plot of connected values").
   h3("HierBubble").
    p("Hierarchical bubble plot where circle is size of column value.").
    p("Hierarchical circles are packed in minimum enclosing circle.").
   h3("Image").
    p("Image plot (x, y, color)").
   h3("Parallel").
    p("Parallel lines plots multiple overlaid value sets").
   h3("Pie").
    p("Pie chart").
   h3("Radar").
    p("Radar (polygon pie chart)").
   h3("Sankey").
    p("Sankey plot of connected values").
   h3("Scatter").
    p("Scatter plot of disparate values").
   h3("Sunburst").
    p("Hierarchical pie chart plot").
   h3("TreeMap").
    p("Hierarchical tree map").
   h3("XY").
    p("Plot of x,y values. x values should be monotonic.").
    p("Multiple y columns can be supplied to produce multi line plot.").
    p("Binariate plot using two y columns.").
    p("Display of connecing lines and individual points can be customized.").
  h2("Parameter and Properties").
   p("Each plot type has a set of parameters which allow the major plot controls "
     "to be configured.").
   p("The plot also support properties for fine tuning the plot display.").
   p("The parameter and properties use Qt variants (QVariant)  and are displayed in a "
     "property view tree which can be displayed at the side of the plot.").
  h2(" Data Model").
   p("The data model can be viewed in a table or tree view in the plot and can be "
     "sorted and filtered to update the plot.");
}

//---

CQCharts::
CQCharts()
{
  // register variant meta types
  CQChartsGeom::registerMetaTypes();

  CQChartsAlpha                 ::registerMetaType();
  CQChartsAngle                 ::registerMetaType();
  CQChartsArrowData             ::registerMetaType();
  CQChartsAxisSide              ::registerMetaType();
  CQChartsAxisTickLabelPlacement::registerMetaType();
  CQChartsAxisValueType         ::registerMetaType();
  CQChartsBoxData               ::registerMetaType();
  CQChartsColor                 ::registerMetaType();
  CQChartsColorStops            ::registerMetaType();
  CQChartsColumnNum             ::registerMetaType();
  CQChartsColumn                ::registerMetaType();
  CQChartsColumns               ::registerMetaType();
  CQChartsConnectionList        ::registerMetaType();
  CQChartsFillData              ::registerMetaType();
  CQChartsFillPattern           ::registerMetaType();
  CQChartsFillUnderPos          ::registerMetaType();
  CQChartsFillUnderSide         ::registerMetaType();
  CQChartsFont                  ::registerMetaType();
  CQChartsImage                 ::registerMetaType();
  CQChartsKeyLocation           ::registerMetaType();
  CQChartsKeyPressBehavior      ::registerMetaType();
  CQChartsLength                ::registerMetaType();
  CQChartsLineDash              ::registerMetaType();
  CQChartsLineData              ::registerMetaType();
  CQChartsMargin                ::registerMetaType();
  CQChartsModelIndex            ::registerMetaType();
  CQChartsNamePair              ::registerMetaType();
  CQChartsObjRef                ::registerMetaType();
  CQChartsPaletteName           ::registerMetaType();
  CQChartsPath                  ::registerMetaType();
  CQChartsPoints                ::registerMetaType();
  CQChartsPolygonList           ::registerMetaType();
  CQChartsPolygon               ::registerMetaType();
  CQChartsPosition              ::registerMetaType();
  CQChartsReals                 ::registerMetaType();
  CQChartsRect                  ::registerMetaType();
  CQChartsShapeData             ::registerMetaType();
  CQChartsSides                 ::registerMetaType();
  CQChartsStrokeData            ::registerMetaType();
  CQChartsStyle                 ::registerMetaType();
  CQChartsSymbolData            ::registerMetaType();
  CQChartsSymbol                ::registerMetaType();
  CQChartsTextBoxData           ::registerMetaType();
  CQChartsTextData              ::registerMetaType();
  CQChartsThemeName             ::registerMetaType();
  CQChartsTitleLocation         ::registerMetaType();
  CQChartsWidget                ::registerMetaType();

  CQChartsOptInt     ::registerMetaType();
  CQChartsOptLength  ::registerMetaType();
  CQChartsOptPosition::registerMetaType();
  CQChartsOptReal    ::registerMetaType();
  CQChartsOptRect    ::registerMetaType();
  CQChartsOptString  ::registerMetaType();

  //---

  // init theme
  plotTheme_.setName("default");

  //---

  interfaceTheme_ = new CQChartsInterfaceTheme;

  interfaceTheme()->setDark(false);

  //---

  addProc(ProcType::SVG, "logMessage", "s",
    "document.getElementById(\"log_message\").innerHTML = s;");

  addProc(ProcType::SVG, "plotObjClick", "id", "logMessage('Click ' + id);");
  addProc(ProcType::SVG, "annotationClick", "id", "logMessage('Click ' + id);");

  //---

  addProc(ProcType::SCRIPT, "logMessage", "s",
    "document.getElementById(\"log_message\").innerHTML = s;");

  addProc(ProcType::SCRIPT, "plotObjClick", "id", "charts.log('Click ' + id);");
  addProc(ProcType::SCRIPT, "annotationClick", "id", "charts.log('Click ' + id);");

  //---

  cmdTcl_ = new CQTcl();
}

CQCharts::
~CQCharts()
{
  delete interfaceTheme_;

  for (auto &modelData : modelDatas_)
    delete modelData;

  delete plotTypeMgr_;
  delete columnTypeMgr_;
}

void
CQCharts::
init()
{
  plotTypeMgr_   = new CQChartsPlotTypeMgr;
  columnTypeMgr_ = new CQChartsColumnTypeMgr(this);

  //---

  // add plot types
  plotTypeMgr_->addType("adjacency"    , new CQChartsAdjacencyPlotType    );
  plotTypeMgr_->addType("barchart"     , new CQChartsBarChartPlotType     );
  plotTypeMgr_->addType("box"          , new CQChartsBoxPlotType          );
  plotTypeMgr_->addType("bubble"       , new CQChartsBubblePlotType       );
  plotTypeMgr_->addType("chord"        , new CQChartsChordPlotType        );
  plotTypeMgr_->addType("composite"    , new CQChartsCompositePlotType    );
  plotTypeMgr_->addType("contour"      , new CQChartsContourPlotType      );
  plotTypeMgr_->addType("correlation"  , new CQChartsCorrelationPlotType  );
  plotTypeMgr_->addType("delaunay"     , new CQChartsDelaunayPlotType     );
  plotTypeMgr_->addType("dendrogram"   , new CQChartsDendrogramPlotType   );
  plotTypeMgr_->addType("distribution" , new CQChartsDistributionPlotType );
  plotTypeMgr_->addType("empty"        , new CQChartsEmptyPlotType        );
  plotTypeMgr_->addType("forcedirected", new CQChartsForceDirectedPlotType);
  plotTypeMgr_->addType("geometry"     , new CQChartsGeometryPlotType     );
  plotTypeMgr_->addType("graph"        , new CQChartsGraphPlotType        );
  plotTypeMgr_->addType("hierbubble"   , new CQChartsHierBubblePlotType   );
  plotTypeMgr_->addType("hierscatter"  , new CQChartsHierScatterPlotType  );
  plotTypeMgr_->addType("image"        , new CQChartsImagePlotType        );
  plotTypeMgr_->addType("parallel"     , new CQChartsParallelPlotType     );
  plotTypeMgr_->addType("pie"          , new CQChartsPiePlotType          );
  plotTypeMgr_->addType("pivot"        , new CQChartsPivotPlotType        );
  plotTypeMgr_->addType("radar"        , new CQChartsRadarPlotType        );
  plotTypeMgr_->addType("sankey"       , new CQChartsSankeyPlotType       );
  plotTypeMgr_->addType("scatter"      , new CQChartsScatterPlotType      );
  plotTypeMgr_->addType("scatter3d"    , new CQChartsScatterPlot3DType    );
  plotTypeMgr_->addType("sunburst"     , new CQChartsSunburstPlotType     );
  plotTypeMgr_->addType("table"        , new CQChartsTablePlotType        );
  plotTypeMgr_->addType("treemap"      , new CQChartsTreeMapPlotType      );
  plotTypeMgr_->addType("wordCloud"    , new CQChartsWordCloudPlotType    );
  plotTypeMgr_->addType("xy"           , new CQChartsXYPlotType           );

  //---

  // add column types
  columnTypeMgr_->addType(CQBaseModelType::STRING         , new CQChartsColumnStringType        );
  columnTypeMgr_->addType(CQBaseModelType::INTEGER        , new CQChartsColumnIntegerType       );
  columnTypeMgr_->addType(CQBaseModelType::REAL           , new CQChartsColumnRealType          );
  columnTypeMgr_->addType(CQBaseModelType::BOOLEAN        , new CQChartsColumnBooleanType       );
  columnTypeMgr_->addType(CQBaseModelType::TIME           , new CQChartsColumnTimeType          );
  columnTypeMgr_->addType(CQBaseModelType::COLOR          , new CQChartsColumnColorType         );
  columnTypeMgr_->addType(CQBaseModelType::FONT           , new CQChartsColumnFontType          );
  columnTypeMgr_->addType(CQBaseModelType::RECT           , new CQChartsColumnRectType          );
  columnTypeMgr_->addType(CQBaseModelType::LENGTH         , new CQChartsColumnLengthType        );
  columnTypeMgr_->addType(CQBaseModelType::POLYGON        , new CQChartsColumnPolygonType       );
  columnTypeMgr_->addType(CQBaseModelType::POLYGON_LIST   , new CQChartsColumnPolygonListType   );
  columnTypeMgr_->addType(CQBaseModelType::IMAGE          , new CQChartsColumnImageType         );
  columnTypeMgr_->addType(CQBaseModelType::SYMBOL         , new CQChartsColumnSymbolTypeType    );
  columnTypeMgr_->addType(CQBaseModelType::SYMBOL_SIZE    , new CQChartsColumnSymbolSizeType    );
  columnTypeMgr_->addType(CQBaseModelType::FONT_SIZE      , new CQChartsColumnFontSizeType      );
  columnTypeMgr_->addType(CQBaseModelType::PATH           , new CQChartsColumnPathType          );
  columnTypeMgr_->addType(CQBaseModelType::STYLE          , new CQChartsColumnStyleType         );
  columnTypeMgr_->addType(CQBaseModelType::NAME_PAIR      , new CQChartsColumnNamePairType      );
  columnTypeMgr_->addType(CQBaseModelType::CONNECTION_LIST, new CQChartsColumnConnectionListType);

  //---

  static bool typesInitialized = false;

  if (! typesInitialized) {
    typesInitialized = true;

    //---

    // add property types (and editors)
    auto *viewMgr = CQPropertyViewMgrInst;

    viewMgr->addType("CQChartsAlpha"           , new CQChartsAlphaPropertyViewType           );
    viewMgr->addType("CQChartsAngle"           , new CQChartsAnglePropertyViewType           );
    viewMgr->addType("CQChartsArrowData"       , new CQChartsArrowDataPropertyViewType       );
    viewMgr->addType("CQChartsAxisSide"        , new CQChartsAxisSidePropertyViewType        );
    viewMgr->addType("CQChartsAxisTickLabelPlacement",
                     new CQChartsAxisTickLabelPlacementPropertyViewType);
    viewMgr->addType("CQChartsAxisValueType"   , new CQChartsAxisValueTypePropertyViewType   );
    viewMgr->addType("CQChartsBoxData"         , new CQChartsBoxDataPropertyViewType         );
    viewMgr->addType("CQChartsColor"           , new CQChartsColorPropertyViewType           );
    viewMgr->addType("CQChartsColumn"          , new CQChartsColumnPropertyViewType          );
    viewMgr->addType("CQChartsColumns"         , new CQChartsColumnsPropertyViewType         );
    viewMgr->addType("CQChartsColumnNum"       , new CQChartsColumnNumPropertyViewType       );
    viewMgr->addType("CQChartsFillData"        , new CQChartsFillDataPropertyViewType        );
    viewMgr->addType("CQChartsFillPattern"     , new CQChartsFillPatternPropertyViewType     );
    viewMgr->addType("CQChartsFillUnderPos"    , new CQChartsFillUnderPosPropertyViewType    );
    viewMgr->addType("CQChartsFillUnderSide"   , new CQChartsFillUnderSidePropertyViewType   );
    viewMgr->addType("CQChartsFont"            , new CQChartsFontPropertyViewType            );
    viewMgr->addType("CQChartsGeom::BBox"      , new CQChartsGeomBBoxPropertyViewType        );
    viewMgr->addType("CQChartsGeom::Point"     , new CQChartsGeomPointPropertyViewType       );
    viewMgr->addType("CQChartsKeyLocation"     , new CQChartsKeyLocationPropertyViewType     );
    viewMgr->addType("CQChartsKeyPressBehavior", new CQChartsKeyPressBehaviorPropertyViewType);
    viewMgr->addType("CQChartsTitleLocation"   , new CQChartsTitleLocationPropertyViewType   );
    viewMgr->addType("CQChartsLength"          , new CQChartsLengthPropertyViewType          );
    viewMgr->addType("CQChartsLineDash"        , new CQChartsLineDashPropertyViewType        );
    viewMgr->addType("CQChartsLineData"        , new CQChartsLineDataPropertyViewType        );
    viewMgr->addType("CQChartsMargin"          , new CQChartsMarginPropertyViewType          );
//  viewMgr->addType("CQChartsObjRef"          , new CQChartsObjRefPropertyViewType          );
    viewMgr->addType("CQChartsPaletteName"     , new CQChartsPaletteNamePropertyViewType     );
//  viewMgr->addType("CQChartsPoints"          , new CQChartsPointsPropertyViewType          );
    viewMgr->addType("CQChartsPolygon"         , new CQChartsPolygonPropertyViewType         );
    viewMgr->addType("CQChartsPosition"        , new CQChartsPositionPropertyViewType        );
//  viewMgr->addType("CQChartsReals"           , new CQChartsRealsPropertyViewType           );
    viewMgr->addType("CQChartsRect"            , new CQChartsRectPropertyViewType            );
    viewMgr->addType("CQChartsShapeData"       , new CQChartsShapeDataPropertyViewType       );
    viewMgr->addType("CQChartsSides"           , new CQChartsSidesPropertyViewType           );
    viewMgr->addType("CQChartsStrokeData"      , new CQChartsStrokeDataPropertyViewType      );
    viewMgr->addType("CQChartsSymbolData"      , new CQChartsSymbolDataPropertyViewType      );
    viewMgr->addType("CQChartsSymbol"          , new CQChartsSymbolPropertyViewType          );
    viewMgr->addType("CQChartsTextBoxData"     , new CQChartsTextBoxDataPropertyViewType     );
    viewMgr->addType("CQChartsTextData"        , new CQChartsTextDataPropertyViewType        );

    viewMgr->addType("CQChartsOptInt"     , new CQChartsOptIntPropertyViewType     );
    viewMgr->addType("CQChartsOptReal"    , new CQChartsOptRealPropertyViewType    );
    viewMgr->addType("CQChartsOptString"  , new CQChartsOptStringPropertyViewType  );
    viewMgr->addType("CQChartsOptLength"  , new CQChartsOptLengthPropertyViewType  );
    viewMgr->addType("CQChartsOptPosition", new CQChartsOptPositionPropertyViewType);
    viewMgr->addType("CQChartsOptRect"    , new CQChartsOptRectPropertyViewType    );

    //---

    // add widget factories

    // controls
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotControlFrame",
      new CQWidgetFactoryT<CQChartsPlotControlFrame>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotRealControl",
      new CQWidgetFactoryT<CQChartsPlotRealControl>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotIntControl",
      new CQWidgetFactoryT<CQChartsPlotIntControl>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotTimeControl",
      new CQWidgetFactoryT<CQChartsPlotTimeControl>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotValueControl",
      new CQWidgetFactoryT<CQChartsPlotValueControl>());

    // model view
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsModelViewHolder",
      new CQWidgetFactoryNoArgsT<CQChartsModelViewHolder>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsModelDetailsTable",
      new CQWidgetFactoryNoArgsT<CQChartsModelDetailsTable>());

    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsModelColumnDataControl",
      new CQWidgetFactoryNoArgsT<CQChartsModelColumnDataControl>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsModelExprControl",
      new CQWidgetFactoryNoArgsT<CQChartsModelExprControl>());
#if 0
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsModelFoldControl",
      new CQWidgetFactoryNoArgsT<CQChartsModelFoldControl>());
#endif

    // other edits (all ?)
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsAlphaEdit",
      new CQWidgetFactoryT<CQChartsAlphaEdit>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsAngleEdit",
      new CQWidgetFactoryT<CQChartsAngleEdit>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsColorEdit",
      new CQWidgetFactoryT<CQChartsColorEdit>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsFontEdit",
      new CQWidgetFactoryT<CQChartsFontEdit>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsLengthEdit",
      new CQWidgetFactoryT<CQChartsLengthEdit>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPositionEdit",
      new CQWidgetFactoryT<CQChartsPositionEdit>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsSymbolEdit",
      new CQWidgetFactoryT<CQChartsSymbolEdit>());

    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotPropertyEditGroup",
      new CQWidgetFactoryNoArgsT<CQChartsPlotPropertyEditGroup>());
    CQWidgetFactoryMgrInst->addWidgetFactory("CQChartsPlotPropertyEdit",
      new CQWidgetFactoryNoArgsT<CQChartsPlotPropertyEdit>());
  }
}

//---

void
CQCharts::
getModelTypeNames(QStringList &names) const
{
  names << CQChartsFileTypeUtil::fileTypeNames();
}

//---

bool
CQCharts::
isPlotType(const QString &name) const
{
  return plotTypeMgr_->isType(name);
}

CQChartsPlotType *
CQCharts::
plotType(const QString &name) const
{
  return plotTypeMgr_->type(name);
}

void
CQCharts::
getPlotTypes(PlotTypes &types) const
{
  plotTypeMgr_->getTypes(types);
}

void
CQCharts::
getPlotTypeNames(QStringList &names, QStringList &descs) const
{
  plotTypeMgr_->getTypeNames(names, descs);
}

//---

QColor
CQCharts::
interpColor(const CQChartsColor &c, const ColorInd &ind) const
{
  return interpColorValueI(c, /*ig*/0, /*ng*/1, ind.value(), ind.c);
}

QColor
CQCharts::
interpColorValueI(const CQChartsColor &c, int ig, int ng, double value, const QColor &ic) const
{
  if (! c.isValid())
    return QColor();

  if      (c.type() == CQChartsColor::Type::COLOR)
    return c.color();
  else if (c.type() == CQChartsColor::Type::PALETTE ||
           c.type() == CQChartsColor::Type::PALETTE_VALUE) {
    if      (c.hasPaletteIndex()) {
      if (c.type() == CQChartsColor::Type::PALETTE_VALUE)
        return interpIndPaletteColor(c.ind(), c.value(), c.isScale());
      else
        return interpIndPaletteColorValue(c.ind(), ig, ng, value, c.isScale());
    }
    else if (c.hasPaletteName()) {
      QString name;

      if (c.getPaletteName(name)) {
        if (c.type() == CQChartsColor::Type::PALETTE_VALUE)
          return interpNamePaletteColor(name, c.value(), c.isScale());
        else
          return interpNamePaletteColorValue(name, ig, ng, value, c.isScale());
      }
      else {
        if (c.type() == CQChartsColor::Type::PALETTE_VALUE)
          return interpPaletteColor(ColorInd(c.value()), c.isScale());
        else
          return interpPaletteColorValue(ig, ng, value, c.isScale());
      }
    }
    else {
      if (c.type() == CQChartsColor::Type::PALETTE_VALUE)
        return interpPaletteColor(ColorInd(c.value()), c.isScale());
      else
        return interpPaletteColorValue(ig, ng, value, c.isScale());
    }
  }
  else if (c.type() == CQChartsColor::Type::INDEXED ||
           c.type() == CQChartsColor::Type::INDEXED_VALUE) {
    if      (c.hasPaletteIndex()) {
      if (c.type() == CQChartsColor::Type::INDEXED_VALUE)
        return indexIndPaletteColor(c.ind(), int(c.value()), ng);
      else
        return indexIndPaletteColor(c.ind(), ig, ng);
    }
    else if (c.hasPaletteName()) {
      QString name;

      if (c.getPaletteName(name)) {
        if (c.type() == CQChartsColor::Type::INDEXED_VALUE)
          return indexNamePaletteColor(name, int(c.value()), ng);
        else
          return indexNamePaletteColor(name, ig, ng);
      }
      else {
        if (c.type() == CQChartsColor::Type::INDEXED_VALUE)
          return indexPaletteColor(int(c.value()), ng);
        else
          return indexPaletteColor(ig, ng);
      }
    }
    else {
      if (c.type() == CQChartsColor::Type::INDEXED_VALUE)
        return indexPaletteColor(int(c.value()), ng);
      else
        return indexPaletteColor(ig, ng);
    }
  }
  else if (c.type() == CQChartsColor::Type::INTERFACE ||
           c.type() == CQChartsColor::Type::INTERFACE_VALUE) {
    if (c.type() == CQChartsColor::Type::INTERFACE_VALUE)
      return interpThemeColor(ColorInd(c.value()));
    else
      return interpThemeColor(ColorInd(value));
  }
  else if (c.type() == CQChartsColor::Type::CONTRAST ||
           c.type() == CQChartsColor::Type::CONTRAST_VALUE) {
    value = 1.0; // no interp ?

    auto cc = contrastColor();

    if (! cc.isValid()) {
      if (c.type() == CQChartsColor::Type::CONTRAST_VALUE)
        return interpThemeColor(ColorInd(1.0 - c.value()));
      else
        return interpThemeColor(ColorInd(1.0 - value));
    }

    auto c1 = CQChartsUtil::bwColor(cc);
    auto c2 = CQChartsUtil::bwColor(c1);

    if (c.type() == CQChartsColor::Type::CONTRAST_VALUE)
      c1 = CQChartsUtil::blendColors(c1, c2, CMathUtil::clamp(c.value(), 0.0, 1.0));
    else
      c1 = CQChartsUtil::blendColors(c1, c2, CMathUtil::clamp(value, 0.0, 1.0));

    return c1;
  }
  else if (c.type() == CQChartsColor::Type::MODEL ||
           c.type() == CQChartsColor::Type::MODEL_VALUE) {
    if (c.type() == CQChartsColor::Type::MODEL_VALUE)
      return interpModelColor(c, c.value());
    else
      return interpModelColor(c, value);
  }
  else if (c.type() == CQChartsColor::Type::LIGHTER ||
           c.type() == CQChartsColor::Type::LIGHTER_VALUE) {
    if (c.type() == CQChartsColor::Type::LIGHTER_VALUE)
      return ic.lighter(150 + c.value());
    else
      return ic.lighter();
  }
  else if (c.type() == CQChartsColor::Type::DARKER ||
           c.type() == CQChartsColor::Type::DARKER_VALUE) {
    if (c.type() == CQChartsColor::Type::DARKER_VALUE)
      return ic.darker(150 + c.value());
    else
      return ic.darker();
  }

  return QColor(0, 0, 0);
}

//---

void
CQCharts::
setPlotTheme(const CQChartsThemeName &themeName)
{
  CQChartsUtil::testAndSet(plotTheme_, themeName, [&]() { emit themeChanged(); } );
}

bool
CQCharts::
isDark() const
{
  return interfaceTheme()->isDark();
}

void
CQCharts::
setDark(bool b)
{
  interfaceTheme()->setDark(b);

  emit interfaceThemeChanged();
}

//---

QColor
CQCharts::
interpPaletteColor(const ColorInd &ind, bool scale) const
{
  return interpIndPaletteColor(/*palette_ind*/-1, ind.value(), scale);
}

QColor
CQCharts::
interpIndPaletteColor(int ind, int i, int n, bool scale) const
{
  double r = CMathUtil::norm(i, 0, n - 1);

  return interpIndPaletteColor(ind, r, scale);
}

QColor
CQCharts::
interpIndPaletteColor(int ind, double r, bool scale) const
{
  return interpIndPaletteColorValue(ind, 0, 1, r, scale);
}

QColor
CQCharts::
interpNamePaletteColor(const QString &name, double r, bool scale) const
{
  return interpNamePaletteColorValue(name, 0, 1, r, scale);
}

QColor
CQCharts::
interpGroupPaletteColor(const ColorInd &ig, const ColorInd &iv, bool scale) const
{
  return themeGroupPalette(ig.i, ig.n)->getColor(iv.value(), scale);
}

QColor
CQCharts::
interpPaletteColorValue(int ig, int ng, int i, int n, bool scale) const
{
  double r = CMathUtil::norm(i, 0, n - 1);

  return interpIndPaletteColorValue(/*palette_ind*/-1, ig, ng, r, scale);
}

QColor
CQCharts::
interpPaletteColorValue(int ig, int ng, double r, bool scale) const
{
  return interpIndPaletteColorValue(/*palette_ind*/-1, ig, ng, r, scale);
}

QColor
CQCharts::
interpIndPaletteColorValue(int ind, int /*ig*/, int /*ng*/, double r, bool scale) const
{
  // if ind unset then use default palette number
  if (ind < 0)
    ind = 0;

  auto *palette = this->themePalette(ind);

#if 0
  if (palette->isDistinct() && ng > 0) {
    int nc = palette->numDefinedColors();
    assert(nc > 0);

    int i1 = (ig % nc);

    double r1 = CMathUtil::norm(i1, 0, nc - 1);

    return palette->getColor(r1, /*scale*/false);
  }
#endif

  return palette->getColor(r, scale);
}

QColor
CQCharts::
interpNamePaletteColorValue(const QString &name, int /*ig*/, int /*ng*/,
                            double r, bool scale) const
{
  auto *palette = CQColorsMgrInst->getNamedPalette(name);
  if (! palette) return QColor(); // assert ?

  return palette->getColor(r, scale);
}

QColor
CQCharts::
indexPaletteColor(int i, int n) const
{
  return indexIndPaletteColor(0, i, n);
}

QColor
CQCharts::
indexIndPaletteColor(int ind, int i, int n) const
{
  // if ind unset then use default palette number
  if (ind < 0)
    ind = 0;

  auto *palette = this->themePalette(ind);

  return palette->getColor(i, n, CQColorsPalette::WrapMode::REPEAT);
}

QColor
CQCharts::
indexNamePaletteColor(const QString &name, int i, int n) const
{
  auto *palette = CQColorsMgrInst->getNamedPalette(name);
  if (! palette) return QColor(); // assert ?

  return palette->getColor(i, n, CQColorsPalette::WrapMode::REPEAT);
}

QColor
CQCharts::
interpThemeColor(const ColorInd &ind) const
{
  return this->interfaceTheme()->interpColor(ind.value(), /*scale*/true);
}

QColor
CQCharts::
interpInterfaceColor(double r) const
{
  return this->interfaceTheme()->interpColor(r, /*scale*/true);
}

CQColorsPalette *
CQCharts::
themeGroupPalette(int ig, int /*ng*/) const
{
  if (ig < 0)
    ig = 0;

  return theme()->palette(ig);
}

CQColorsPalette *
CQCharts::
themePalette(int ind) const
{
  // if ind unset then use default palette number
  if (ind < 0)
    ind = 0;

  return theme()->palette(ind);
}

const CQColorsTheme *
CQCharts::
theme() const
{
  return plotTheme().obj();
}

CQColorsTheme *
CQCharts::
theme()
{
  return plotTheme().obj();
}

CQChartsColor
CQCharts::
adjustDefaultPalette(const CQChartsColor &c, const QString &defaultPalette) const
{
  if ((c.type() == CQChartsColor::Type::PALETTE ||
       c.type() == CQChartsColor::Type::PALETTE_VALUE) &&
      c.ind() < 0) {
    auto c1 = c;

    int ind = theme()->paletteInd(defaultPalette);

    c1.setInd(ind);

    return c1;
  }

  return c;
}

//---

QColor
CQCharts::
interpModelColor(const CQChartsColor &c, double value) const
{
  int ir, ig, ib;

  c.getModelRGB(ir, ig, ib);

  double r = CQColorsPalette::interpModel(ir, value);
  double g = CQColorsPalette::interpModel(ig, value);
  double b = CQColorsPalette::interpModel(ib, value);

  return CQChartsUtil::rgbToColor(r, g, b);
}

//---

CQChartsModelData *
CQCharts::
initModelData(ModelP &model)
{
  int ind;

  if (! getModelInd(model.data(), ind))
    ind = addModelData(model);

  return getModelData(ind);
}

CQChartsModelData *
CQCharts::
getModelData(const QAbstractItemModel *model) const
{
  if (! model)
    return nullptr;

  int ind;

  if (! getModelInd(model, ind))
    return nullptr;

  return getModelData(ind);
}

void
CQCharts::
setCurrentModelData(CQChartsModelData *modelData)
{
  if (modelData)
    setCurrentModelInd(modelData->ind());
  else
    setCurrentModelInd(-1);
}

void
CQCharts::
setCurrentModelInd(int ind)
{
  currentModelInd_ = ind;

  emit currentModelChanged(currentModelInd_);
}

CQChartsModelData *
CQCharts::
currentModelData() const
{
  if (modelDatas_.empty())
    return nullptr;

  if (currentModelInd_ >= 0 ) {
    for (auto &modelData : modelDatas_) {
      if (modelData->ind() == currentModelInd_)
        return modelData;
    }
  }

  return modelDatas_.back();
}

int
CQCharts::
addModelData(ModelP &model)
{
  auto *modelData = new CQChartsModelData(this, model);

  int ind;

  bool rc = assignModelInd(model.data(), ind);

  assert(rc);

  modelData->setInd(ind);

  connect(modelData, SIGNAL(dataChanged()), this, SIGNAL(modelDataDataChanged()));
//connect(modelData, SIGNAL(modelChanged()), this, SIGNAL(modelDataContentsChanged()));
//connect(modelData, SIGNAL(currentModelChanged()), this, SIGNAL(modelDataCurrentModelChanged()));

  //---

  modelDatas_.push_back(modelData);

  ind = modelData->ind();

  emit modelDataAdded(ind);
  emit modelDataChanged();

  return ind;
}

bool
CQCharts::
removeModelData(ModelP &model)
{
  auto *modelData = getModelData(model.data());

  if (! modelData)
    return false;

  return removeModelData(modelData);
}

bool
CQCharts::
removeModelData(CQChartsModelData *modelData)
{
  int ind = modelData->ind();

  int i = 0;
  int n = modelDatas_.size();

  for ( ; i < n; ++i) {
    if (modelDatas_[i] == modelData)
      break;
  }

  if (i >= n)
    return false;

  ++i;

  for ( ; i < n; ++i)
    modelDatas_[i - 1] = modelDatas_[i];

  modelDatas_.pop_back();

  modelData->emitDeleted();

  delete modelData;

  emit modelDataRemoved(ind);
  emit modelDataChanged();

  return true;
}

bool
CQCharts::
getModelInd(const QAbstractItemModel *model, int &ind) const
{
  ind = -1;

  if (! model)
    return false;

  bool ok;

  ind = (int) CQChartsVariant::toInt(model->property("modelInd"), ok);

  if (! ok)
    return false;

  return true;
}

bool
CQCharts::
assignModelInd(QAbstractItemModel *model, int &ind)
{
  ind = ++lastModelInd_;

  return setModelInd(model, ind);
}

bool
CQCharts::
setModelInd(QAbstractItemModel *model, int ind)
{
  if (! model)
    return false;

  model->setProperty("modelInd", ind);

  return true;
}

CQChartsModelData *
CQCharts::
getModelData(int ind) const
{
  for (auto &modelData : modelDatas_)
    if (modelData->ind() == ind)
      return modelData;

  return nullptr;
}

CQChartsModelData *
CQCharts::
getModelData(const QString &id) const
{
  for (auto &modelData : modelDatas_)
    if (modelData->id() == id)
      return modelData;

  return nullptr;
}

void
CQCharts::
getModelDatas(ModelDatas &modelDatas) const
{
  modelDatas = modelDatas_;
}

void
CQCharts::
setModelName(CQChartsModelData *modelData, const QString &name)
{
  modelData->setName(name);
}

void
CQCharts::
setModelFileName(CQChartsModelData *modelData, const QString &fileName)
{
  modelData->setFilename(fileName);
}

//---

CQChartsView *
CQCharts::
addView(const QString &id)
{
  auto *view = createView();

  if (id.length())
    view->setId(id);

  addView(view);

  return view;
}

void
CQCharts::
addView(CQChartsView *view)
{
  connect(view, SIGNAL(plotAdded(CQChartsPlot *)), this, SIGNAL(plotAdded(CQChartsPlot *)));

  //---

  QString id = view->id();

  if (id == "")
    id = QString("view%1").arg(views_.size() + 1);

  assert(! getView(id));

  view->setId(id);

  view->setObjectName(id);

  views_[id] = view;

  emit viewAdded(view);
}

CQChartsView *
CQCharts::
createView()
{
  auto *view = new CQChartsView(this);

  return view;
}

void
CQCharts::
deleteView(CQChartsView *view)
{
  delete view;
}

CQChartsView *
CQCharts::
getView(const QString &id) const
{
  auto p = views_.find(id);

  if (p == views_.end())
    return nullptr;

  return (*p).second;
}

CQChartsView *
CQCharts::
currentView() const
{
  if (views_.empty())
    return nullptr;

  return views_.rbegin()->second;
}

void
CQCharts::
getViews(Views &views) const
{
  for (const auto &view : views_)
    views.push_back(view.second);
}

void
CQCharts::
getViewIds(QStringList &names) const
{
  for (const auto &view : views_)
    names.push_back(view.second->id());
}

void
CQCharts::
removeView(CQChartsView *view)
{
  emit viewRemoved(view);

  views_.erase(view->id());
}

//---

CQChartsWindow *
CQCharts::
createWindow(CQChartsView *view)
{
  auto *window = CQChartsWindowMgrInst->createWindow(view);

  emit windowCreated(window);

  return window;
}

void
CQCharts::
deleteWindow(CQChartsWindow *window)
{
  emit windowRemoved(window);

  CQChartsWindowMgrInst->removeWindow(window);
}

//---

void
CQCharts::
addProc(ProcType type, const QString &name, const QString &args, const QString &body)
{
  typeProcs_[type][name] = ProcData(name, args, body);
}

void
CQCharts::
removeProc(ProcType type, const QString &name)
{
  typeProcs_[type].erase(name);
}

void
CQCharts::
getProcNames(ProcType type, QStringList &names) const
{
  auto pt = typeProcs_.find(type);
  if (pt == typeProcs_.end()) return;

  const auto &procs = (*pt).second;

  for (const auto &proc : procs) {
    names.push_back(proc.first);
  }
}

bool
CQCharts::
getProcData(ProcType type, const QString &name, QString &args, QString &body) const
{
  auto pt = typeProcs_.find(type);
  if (pt == typeProcs_.end()) return false;

  const auto &procs = (*pt).second;

  auto pn = procs.find(name);
  if (pn == procs.end()) return false;

  args = (*pn).second.args;
  body = (*pn).second.body;

  return true;
}

const CQCharts::Procs &
CQCharts::
procs(ProcType type) const
{
  static Procs noProcs;

  auto pt = typeProcs_.find(type);
  if (pt == typeProcs_.end()) return noProcs;

  return (*pt).second;
}

//---

void
CQCharts::
emitModelTypeChanged(int modelId)
{
  emit modelTypeChanged(modelId);
}

//---

void
CQCharts::
setItemIsStyle(CQPropertyViewItem *item)
{
  item->setProperty("style_prop", true);
}

bool
CQCharts::
getItemIsStyle(const CQPropertyViewItem *item)
{
  return item->property("style_prop").isValid();
}

void
CQCharts::
setItemIsHidden(CQPropertyViewItem *item)
{
  item->setHidden(true);

  item->setProperty("hidden_prop", true);
}

bool
CQCharts::
getItemIsHidden(const CQPropertyViewItem *item)
{
  return item->property("hidden_prop").isValid();
}

//---

CQChartsLoadModelDlg *
CQCharts::
loadModelDlg()
{
  if (! loadModelDlg_)
    loadModelDlg_ = new CQChartsLoadModelDlg(this);

  loadModelDlg_->show();
  loadModelDlg_->raise();

  return loadModelDlg_;
}

CQChartsManageModelsDlg *
CQCharts::
manageModelsDlg()
{
  if (! manageModelsDlg_)
    manageModelsDlg_ = new CQChartsManageModelsDlg(this);

  manageModelsDlg_->show();
  manageModelsDlg_->raise();

  return manageModelsDlg_;
}

CQChartsEditModelDlg *
CQCharts::
editModelDlg(CQChartsModelData *modelData)
{
  if (! editModelDlg_ || editModelDlg_->model() != modelData->currentModel()) {
    delete editModelDlg_;

    editModelDlg_ = new CQChartsEditModelDlg(this, modelData);
  }

  editModelDlg_->show();
  editModelDlg_->raise();

  return editModelDlg_;
}

CQChartsCreatePlotDlg *
CQCharts::
createPlotDlg(CQChartsModelData *modelData)
{
  if (! createPlotDlg_ || createPlotDlg_->model() != modelData->currentModel()) {
    delete createPlotDlg_;

    createPlotDlg_ = new CQChartsCreatePlotDlg(this, modelData);
  }

  createPlotDlg_->show();
  createPlotDlg_->raise();

  return createPlotDlg_;
}

//---

void
CQCharts::
errorMsg(const QString &msg) const
{
  std::cerr << msg.toStdString() << "\n";
}
