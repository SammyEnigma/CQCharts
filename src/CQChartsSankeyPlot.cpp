#include <CQChartsSankeyPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsAnalyzeModelData.h>
#include <CQChartsModelUtil.h>
#include <CQChartsUtil.h>
#include <CQChartsVariant.h>
#include <CQCharts.h>
#include <CQChartsConnectionList.h>
#include <CQChartsValueSet.h>
#include <CQChartsTip.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsEditHandles.h>
#include <CQChartsTextPlacer.h>
#include <CQChartsHtml.h>
#include <CQChartsRand.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>
#include <CMathRound.h>

#include <QMenu>
#include <QAction>

CQChartsSankeyPlotType::
CQChartsSankeyPlotType()
{
}

void
CQChartsSankeyPlotType::
addParameters()
{
  CQChartsConnectionPlotType::addParameters();
}

QString
CQChartsSankeyPlotType::
description() const
{
  auto B   = [](const QString &str) { return CQChartsHtml::Str::bold(str); };
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
    h2("Sankey Plot").
     h3("Summary").
      p("Draw connected objects as a connected flow graph.").
     h3("Columns").
      p("Connections between two nodes (edge) are described using " + B("Node/Connections") +
        " columns, a " + B("Link") + " column, a " + B("Path") + " column or " + B("From/To") +
        " columns").
      p("The connection size can be specified using the " + B("Value") + " column.").
      p("The depth (x postion) can be specified using the " + B("Depth") + " column.").
      p("Extra attributes (color, label) for the source node, destination node or edge "
        "can be specified using the " + B("Attributes") + " column.").
      p("The depth (x postion) can be specified using the " + B("Depth") + " column.").
      p("The source node group can be specified using the " + B("Group") + " column.").
      p("The source node name can be specified using the " + B("Name") + " column.").
     h3("Options").
      p("The separator character for hierarchical names (Link or Path columns) can be "
        "specified using the " + B("separator") + " option.").
      p("The " + B("symmetric") + " option can be used to specify that the connection is "
        "symmetric (value of connection for from/to is the same as to/from)").
      p("The " + B("sorted") + " option can be used to sort the connections by value").
      p("The " + B("maxDepth") + " option can be used to filter out connections greater than "
        "the specified depth.").
      p("The " + B("minValue") + " option can be used filter out connections less than the "
        "specified value.").
     h3("Limitations").
      p("As each model row is an edge it is hard to specify data on just a node.").
      p("The From/To columns do allow the To column to be empty to define attributes "
        "on just the from node but this is not ideal as it makes the raw data hard to "
        "manipulate").
     h3("Example").
      p(IMG("images/sankey.png"));
}

bool
CQChartsSankeyPlotType::
isColumnForParameter(ColumnDetails *columnDetails, Parameter *parameter) const
{
  return CQChartsConnectionPlotType::isColumnForParameter(columnDetails, parameter);
}

void
CQChartsSankeyPlotType::
analyzeModel(ModelData *modelData, AnalyzeModelData &analyzeModelData)
{
  CQChartsConnectionPlotType::analyzeModel(modelData, analyzeModelData);
}

CQChartsPlot *
CQChartsSankeyPlotType::
create(View *view, const ModelP &model) const
{
  return new CQChartsSankeyPlot(view, model);
}

//------

CQChartsSankeyPlot::
CQChartsSankeyPlot(View *view, const ModelP &model) :
 CQChartsConnectionPlot(view, view->charts()->plotType("sankey"), model),
 CQChartsObjNodeShapeData<CQChartsSankeyPlot>(this),
 CQChartsObjNodeTextData <CQChartsSankeyPlot>(this),
 CQChartsObjEdgeShapeData<CQChartsSankeyPlot>(this),
 CQChartsObjEdgeTextData <CQChartsSankeyPlot>(this)
{
}

CQChartsSankeyPlot::
~CQChartsSankeyPlot()
{
  CQChartsSankeyPlot::term();
}

//---

void
CQChartsSankeyPlot::
init()
{
  CQChartsConnectionPlot::init();

  //---

  NoUpdate noUpdate(this);

  setLayerActive(Layer::Type::FG_PLOT, true);

  //---

  auto bg = Color::makePalette();

  setNodeFilled(true);
  setNodeFillColor(bg);
  setNodeFillAlpha(Alpha(1.0));

  setNodeStroked(true);
  setNodeStrokeAlpha(Alpha(0.2));

  //---

  setEdgeFilled(true);
  setEdgeFillColor(bg);
  setEdgeFillAlpha(Alpha(0.25));

  setEdgeStroked(true);
  setEdgeStrokeAlpha(Alpha(0.2));

  //---

  addTitle();

  //---

  addColorMapKey();

  //---

  bbox_ = targetBBox_;

  setFitMargin(PlotMargin(Length::percent(5), Length::percent(5),
                          Length::percent(5), Length::percent(5)));

  //---

  placer_ = std::make_unique<CQChartsTextPlacer>();
}

void
CQChartsSankeyPlot::
term()
{
  // delete objects first to ensure link from edge/node to object reset
  clearPlotObjects();

  clearNodesAndEdges();

  clearGraph();
}

//---

void
CQChartsSankeyPlot::
initNodeColumns()
{
  CQChartsConnectionPlot::initNodeColumns();

  nodeLabelColumn_      .setModelInd(nodeModel_.modelInd());
  nodeValueColumn_      .setModelInd(nodeModel_.modelInd());
  nodeFillColorColumn_  .setModelInd(nodeModel_.modelInd());
  nodeFillAlphaColumn_  .setModelInd(nodeModel_.modelInd());
  nodeFillPatternColumn_.setModelInd(nodeModel_.modelInd());
  nodeStrokeColorColumn_.setModelInd(nodeModel_.modelInd());
  nodeStrokeAlphaColumn_.setModelInd(nodeModel_.modelInd());
  nodeStrokeWidthColumn_.setModelInd(nodeModel_.modelInd());
  nodeStrokeDashColumn_ .setModelInd(nodeModel_.modelInd());
}

void
CQChartsSankeyPlot::
setNodeLabelColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeLabelColumn_, c, [&]() {
    nodeLabelColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeValueColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeValueColumn_, c, [&]() {
    nodeValueColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeFillColorColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeFillColorColumn_, c, [&]() {
    nodeFillColorColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeFillAlphaColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeFillAlphaColumn_, c, [&]() {
    nodeFillAlphaColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeFillPatternColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeFillPatternColumn_, c, [&]() {
    nodeFillPatternColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeStrokeColorColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeStrokeColorColumn_, c, [&]() {
    nodeStrokeColorColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeStrokeAlphaColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeStrokeAlphaColumn_, c, [&]() {
    nodeStrokeAlphaColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeStrokeWidthColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeStrokeWidthColumn_, c, [&]() {
    nodeStrokeWidthColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

void
CQChartsSankeyPlot::
setNodeStrokeDashColumn(const CQChartsModelColumn &c)
{
  CQChartsUtil::testAndSet(nodeStrokeDashColumn_, c, [&]() {
    nodeStrokeDashColumn_.setCharts(charts());

    updateRangeAndObjs();
  } );
}

//---

void
CQChartsSankeyPlot::
clearNodesAndEdges()
{
  for (const auto &pn : nameNodeMap_) {
    auto *node = pn.second;

    delete node;
  }

  for (const auto &edge : edges_)
    delete edge;

  nameNodeMap_.clear();
  indNodeMap_ .clear();
  edges_      .clear();

  clearGraph();

  maxNodeDepth_ = 0;
}

//---

void
CQChartsSankeyPlot::
setNodeMargin(const Length &l)
{
  CQChartsUtil::testAndSet(nodeMargin_, l, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setMinNodeMargin(double r)
{
  CQChartsUtil::testAndSet(minNodeMargin_, r, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeWidth(const Length &l)
{
  CQChartsUtil::testAndSet(nodeWidth_, l, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeMinWidth(const Length &l)
{
  CQChartsUtil::testAndSet(nodeMinWidth_, l, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeValueLabel(bool b)
{
  CQChartsUtil::testAndSet(nodeValueLabel_, b, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeValueBar(bool b)
{
  CQChartsUtil::testAndSet(nodeValueBar_, b, [&]() { updateObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeValueBarSize(double s)
{
  CQChartsUtil::testAndSet(nodeValueBarSize_, s, [&]() { updateObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setEdgeLine(bool b)
{
  CQChartsUtil::testAndSet(edgeLine_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setEdgeValueLabel(bool b)
{
  CQChartsUtil::testAndSet(edgeValueLabel_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setOrientation(const Qt::Orientation &o)
{
  CQChartsUtil::testAndSet(orientation_, o, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setSrcColoring(bool b)
{
  CQChartsUtil::testAndSet(srcColoring_, b, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setBlendEdgeColor(const BlendType &t)
{
  CQChartsUtil::testAndSet(blendEdgeColor_, t, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setAlign(const Align &a)
{
  CQChartsUtil::testAndSet(align_, a, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setAlignFirstLast(bool b)
{
  CQChartsUtil::testAndSet(alignFirstLast_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setSpread(const Spread &s)
{
  CQChartsUtil::testAndSet(spread_, s, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setAlignEnds(bool b)
{
  CQChartsUtil::testAndSet(alignEnds_, b, [&]() { updateRangeAndObjs(); } );
}

#ifdef CQCHARTS_GRAPH_PATH_ID
void
CQChartsSankeyPlot::
setSortPathIdNodes(bool b)
{
  CQChartsUtil::testAndSet(sortPathIdNodes_, b, [&]() { updateRangeAndObjs(); } );
}
#endif

#ifdef CQCHARTS_GRAPH_PATH_ID
void
CQChartsSankeyPlot::
setSortPathIdEdges(bool b)
{
  CQChartsUtil::testAndSet(sortPathIdEdges_, b, [&]() { updateRangeAndObjs(); } );
}
#endif

void
CQChartsSankeyPlot::
setAdjustNodes(bool b)
{
  CQChartsUtil::testAndSet(adjustNodes_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setAdjustCenters(bool b)
{
  CQChartsUtil::testAndSet(adjustCenters_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setRemoveOverlaps(bool b)
{
  CQChartsUtil::testAndSet(removeOverlaps_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsSankeyPlot::
setReorderEdges(bool b)
{
  CQChartsUtil::testAndSet(reorderEdges_, b, [&]() { updateRangeAndObjs(); } );
}

#if 0
void
CQChartsSankeyPlot::
setAdjustEdgeOverlaps(bool b)
{
  CQChartsUtil::testAndSet(adjustEdgeOverlaps_, b, [&]() { updateRangeAndObjs(); } );
}
#endif

void
CQChartsSankeyPlot::
setAdjustIterations(int n)
{
  CQChartsUtil::testAndSet(adjustIterations_, n, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setNodeTextLabelPosition(const TextPosition &pos)
{
  CQChartsUtil::testAndSet(nodeTextLabelPosition_, pos, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeTextLabelAlign(const Qt::Alignment &align)
{
  CQChartsUtil::testAndSet(nodeTextLabelAlign_, align, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeTextValuePosition(const TextPosition &pos)
{
  CQChartsUtil::testAndSet(nodeTextValuePosition_, pos, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setNodeTextValueAlign(const Qt::Alignment &align)
{
  CQChartsUtil::testAndSet(nodeTextValueAlign_, align, [&]() { drawObjs(); } );
}

void
CQChartsSankeyPlot::
setAdjustText(bool b)
{
  CQChartsUtil::testAndSet(adjustText_, b, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setUseMaxTotals(bool b)
{
  CQChartsUtil::testAndSet(useMaxTotals_, b, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsSankeyPlot::
setNewVisited(bool b)
{
  CQChartsUtil::testAndSet(newVisited_, b, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsSankeyPlot::
addProperties()
{
  CQChartsConnectionPlot::addProperties();

  //---

  // node columns
  addPropI("nodeModel", "nodeLabelColumn"      , "labelColumn"      , "Node label column");
  addPropI("nodeModel", "nodeValueColumn"      , "valueColumn"      , "Node value column");
  addPropI("nodeModel", "nodeFillColorColumn"  , "fillColorColumn"  , "Node fill color column");
  addPropI("nodeModel", "nodeFillAlphaColumn"  , "fillAlphaColumn"  , "Node fill alpha column");
  addPropI("nodeModel", "nodeFillPatternColumn", "fillPatternColumn", "Node fill pattern column");
  addPropI("nodeModel", "nodeStrokeColorColumn", "strokeColorColumn", "Node stroke color column");
  addPropI("nodeModel", "nodeStrokeAlphaColumn", "strokeAlphaColumn", "Node stroke alpha column");
  addPropI("nodeModel", "nodeStrokeWidthColumn", "strokeWidthColumn", "Node stroke width column");
  addPropI("nodeModel", "nodeStrokeDashColumn" , "strokeDashColumn" , "Node stroke dash column");

  //---

  // placement
  addProp("placement", "align"             , "align"             , "Node alignment");
  addProp("placement", "alignFirstLast"    , "alignFirstLast"    , "Node align first/last");
  addProp("placement", "spread"            , "spread"            , "Node spread");
  addProp("placement", "alignEnds"         , "alignEnds"         , "Align start/end nodes");
#ifdef CQCHARTS_GRAPH_PATH_ID
  addProp("placement", "sortPathIdNodes"   , "sortPathIdNodes"   , "Sort depth nodes by path id");
  addProp("placement", "sortPathIdEdges"   , "sortPathIdEdges"   , "Sort node edges by path id");
#endif
  addProp("placement", "adjustNodes"       , "adjustNodes"       , "Adjust node placement");
  addProp("placement", "adjustCenters"     , "adjustCenters"     , "Adjust node centers");
  addProp("placement", "removeOverlaps"    , "removeOverlaps"    , "Remove overlapping nodes");
  addProp("placement", "reorderEdges"      , "reorderEdges"      , "Reorder edges");
//addProp("placement", "adjustEdgeOverlaps", "adjustEdgeOverlaps", "Adjust edge overlaps");
  addProp("placement", "adjustIterations"  , "adjustIterations"  , "Adjust iterations");
  addProp("placement", "constrainMove"     , "constrainMove"     , "Constrain move in edit mode");

  addProp("placement", "newVisited", "newVisited", "new visited calc");

  // options
  addProp("options", "useMaxTotals", "useMaxTotals", "Use max of src/dest totals for edge scaling");
  addProp("options", "orientation" , "orientation" , "Plot orientation");

  // coloring
  addProp("coloring", "srcColoring"      , "", "Color by Source Nodes");
  addProp("coloring", "blendEdgeColor"   , "", "Blend Edge Node Colors");
  addProp("coloring", "mouseColoring"    , "", "Mouse Over Connection Coloring");
  addProp("coloring", "mouseNodeColoring", "", "Mouse Over Node Coloring");

  // node
  addProp ("node", "nodeMargin"      , "margin"      , "Node margin (Y)");
  addPropI("node", "minNodeMargin"   , "minMargin"   , "Min Node margin (Pixels)");
  addProp ("node", "nodeWidth"       , "width"       , "Node width");
  addProp ("node", "nodeMinWidth"    , "minWidth"    , "Node min width");
  addProp ("node", "nodeValueLabel"  , "valueLabel"  , "Draw node value as label");
  addProp ("node", "nodeValueBar"    , "valueBar"    , "Draw node value bar");
  addProp ("node", "nodeValueBarSize", "valueBarSize", "Draw node value bar size");

  // node style
  addProp("node/stroke", "nodeStroked", "visible", "Node stroke visible");

  addLineProperties("node/stroke", "nodeStroke", "Node");

  addProp("node/fill", "nodeFilled", "visible", "Node fill visible");

  addFillProperties("node/fill", "nodeFill", "Node");

  //---

  // node text
  addProp("node/text", "nodeTextVisible"        , "visible"        , "Node text label visible");
  addProp("node/text", "nodeTextInsideVisible"  , "insideVisible"  , "Node text inside visible");
  addProp("node/text", "nodeTextSelectedVisible", "selectedVisible", "Node text selected visible");

  addProp("node/text", "nodeTextLabelPosition", "labelPosition", "Node text label position");
  addProp("node/text", "nodeTextLabelAlign"   , "labelAlign"   , "Node text label alignment");

  addProp("node/text", "nodeTextValuePosition", "valuePosition", "Node text value position");
  addProp("node/text", "nodeTextValueAlign"   , "valueAlign"   , "Node text value alignment");

  addTextProperties("node/text", "nodeText", "", TextOptions::ValueType::ALL);

  //---

  // edge
  addProp("edge", "edgeLine"      , "line"      , "Draw line for edge");
  addProp("edge", "edgeValueLabel", "valueLabel", "Draw edge value as label");

  // edge style
  addProp("edge/stroke", "edgeStroked", "visible", "Edge stroke visible");

  addLineProperties("edge/stroke", "edgeStroke", "Edge");

  addProp("edge/fill", "edgeFilled", "visible", "Edge fill visible");

  addFillProperties("edge/fill", "edgeFill", "Edge");

  // edge text
  addProp("edge/text", "edgeTextVisible"        , "visible"        , "Edge text label visible");
  addProp("edge/text", "edgeTextInsideVisible"  , "insideVisible"  , "Edge text inside visible");
  addProp("edge/text", "edgeTextSelectedVisible", "selectedVisible", "Edge text selected visible");

  addTextProperties("edge/text", "edgeText", "", TextOptions::ValueType::ALL);

  //---

  // general text properties
  addProp("text", "adjustText", "adjustText", "Adjust text placement");

  //---

  // color map
  addColorMapProperties();

  // color map key
  addColorMapKeyProperties();
}

//---

CQChartsGeom::Range
CQChartsSankeyPlot::
calcRange() const
{
  if (nodeTextLabelPosition() != TextPosition::INTERNAL || isNodeValueBar()) {
    double dx1 = 0.0, dx2 = 0.0, dy1 = 0.0, dy2 = 0.0;

    if (isNodeValueBar()) {
      int depth = this->maxPos() - this->minPos();

      double db = (depth > 1 ? 1.0/depth : 0.0);

      auto barSize = std::min(std::max(nodeValueBarSize(), 0.0), 1.0);

      if (isHorizontal())
        dx2 = barSize*db;
      else
        dy2 = barSize*db;
    }

    if (nodeTextLabelPosition() != TextPosition::INTERNAL) {
      auto font = view()->plotFont(this, nodeTextFont());

      QFontMetricsF fm(font);

      if (isHorizontal()) {
        auto fw = pixelToWindowWidth(fm.height())*1.1;

        dx1 = std::max(dx1, fw);
        dx2 = std::max(dx2, fw);
      }
      else {
        auto fh = pixelToWindowHeight(fm.height())*1.1;

        dy1 = std::max(dy1, fh);
        dy2 = std::max(dy2, fh);
      }
    }

    return Range(targetBBox_.getXMin() - dx1, targetBBox_.getYMin() - dy1,
                 targetBBox_.getXMax() + dx2, targetBBox_.getYMax() + dy2);
  }
  else {
    return Range(targetBBox_);
  }
}

//---

CQChartsGeom::Range
CQChartsSankeyPlot::
objTreeRange() const
{
  auto bbox = nodesBBox();

  return Range(bbox.getXMin(), bbox.getYMax(), bbox.getXMax(), bbox.getYMin());
}

CQChartsGeom::BBox
CQChartsSankeyPlot::
nodesBBox() const
{
  // calc bounding box of all nodes
  BBox bbox;

  for (const auto &idNode : indNodeMap_) {
    auto *node = idNode.second;
    if (! node->isVisible()) continue;

    bbox += node->rect();
  }

  return bbox;
}

//------

bool
CQChartsSankeyPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsSankeyPlot::createObjs");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  th->clearErrors();

  //---

  // check columns
  if (! checkColumns())
    return false;

  //---

  // init objects
  th->clearNodesAndEdges();

  auto *model = this->currentModel().data();
  if (! model) return false;

  //---

#ifdef CQCHARTS_GRAPH_PATH_ID
  pathIdMinMax_.reset();
#endif

  valueRange_.reset();

  //---

  // create objects
  auto columnDataType = calcColumnDataType();

  bool rc = false;

  if      (columnDataType == ColumnDataType::HIER)
    rc = initHierObjs();
  else if (columnDataType == ColumnDataType::LINK)
    rc = initLinkObjs();
  else if (columnDataType == ColumnDataType::CONNECTIONS)
    rc = initConnectionObjs();
  else if (columnDataType == ColumnDataType::PATH)
    rc = initPathObjs();
  else if (columnDataType == ColumnDataType::FROM_TO)
    rc = initFromToObjs();
  else if (columnDataType == ColumnDataType::TABLE)
    rc = initTableObjs();

  if (! rc)
    return false;

  //---

  processMetaData();

  //---

  if (nodeModel().isValid() && nodeIdColumn().isValid()) {
    auto *nodeModelData = nodeModel().modelData();

    class NodeVisitor : public CQChartsModelVisitor {
     public:
      NodeVisitor(const CQChartsSankeyPlot *plot, CQChartsGeom::RMinMax &valueRange) :
       plot_(plot), valueRange_(valueRange) {
        modelInd_ = plot_->nodeModel().modelInd();
        idColumn_ = plot_->nodeIdColumn().column();

        if (plot_->nodeLabelColumn().isValid())
          labelColumn_ = plot_->nodeLabelColumn().column();

        if (plot_->nodeValueColumn().isValid())
          valueColumn_ = plot_->nodeValueColumn().column();

        if (plot_->nodeFillColorColumn().isValid())
          fillColorColumn_ = plot_->nodeFillColorColumn().column();

        if (plot_->nodeFillAlphaColumn().isValid())
          fillAlphaColumn_ = plot_->nodeFillAlphaColumn().column();

        if (plot_->nodeFillPatternColumn().isValid())
          fillPatternColumn_ = plot_->nodeFillPatternColumn().column();

        if (plot_->nodeStrokeColorColumn().isValid())
          strokeColorColumn_ = plot_->nodeStrokeColorColumn().column();

        if (plot_->nodeStrokeAlphaColumn().isValid())
          strokeAlphaColumn_ = plot_->nodeStrokeAlphaColumn().column();

        if (plot_->nodeStrokeWidthColumn().isValid())
          strokeWidthColumn_ = plot_->nodeStrokeWidthColumn().column();

        if (plot_->nodeStrokeDashColumn().isValid())
          strokeDashColumn_ = plot_->nodeStrokeDashColumn().column();
      }

      // visit row
      State visit(const QAbstractItemModel *model, const VisitData &data) override {
        bool ok;

        auto var = CQChartsModelUtil::modelValue(
          plot_->charts(), model, data.row, idColumn_, data.parent, ok);
        if (! var.isValid()) return State::SKIP;

        auto id = var.toString();

        auto *plot = const_cast<CQChartsSankeyPlot *>(plot_);

        auto *node = plot->findNode(id, /*create*/false);
        if (! node) return State::SKIP;

        if (labelColumn_.isValid()) {
          auto labelVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, labelColumn_, data.parent, ok);

          if (labelVar.isValid())
            node->setLabel(labelVar.toString());
        }

        if (valueColumn_.isValid()) {
          auto valueVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, valueColumn_, data.parent, ok);

          if (valueVar.isValid()) {
            auto value = CQChartsVariant::toReal(valueVar, ok);
            if (ok) {
              node->setValue(OptReal(value));

              valueRange_.add(value);
            }
          }
        }

        if (fillColorColumn_.isValid()) {
          auto colorVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, fillColorColumn_, data.parent, ok);

          if (colorVar.isValid()) {
            auto c = CQChartsVariant::toColor(colorVar, ok);
            if (ok) node->setFillColor(c);
          }
        }

        if (fillAlphaColumn_.isValid()) {
          auto alphaVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, fillAlphaColumn_, data.parent, ok);

          if (alphaVar.isValid()) {
            auto a = CQChartsVariant::toAlpha(alphaVar, ok);
            if (ok) node->setFillAlpha(a);
          }
        }

        if (fillPatternColumn_.isValid()) {
          auto patternVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, fillPatternColumn_, data.parent, ok);

          if (patternVar.isValid()) {
            auto p = CQChartsFillPattern(patternVar.toString());
            node->setFillPattern(p);
          }
        }

        if (strokeColorColumn_.isValid()) {
          auto colorVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, strokeColorColumn_, data.parent, ok);

          if (colorVar.isValid()) {
            auto c = CQChartsVariant::toColor(colorVar, ok);
            if (ok) node->setStrokeColor(c);
          }
        }

        if (strokeAlphaColumn_.isValid()) {
          auto alphaVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, strokeAlphaColumn_, data.parent, ok);

          if (alphaVar.isValid()) {
            auto a = CQChartsVariant::toAlpha(alphaVar, ok);
            if (ok) node->setStrokeAlpha(a);
          }
        }

        if (strokeWidthColumn_.isValid()) {
          auto widthVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, strokeWidthColumn_, data.parent, ok);

          if (widthVar.isValid()) {
            auto w = CQChartsVariant::toLength(widthVar, ok);
            if (ok) node->setStrokeWidth(w);
          }
        }

        if (strokeDashColumn_.isValid()) {
          auto dashVar = CQChartsModelUtil::modelValue(
            plot_->charts(), model, data.row, strokeDashColumn_, data.parent, ok);

          if (dashVar.isValid()) {
            auto d = CQChartsLineDash(dashVar.toString());
            node->setStrokeDash(d);
          }
        }

        return State::OK;
      }

     private:
      const CQChartsSankeyPlot* plot_ { nullptr };
      CQChartsGeom::RMinMax&    valueRange_;

      int    modelInd_ { - 1};
      Column idColumn_;
      Column labelColumn_;
      Column valueColumn_;
      Column fillColorColumn_;
      Column fillAlphaColumn_;
      Column fillPatternColumn_;
      Column strokeColorColumn_;
      Column strokeAlphaColumn_;
      Column strokeWidthColumn_;
      Column strokeDashColumn_;
    };

    NodeVisitor nodeVisitor(this, valueRange_);

    CQChartsModelVisit::exec(charts(), nodeModelData->model().data(), nodeVisitor);
  }

  //---

  th->filterObjs();

  //---

  // create graph (and place)
  createObjsGraph(objs);

  //---

  return true;
}

bool
CQChartsSankeyPlot::
fitToBBox() const
{
  bool changed = false;

  //---

  // current
  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  th->bbox_ = nodesBBox();

  double x1 = bbox_.getXMin();
  double y1 = bbox_.getYMin();

  // target
  auto bbox = targetBBox_;

  double x2 = bbox.getXMin  ();
  double y2 = bbox.getYMin  ();
  double w2 = bbox.getWidth ();
  double h2 = bbox.getHeight();

  //---

  if (isHorizontal()) {
    // move all to bottom
    auto dy = y2 - y1;

    if (std::abs(dy) > 1E-6) {
      for (const auto &idNode : th->indNodeMap_) {
        auto *node = idNode.second;
        if (! node->isVisible()) continue;

        node->moveBy(Point(0.0, dy));
      }

      changed = true;
    }
  }
  else {
    // move all to left
    auto dx = x2 - x1;

    if (std::abs(dx) > 1E-6) {
      for (const auto &idNode : th->indNodeMap_) {
        auto *node = idNode.second;
        if (! node->isVisible()) continue;

        node->moveBy(Point(dx, 0.0));
      }

      changed = true;
    }
  }

  //---

  th->bbox_ = nodesBBox();

  x1 = bbox_.getXMin();
  y1 = bbox_.getYMin();

  double w1 = bbox_.getWidth ();
  double h1 = bbox_.getHeight();

  //---

  // spread all to top/right
  double f = 1.0;

  if (isHorizontal())
    f = h2/h1;
  else
    f = w2/w1;

  for (const auto &idNode : th->indNodeMap_) {
    auto *node = idNode.second;
    if (! node->isVisible()) continue;

    if (isHorizontal()) {
      double ny1 = y2 + (node->rect().getYMin() - y1)*f;

      double dy = ny1 - node->rect().getYMin();

      if (std::abs(dy) > 1E-6) {
        node->moveBy(Point(0.0, dy));
        node->scale(1.0, f);
        changed = true;
      }
    }
    else {
      double nx1 = x2 + (node->rect().getXMin() - x1)*f;

      double dx = nx1 - node->rect().getXMin();

      if (std::abs(dx) > 1E-6) {
        node->moveBy(Point(dx, 0.0));
        node->scale(f, 1.0);
        changed = true;
      }
    }
  }

  //---

  if (! changed)
    return false;

  //---

  if (graph_)
    graph_->updateRect();

  return true;
}

//------

void
CQChartsSankeyPlot::
preDrawFgObjs(PaintDevice *) const
{
  if (! isAdjustText())
    return;

  placer_->clear();
}

void
CQChartsSankeyPlot::
postDrawFgObjs(PaintDevice *device) const
{
  if (! isAdjustText())
    return;

  auto rect = this->calcDataRect();

  placer_->place(rect);

  placer_->draw(device);
}

//------

bool
CQChartsSankeyPlot::
initHierObjs() const
{
  CQPerfTrace trace("CQChartsSankeyPlot::initHierObjs");

  //---

  if (! CQChartsConnectionPlot::initHierObjs())
    return false;

  //---

  if (isPropagate()) {
    auto *th = const_cast<CQChartsSankeyPlot *>(this);

    th->propagateHierValues();
  }

  return true;
}

void
CQChartsSankeyPlot::
initHierObjsAddHierConnection(const HierConnectionData &srcHierData,
                              const HierConnectionData &destHierData) const
{
  int srcDepth = srcHierData.linkStrs.size();

  Node *srcNode  = nullptr;
  Node *destNode = nullptr;

  initHierObjsAddConnection(srcHierData.parentStr, destHierData.parentStr, srcDepth,
                            destHierData.total, srcNode, destNode);

  if (srcNode) {
    QString srcStr;

    if (! srcHierData.linkStrs.empty())
      srcStr = srcHierData.linkStrs.back();

    valueRange_.add(destHierData.total);

    srcNode->setValue(OptReal(destHierData.total));
    srcNode->setName (srcStr);
  }
}

void
CQChartsSankeyPlot::
initHierObjsAddLeafConnection(const HierConnectionData &srcHierData,
                              const HierConnectionData &destHierData) const
{
  int srcDepth = srcHierData.linkStrs.size();

  Node *srcNode  = nullptr;
  Node *destNode = nullptr;

  initHierObjsAddConnection(srcHierData.parentStr, destHierData.parentStr, srcDepth,
                            destHierData.total, srcNode, destNode);

  if (destNode) {
    QString destStr;

    if (! destHierData.linkStrs.empty())
      destStr = destHierData.linkStrs.back();

    valueRange_.add(destHierData.total);

    destNode->setValue(OptReal(destHierData.total));
    destNode->setName (destStr);
  }
}

void
CQChartsSankeyPlot::
initHierObjsAddConnection(const QString &srcStr, const QString &destStr, int srcDepth,
                          double value, Node* &srcNode, Node* &destNode) const
{
  int destDepth = srcDepth + 1;

  if (maxDepth() <= 0 || srcDepth <= maxDepth())
    srcNode = findNode(srcStr);

  if (maxDepth() <= 0 || destDepth <= maxDepth())
    destNode = findNode(destStr);

  //---

  // create edge and link src/dest nodes
  auto *edge = (srcNode && destNode ? createEdge(OptReal(value), srcNode, destNode) : nullptr);

  if (edge) {
    srcNode ->addDestEdge(edge);
    destNode->addSrcEdge (edge);
  }

  //---

  // set node depths
  if (srcNode)
    srcNode->setDepth(srcDepth);

  if (destNode)
    destNode->setDepth(destDepth);
}

//---

bool
CQChartsSankeyPlot::
initPathObjs() const
{
  CQPerfTrace trace("CQChartsSankeyPlot::initPathObjs");

  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  th->maxNodeDepth_ = 0;

  //---

  if (! CQChartsConnectionPlot::initPathObjs())
    return false;

  //---

  if (isPropagate())
    th->propagatePathValues();

  return true;
}

void
CQChartsSankeyPlot::
addPathValue(const PathData &pathData) const
{
  int n = pathData.pathStrs.length();
  assert(n > 0);

  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  th->maxNodeDepth_ = std::max(maxNodeDepth_, n - 1);

  auto separator = calcSeparator();

  auto path1 = pathData.pathStrs[0];

  for (int i = 1; i < n; ++i) {
    auto path2 = path1 + separator + pathData.pathStrs[i];

    auto *srcNode  = findNode(path1);
    auto *destNode = findNode(path2);

    srcNode ->setLabel(pathData.pathStrs[i - 1]);
    destNode->setLabel(pathData.pathStrs[i    ]);

    srcNode ->setDepth(i - 1);
    destNode->setDepth(i    );

    if (i < n - 1) {
      bool hasEdge = srcNode->hasDestNode(destNode);

      if (! hasEdge) {
        // create edge and link src/dest nodes
        auto *edge = createEdge(OptReal(), srcNode, destNode);

        srcNode ->addDestEdge(edge);
        destNode->addSrcEdge (edge);
      }
    }
    else {
      // create edge and link src/dest nodes
      auto edgeValue = (! isPropagate() ? pathData.value : OptReal());

      auto *edge = createEdge(edgeValue, srcNode, destNode);

      srcNode ->addDestEdge(edge);
      destNode->addSrcEdge (edge);

      //---

      // add model indices
      auto addModelInd = [&](const ModelIndex &modelInd) {
        if (modelInd.isValid())
          edge->addModelInd(modelInd);
      };

      addModelInd(pathData.pathModelInd );
      addModelInd(pathData.valueModelInd);

      //---

      // set destination node value (will be propagated up)
      if (pathData.value.isSet())
        valueRange_.add(pathData.value.real());

      destNode->setValue(pathData.value);
    }

    path1 = path2;
  }
}

void
CQChartsSankeyPlot::
propagatePathValues()
{
  // propagate node value up through edges and parent nodes
  for (int depth = maxNodeDepth_; depth >= 0; --depth) {
    for (const auto &p : nameNodeMap_) {
      auto *node = p.second;
      if (node->depth() != depth) continue;

      // set node value from sum of dest values
      if (! node->hasValue()) {
        if (! node->destEdges().empty()) {
          OptReal sum;

          for (const auto &edge : node->destEdges()) {
            if (edge->hasValue()) {
              double value = edge->value().real();

              if (sum.isSet())
                sum = OptReal(sum.real() + value);
              else
                sum = OptReal(value);
            }
          }

          if (sum.isSet())
            node->setValue(sum);
        }
      }

      // propagate set node value up to source nodes
      if (node->hasValue()) {
        if (! node->srcEdges().empty()) {
          //int ns = node->srcEdges().size();

          double value = node->value().realOr(1.0);

          for (auto &srcEdge : node->srcEdges()) {
            if (! srcEdge->hasValue())
              srcEdge->setValue(OptReal(value));

            auto *srcNode = srcEdge->srcNode();

            for (const auto &edge : srcNode->destEdges()) {
              if (edge->destNode() == node)
                edge->setValue(OptReal(value));
            }
          }
        }
      }
    }
  }
}

void
CQChartsSankeyPlot::
propagateHierValues()
{
  for (const auto &pn : nameNodeMap_) {
    auto *node = pn.second;

    if (node->value().isSet())
      continue;

    auto value = calcHierValue(node);

    node->setValue(OptReal(value));
  }
}

double
CQChartsSankeyPlot::
calcHierValue(Node *node) const
{
  // TODO: catch loops ?
  double hierValue = 0.0;

  for (auto *edge : node->destEdges()) {
    auto *destNode = edge->destNode();

    if (destNode->value().isSet())
      hierValue += destNode->value().real();
    else
      hierValue += calcHierValue(destNode);
  }

  return hierValue;
}

//---

bool
CQChartsSankeyPlot::
initFromToObjs() const
{
  CQPerfTrace trace("CQChartsSankeyPlot::initFromToObjs");

  return CQChartsConnectionPlot::initFromToObjs();
}

void
CQChartsSankeyPlot::
addFromToValue(const FromToData &fromToData) const
{
  // get src node
  auto *srcNode = findNode(fromToData.fromStr);

  if (fromToData.depth >= 0)
    srcNode->setDepth(fromToData.depth);

  //---

  // set group
  if (fromToData.groupData.ng > 1)
    srcNode->setGroup(fromToData.groupData.ig, fromToData.groupData.ng);
  else
    srcNode->setGroup(-1);

  //---

  // get node image
  auto *fromDetails = columnDetails(fromColumn());

  CQChartsImage fromImage;

  if (fromDetails && fromDetails->namedImage(fromToData.fromStr, fromImage) && fromImage.isValid())
    srcNode->setImage(fromImage);

  //---

  // Just node
  if (fromToData.toStr == "") {
    // set node color (if color column specified)
    Color c;

    if (colorColumnColor(fromToData.fromModelInd.row(), fromToData.fromModelInd.parent(), c))
      srcNode->setFillColor(c);

    //---

    // set node name values (attribute column)
    processNodeNameValues(srcNode, fromToData.nameValues);
  }
  else {
    // ignore self connection (TODO: allow)
    if (fromToData.fromStr == fromToData.toStr)
      return;

    auto *destNode = findNode(fromToData.toStr);

    if (fromToData.depth >= 0)
      destNode->setDepth(fromToData.depth + 1);

    //---

    // get node image
    auto *toDetails = columnDetails(toColumn());

    CQChartsImage toImage;

    if (toDetails && toDetails->namedImage(fromToData.toStr, toImage) && toImage.isValid())
      destNode->setImage(toImage);

    //---

    // create edge and link src/dest nodes
    auto *edge = createEdge(fromToData.value, srcNode, destNode);

    srcNode ->addDestEdge(edge, /*primary*/true );
    destNode->addSrcEdge (edge, /*primary*/false);

    //---

    // add model indices
    auto addModelInd = [&](const ModelIndex &modelInd) {
      if (modelInd.isValid())
        edge->addModelInd(modelInd);
    };

    auto fromModelIndex  = modelIndex(fromToData.fromModelInd);
    auto fromModelIndex1 = normalizeIndex(fromModelIndex);

    edge->setModelInd(fromModelIndex1);

    addModelInd(fromToData.fromModelInd  );
    addModelInd(fromToData.toModelInd    );
    addModelInd(fromToData.valueModelInd );
    addModelInd(fromToData.depthModelInd );
#ifdef CQCHARTS_GRAPH_PATH_ID
    addModelInd(fromToData.pathIdModelInd);
#endif

    edge->setNamedColumn("From"  , fromToData.fromModelInd  .column());
    edge->setNamedColumn("To"    , fromToData.toModelInd    .column());
    edge->setNamedColumn("Value" , fromToData.valueModelInd .column());
    edge->setNamedColumn("Depth" , fromToData.depthModelInd .column());
#ifdef CQCHARTS_GRAPH_PATH_ID
    edge->setNamedColumn("PathId", fromToData.pathIdModelInd.column());
#endif

    //---

#ifdef CQCHARTS_GRAPH_PATH_ID
    // set path id if column specified
    if (fromToData.pathId.isSet()) {
      int pathId = fromToData.pathId.integer();

      edge->setPathId(pathId);

      pathIdMinMax_.add(pathId);
    }
#endif

    //---

    // set edge color (if color column specified)
    // Note: from and to are same row so we can use either
    Color c;

    if (colorColumnColor(fromToData.fromModelInd.row(), fromToData.fromModelInd.parent(), c))
      edge->setFillColor(c);

    //---

    // set edge name values (attribute column)
    processEdgeNameValues(edge, fromToData.nameValues);
  }
}

//---

bool
CQChartsSankeyPlot::
initLinkObjs() const
{
  CQPerfTrace trace("CQChartsSankeyPlot::initLinkObjs");

  if (! CQChartsConnectionPlot::initLinkObjs())
    return false;

  if (isPropagate()) {
    auto *th = const_cast<CQChartsSankeyPlot *>(this);

    th->propagateLinkValues();
  }

  return true;
}

void
CQChartsSankeyPlot::
addLinkConnection(const LinkConnectionData &linkConnectionData) const
{
  // get src/dest nodes (TODO: allow single source node)
  auto *srcNode  = findNode(linkConnectionData.srcStr);
  auto *destNode = findNode(linkConnectionData.destStr);
//assert(srcNode != destNode);

  //---

  // create edge and link src/dest nodes
  auto *edge = createEdge(linkConnectionData.value, srcNode, destNode);

  srcNode ->addDestEdge(edge);
  destNode->addSrcEdge (edge);

  //---

  // add model indices
  auto addModelInd = [&](const ModelIndex &modelInd) {
    if (modelInd.isValid())
      edge->addModelInd(modelInd);
  };

  addModelInd(linkConnectionData.groupModelInd);
  addModelInd(linkConnectionData.linkModelInd );
  addModelInd(linkConnectionData.valueModelInd);
  addModelInd(linkConnectionData.nameModelInd );
  addModelInd(linkConnectionData.depthModelInd);

  //---

  // set edge color (if color column specified)
  Color c;

  if (colorColumnColor(linkConnectionData.linkModelInd.row(),
                       linkConnectionData.linkModelInd.parent(), c))
    edge->setFillColor(c);

  //---

  if (linkConnectionData.value.isSet())
    valueRange_.add(linkConnectionData.value.real());

#if 0
  // set value on dest node (NEEDED ?)
  destNode->setValue(linkConnectionData.value);
#endif

  //---

  // set group
  if (linkConnectionData.groupData.isValid())
    srcNode->setGroup(linkConnectionData.groupData.ig, linkConnectionData.groupData.ng);
  else
    srcNode->setGroup(-1);

  //---

  // set name index
  if (linkConnectionData.nameModelInd.isValid()) {
    auto nameModelInd1 = normalizeIndex(linkConnectionData.nameModelInd);

    srcNode->setInd(nameModelInd1);
  }

  //---

  // set depth
  if (linkConnectionData.depth > 0) {
    srcNode ->setDepth(linkConnectionData.depth);
    destNode->setDepth(linkConnectionData.depth + 1);
  }

  //---

  // set edge name values (attribute column)
  processEdgeNameValues(edge, linkConnectionData.nameValues);
}

void
CQChartsSankeyPlot::
propagateLinkValues()
{
  for (const auto &pn : nameNodeMap_) {
    auto *node = pn.second;

    auto sum = node->edgeSum();

    node->setValue(OptReal(sum));
  }
}

//---

bool
CQChartsSankeyPlot::
initConnectionObjs() const
{
  CQPerfTrace trace("CQChartsSankeyPlot::initConnectionObjs");

  return CQChartsConnectionPlot::initConnectionObjs();
}

void
CQChartsSankeyPlot::
addConnectionObj(int id, const ConnectionsData &connectionsData, const NodeIndex &) const
{
  // get src node
  auto srcStr = QString::number(id);

  auto *srcNode = findNode(srcStr);

  srcNode->setName(connectionsData.name);

  //---

  // set group
  if (connectionsData.groupData.isValid())
    srcNode->setGroup(connectionsData.groupData.ig, connectionsData.groupData.ng);
  else
    srcNode->setGroup(-1);

  //---

  // add model indices
  auto addModelInd = [&](const ModelIndex &modelInd) {
    if (modelInd.isValid())
      srcNode->addModelInd(modelInd);
  };

  addModelInd(connectionsData.nodeModelInd);
  addModelInd(connectionsData.connectionsModelInd);
  addModelInd(connectionsData.nameModelInd);
  addModelInd(connectionsData.groupModelInd);

  //---

  // set node name values (attribute column)
  processNodeNameValues(srcNode, connectionsData.nameValues);

  //---

  // process connections
  for (const auto &connection : connectionsData.connections) {
    // get destination node
    auto destStr = QString::number(connection.node);

    auto *destNode = findNode(destStr);

    //---

    // create edge and link src/dest nodes
    auto *edge = createEdge(OptReal(connection.value), srcNode, destNode);

    srcNode ->addDestEdge(edge);
    destNode->addSrcEdge (edge);

    //---

    // set value
    valueRange_.add(connection.value);

    destNode->setValue(OptReal(connection.value));
  }
}

//---

bool
CQChartsSankeyPlot::
initTableObjs() const
{
  CQPerfTrace trace("CQChartsSankeyPlot::initTableObjs");

  //---

  TableConnectionDatas tableConnectionDatas;
  TableConnectionInfo  tableConnectionInfo;

  if (! processTableModel(tableConnectionDatas, tableConnectionInfo))
    return false;

  //---

  auto nv = tableConnectionDatas.size();

  for (size_t row = 0; row < nv; ++row) {
    const auto &tableConnectionData = tableConnectionDatas[row];

    if (tableConnectionData.values().empty())
      continue;

    auto srcStr = QString::number(tableConnectionData.from());

    auto *srcNode = findNode(srcStr);

    srcNode->setName (tableConnectionData.name());
    srcNode->setGroup(tableConnectionData.group().ig, tableConnectionData.group().ng);

    for (const auto &value : tableConnectionData.values()) {
      auto destStr = QString::number(value.to);

      auto *destNode = findNode(destStr);

      // create edge and link src/dest nodes
      auto *edge = createEdge(OptReal(value.value), srcNode, destNode);

      srcNode ->addDestEdge(edge);
      destNode->addSrcEdge (edge);
    }
  }

  //---

  return true;
}

//----

bool
CQChartsSankeyPlot::
processMetaNodeValue(const QString &name, const QString &key, const QVariant &value)
{
  auto *node = findNode(name);

  return processNodeNameVar(node, key, value);
}

void
CQChartsSankeyPlot::
processNodeNameValues(Node *node, const NameValues &nameValues) const
{
  for (const auto &nv : nameValues.nameValues()) {
    const auto &name  = nv.first;
    auto        value = nv.second;

    processNodeNameValue(node, name, value);
  }
}

void
CQChartsSankeyPlot::
processNodeNameValue(Node *node, const QString &name, const QVariant &value) const
{
  if (! processNodeNameVar(node, name, value)) {
    auto *th = const_cast<CQChartsSankeyPlot *>(this);

    th->addDataError(ModelIndex(), QString("Unhandled name '%1'").arg(name));
  }
}

bool
CQChartsSankeyPlot::
processNodeNameVar(Node *node, const QString &name, const QVariant &var) const
{
  bool ok;

  // custom label
  if      (name == "label") {
    node->setLabel(var.toString());
  }
  // custom value
  else if (name == "value") {
    auto r = CQChartsVariant::toReal(var, ok); if (! ok) return false;
    node->setValue(OptReal(r));

    valueRange_.add(r);
  }
  // custom fill color
  else if (name == "fill_color" || name == "color") {
    auto c = CQChartsVariant::toColor(var, ok); if (! ok) return false;
    node->setFillColor(c);
  }
  // custom fill alpha
  else if (name == "fill_alpha" || name == "alpha") {
    auto a = CQChartsVariant::toAlpha(var, ok); if (! ok) return false;
    node->setFillAlpha(a);
  }
  // custom fill pattern
  else if (name == "fill_pattern" || name == "pattern") {
    node->setFillPattern(CQChartsFillPattern(var.toString()));
  }
  // custom stroke color
  else if (name == "stroke_color") {
    auto c = CQChartsVariant::toColor(var, ok); if (! ok) return false;
    node->setStrokeColor(c);
  }
  // custom stroke alpha
  else if (name == "stroke_alpha") {
    auto a = CQChartsVariant::toAlpha(var, ok); if (! ok) return false;
    node->setStrokeAlpha(a);
  }
  // custom stroke width
  else if (name == "stroke_width" || name == "width") {
    auto w = CQChartsVariant::toLength(var, ok); if (! ok) return false;
    node->setStrokeWidth(w);
  }
  // custom stroke dash
  else if (name == "stroke_dash" || name == "dash") {
    node->setStrokeDash(CQChartsLineDash(var.toString()));
  }
  else
    return false;

  return true;
}

void
CQChartsSankeyPlot::
processEdgeNameValues(Edge *edge, const NameValues &nameValues) const
{
  auto *srcNode  = edge->srcNode ();
  auto *destNode = edge->destNode();

  for (const auto &nv : nameValues.nameValues()) {
    const auto &name = nv.first;
    auto        var  = nv.second;

    bool ok;

    // custom fill color
    if      (name == "fill_color" || name == "color") {
      auto c = CQChartsVariant::toColor(var, ok); if (! ok) continue;
      edge->setFillColor(c);
    }
#if 0
    else if (name == "label") {
      edge->setLabel(var.toString());
    }
    else if (name == "value") {
      auto r = CQChartsVariant::toReal(var, ok); if (! ok) continue;
      edge->setValue(OptReal(r));
    }
#endif
#ifdef CQCHARTS_GRAPH_PATH_ID
    // TODO: remove path_id and color (use columns)
    else if (name == "path_id") {
      long pathId = CQChartsUtil::toInt(var, ok); if (! ok || pathId < 0) continue;
      edge->setPathId(int(pathId));
      pathIdMinMax_.add(int(pathId));
    }
#endif
    // handle custom value for source node
    else if (name.left(4) == "src_") {
      processNodeNameValue(srcNode, name.mid(4), var);
    }
    // handle custom value for destination node
    else if (name.left(5) == "dest_") {
      processNodeNameValue(destNode, name.mid(5), var);
    }
  }
}

//---

void
CQChartsSankeyPlot::
filterObjs()
{
  // hide nodes below depth
  if (maxDepth() > 0) {
    for (const auto &p : nameNodeMap_) {
      auto *node = p.second;

      if (node->depth() > maxDepth())
        node->setVisible(false);
    }
  }

  // hide nodes less than min value
  if (minValue() > 0) {
    for (const auto &p : nameNodeMap_) {
      auto *node = p.second;

      if (! node->value().isSet() || node->value().real() < minValue())
        node->setVisible(false);
    }
  }
}

//---

bool
CQChartsSankeyPlot::
addMenuItems(QMenu *menu, const Point &)
{
  bool added = false;

  if (canDrawColorMapKey()) {
    addColorMapKeyItems(menu);

    added = true;
  }

  return added;
}

//---

// main entry point in creating objects from model data
void
CQChartsSankeyPlot::
createObjsGraph(PlotObjs &objs) const
{
  // create graph
  createGraph();

  //---

  // place graph
  placeGraph(/*placed*/false);

#ifdef CQCHARTS_GRAPH_PATH_ID
  // adjust rects to match path id
  adjustPathIdSrcDestRects();
#endif

  //---

  // re-place graph using placed edges
  placeGraph(/*placed*/true);

#ifdef CQCHARTS_GRAPH_PATH_ID
  // adjust rects to match path id
  adjustPathIdSrcDestRects();
#endif

  //---

  // add objects to plot
  addObjects(objs);
}

void
CQChartsSankeyPlot::
addObjects(PlotObjs &objs) const
{
  assert(graph_);

  if (graph_->nodes().empty() || ! graph_->rect().isSet())
    return;

  // if graph rectangle invalid (zero width/height) then adjust to valid rect
  if (! graph_->rect().isValid()) {
    auto rect = graph_->rect();

    auto *th = const_cast<CQChartsSankeyPlot *>(this);

    if (rect.getWidth() <= 0) {
      double cx = rect.getXMid();

      rect.setXMin(cx - 1.0);
      rect.setXMax(cx + 1.0);
    }

    if (rect.getHeight() <= 0) {
      double cy = rect.getYMid();

      rect.setYMin(cy - 1.0);
      rect.setYMax(cy + 1.0);
    }

    th->graph_->setRect(rect);
  }

  //---

  // add node objects
  for (auto *node : graph_->nodes()) {
    auto *nodeObj = createObjFromNode(node);

    objs.push_back(nodeObj);
  }

  //---

  // add edge objects
  for (const auto &edge : edges_) {
    if (! edge->srcNode()->isVisible() || ! edge->destNode()->isVisible())
      continue;

    auto *edgeObj = addEdgeObj(edge);

    objs.push_back(edgeObj);
  }
}

// place nodes in graph
void
CQChartsSankeyPlot::
placeGraph(bool placed) const
{
  assert(graph_);

  // get placeable nodes (nodes and sub graphs)
  auto nodes = graph_->placeableNodes();

  placeGraphNodes(nodes, placed);
}

void
CQChartsSankeyPlot::
placeGraphNodes(const Nodes &nodes, bool placed) const
{
  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  //---

  // set max depth of all graph nodes
  updateGraphMaxDepth(nodes);

  //---

  // set pos of nodes
  calcGraphNodesPos(nodes);

  //---

#ifdef CQCHARTS_GRAPH_PATH_ID
  // sort x nodes by associated path ids
  if (hasAnyPathId())
    th->sortPathIdDepthNodes();
#endif

  //---

  // calc max height (fron node count) and max size (from value) for each x
  graph_->setMaxHeight(0);
  graph_->setTotalSize(0.0);

  for (const auto &depthNodes : graph_->depthNodesMap()) {
    const auto &nodes = depthNodes.second.nodes;
    double      size  = depthNodes.second.size;

    graph_->setMaxHeight(std::max(graph_->maxHeight(), int(nodes.size())));
    graph_->setTotalSize(std::max(graph_->totalSize(), size));
  }

  //---

  // calc perp value scale and margins to fit in bbox
  th->calcValueMarginScale();

  //---

  // place node objects at each depth (position)
  placeDepthNodes();

  //---

  // adjust nodes in graph
  adjustGraphNodes(nodes, placed);

  //---

  if (this->spread() == Spread::NONE)
    (void) fitToBBox();

  //---

  graph_->updateRect();
}

void
CQChartsSankeyPlot::
placeEdges() const
{
  for (auto *node : graph_->nodes())
    node->placeEdges(/*reset*/true);
}

void
CQChartsSankeyPlot::
calcGraphNodesPos(const Nodes &nodes) const
{
  graph_->clearDepthNodesMap();

  // set max depth of all graph nodes
  updateGraphMaxDepth(nodes);

  // place graph nodes at each position
  for (const auto &node : nodes) {
    int pos = calcPos(node);

    graph_->addDepthSize(pos, node->edgeSum());
    graph_->addDepthNode(pos, node);
  }

  //----

  // check if all nodes at single x
  if (graph_->depthNodesMap().size() == 1 && align() != Align::RAND) {
    auto *th = const_cast<CQChartsSankeyPlot *>(this);

    // TODO: stable pos (spiral/circle/grid ?)
    th->align_     = Align::RAND;
    th->alignRand_ = std::max(CMathRound::RoundNearest(sqrt(double(nodes.size()))), 2);

    for (const auto &node : nodes)
      node->setDepth(-1);

    calcGraphNodesPos(nodes);
  }
}

#ifdef CQCHARTS_GRAPH_PATH_ID
void
CQChartsSankeyPlot::
sortPathIdDepthNodes(bool force)
{
  if (! force && ! isSortPathIdNodes())
    return;

  using PathIdNodes = std::map<int, Nodes>;

  for (auto &depthNodes : graph_->depthNodesMap()) {
    PathIdNodes pathIdNodes;

    for (auto &node : depthNodes.second.nodes)
      pathIdNodes[-node->minPathId()].push_back(node);

    Nodes nodes;

    for (const auto &pn : pathIdNodes)
      for (const auto &node : pn.second)
        nodes.push_back(node);

    depthNodes.second.nodes = nodes;
  }
}
#endif

#ifdef CQCHARTS_GRAPH_PATH_ID
void
CQChartsSankeyPlot::
adjustPathIdSrcDestRects() const
{
  if (! hasAnyPathId())
    return;

  // place edges (reset)
  placeEdges();

  // adjust node rects to align on matching path id
  for (auto *node : graph_->nodes())
    node->adjustPathIdSrcDestRects();
}
#endif

void
CQChartsSankeyPlot::
createGraph() const
{
  if (! graph_) {
    auto *th = const_cast<CQChartsSankeyPlot *>(this);

    th->graph_ = new Graph(this);
  }

  assert(graph_);

  // Add nodes to graph for group
  for (const auto &idNode : indNodeMap_) {
    auto *node = idNode.second;
    if (! node->isVisible()) continue;

    graph_->addNode(node);
  }
}

void
CQChartsSankeyPlot::
clearGraph()
{
  delete graph_;

  graph_ = nullptr;
}

void
CQChartsSankeyPlot::
calcValueMarginScale()
{
  // get node margin (in window coords ?)
  double nodeMargin = calcNodeMargin();

  //---

  double boxSize = 2.0; // default size of bbox

  double boxSize1 = nodeMargin*boxSize;
  double boxSize2 = boxSize - boxSize1; // size minus margin

  //---

  // calc value margin (per node)/scale (to fit nodes in box)
  graph_->setValueMargin(graph_->maxHeight() > 1.0 ? boxSize1/(graph_->maxHeight() - 1) : 0.0);
  graph_->setValueScale (graph_->totalSize() > 0.0 ? boxSize2/ graph_->totalSize()      : 1.0);
}

double
CQChartsSankeyPlot::
calcNodeMargin() const
{
  double nodeMargin = lengthPlotPerpSize(this->nodeMargin(), isHorizontal());

  nodeMargin = std::min(std::max(nodeMargin, 0.0), 1.0);

  // get pixel margin perp to position axis
  auto pixelNodeMargin = windowToPixelPerpSize(nodeMargin, isHorizontal());

  // stop margin from being too small
  if (pixelNodeMargin < minNodeMargin())
    nodeMargin = pixelToWindowPerpSize(minNodeMargin(), isHorizontal());

  return nodeMargin;
}

void
CQChartsSankeyPlot::
placeDepthNodes() const
{
  // place node objects at each depth (pos)
  for (const auto &depthNodes : graph_->depthNodesMap()) {
    int         pos   = depthNodes.first;
    const auto &nodes = depthNodes.second.nodes;

    placeDepthSubNodes(pos, nodes);
  }
}

void
CQChartsSankeyPlot::
placeDepthSubNodes(int pos, const Nodes &nodes) const
{
  auto bbox = targetBBox_;

  // place nodes to fit in bbox (perp to position axis)
  double boxSize = bbox.getPerpSize(isHorizontal());

  int minPos = this->minPos();
  int maxPos = this->maxPos();

  auto posMargin = lengthPlotSize(nodeWidth(), isHorizontal());

  //---

  // get sum of perp margins nodes at depth
  double perpSize = graph_->valueMargin()*(int(nodes.size()) - 1);

  // get sum of scaled values for nodes at depth
  for (const auto &node : nodes)
    perpSize += graph_->valueScale()*node->edgeSum();

  // TODO: assert matches bbox perp size (with tolerance)

  //---

  // calc perp top (placing top to bottom)
  double perpPos1;

  if (isHorizontal())
    perpPos1 = bbox.getYMax() - (boxSize - perpSize)/2.0;
  else
    perpPos1 = bbox.getXMax() - (boxSize - perpSize)/2.0;

  //---

  for (const auto &node : nodes) {
    // calc perp node size
    double nodePerpSize = graph_->valueScale()*node->edgeSum();

    if (nodePerpSize <= 0.0)
      nodePerpSize = 0.1;

    //---

    // calc rect
    int pos1 = calcPos(node);
    assert(pos == pos1);

    double nodePerpMid = perpPos1 - nodePerpSize/2.0; // placement center

    double nodePerpPos1 = nodePerpMid - nodePerpSize/2.0; // perpPos1 - nodePerpSize
    double nodePerpPos2 = nodePerpMid + nodePerpSize/2.0; // perpPos1

    //---

    // calc bbox (adjust align for first left edge (minPos) or right edge (maxPos))
    BBox rect;

    double posStart, posEnd;

    if (isNodeValueBar()) {
      posStart = targetBBox_.getMinExtent(isHorizontal());
      posEnd   = targetBBox_.getMaxExtent(isHorizontal());
    }
    else {
      // map pos to bbox range minus margins
      if (isAlignEnds()) {
        posStart = targetBBox_.getMinExtent(isHorizontal()) + posMargin/2.0;
        posEnd   = targetBBox_.getMaxExtent(isHorizontal()) - posMargin/2.0;
      }
      // map pos to bbox range (use for left)
      else {
        posStart = targetBBox_.getMinExtent(isHorizontal());
        posEnd   = targetBBox_.getMaxExtent(isHorizontal());
      }
    }

    double nodePos1 = CMathUtil::map(pos1, minPos, maxPos, posStart, posEnd);

    if (isNodeValueBar()) {
      double nodePos2 = CMathUtil::map(pos1 + 1, minPos, maxPos, posStart, posEnd);

      auto minSize = lengthPlotSize(nodeMinWidth(), isHorizontal());

      auto size = 0.0;

      if (node->value().isSet()) {
        auto scale = valueRange_.map(node->value().real(), 0.0, 1.0);

        auto barSize = std::min(std::max(nodeValueBarSize(), 0.0), 1.0);

        auto maxSize = std::max(barSize*(nodePos2 - nodePos1), minSize);

        size = CMathUtil::map(scale, 0.0, 1.0, minSize, maxSize);
      }
      else
        size = minSize;

      if (isHorizontal())
        rect = BBox(nodePos1, nodePerpPos1, nodePos1 + size, nodePerpPos2);
      else
        rect = BBox(nodePerpPos1, nodePos1, nodePerpPos2, nodePos1 + size);
    }
    else {
      // center align
      if (isHorizontal())
        rect = BBox(nodePos1 - posMargin/2.0, nodePerpPos1,
                    nodePos1 + posMargin/2.0, nodePerpPos2);
      else
        rect = BBox(nodePerpPos1, nodePos1 - posMargin/2.0,
                    nodePerpPos2, nodePos1 + posMargin/2.0);
    }

    //---

    node->setRect(rect);

    //---

    perpPos1 -= nodePerpSize + graph_->valueMargin();
  }
}

CQChartsSankeyPlot::NodeObj *
CQChartsSankeyPlot::
createObjFromNode(Node *node) const
{
  int numNodes = int(indNodeMap_.size());

  ColorInd ig;

  if (node->ngroup() > 0 && node->group() >= 0 && node->group() < node->ngroup())
    ig = ColorInd(node->group(), node->ngroup());

  ColorInd iv(node->id(), numNodes);

  auto *nodeObj = createNodeObj(node->rect(), node, ig, iv);

  nodeObj->connectDataChanged(this, SLOT(updateSlot()));

  nodeObj->setHierName(node->str());

  // add model indices
  for (const auto &modelInd : node->modelInds())
    nodeObj->addModelInd(normalizedModelIndex(modelInd));

  node->setObj(nodeObj);

  nodeObj->setSelected(node->isSelected());

  return nodeObj;
}

int
CQChartsSankeyPlot::
calcPos(Node *node) const
{
  int pos = 0;

  if (node->depth() >= 0) {
    pos = node->depth();
  }
  else {
    int maxNodeDepth = graph_->maxNodeDepth();

    int srcDepth  = node->srcDepth (); // min depth of previous nodes
    int destDepth = node->destDepth(); // max depth of subsequent nodes

    if      (srcDepth == 0 && align() != Align::SRC_ALL && align() != Align::DEST_ALL) {
      if (isAlignFirstLast()) {
        if (! node->destEdges().empty()) {
          int minDest = node->destDepth();

          for (const auto &edge : node->destEdges()) {
            auto *node1 = edge->destNode();

            minDest = std::min(minDest, calcPos(node1));
          }

          pos = std::max(minDest - 1, 0);
        }
        else
          pos = 0;
      }
      else
        pos = 0;
    }
    else if (destDepth == 0 && align() != Align::SRC_ALL && align() != Align::DEST_ALL) {
      if (isAlignFirstLast()) {
        if (! node->srcEdges().empty()) {
          int maxSrc = 0;

          for (const auto &edge : node->srcEdges()) {
            auto *node1 = edge->srcNode();

            maxSrc = std::max(maxSrc, calcPos(node1));
          }

          pos = std::min(maxSrc + 1, maxNodeDepth);
        }
        else
          pos = maxNodeDepth;
      }
      else
        pos = maxNodeDepth;
    }
    else {
      if      (align() == Align::SRC || align() == Align::SRC_ALL)
        pos = srcDepth;
      else if (align() == Align::DEST || align() == Align::DEST_ALL)
        pos = maxNodeDepth - destDepth;
      else if (align() == Align::JUSTIFY || align() == Align::LARGEST) {
        double f = 1.0*srcDepth/(srcDepth + destDepth);

        pos = int(f*maxNodeDepth);
      }
      else if (align() == Align::RAND) {
        CQChartsRand::RealInRange rand(0, alignRand_);

        pos = CMathRound::RoundNearest(rand.gen());
        assert(pos >= 0 && pos <= alignRand_);

        const_cast<Node *>(node)->setDepth(pos);
      }
    }
  }

  //--

  node->setPos(pos);

  return pos;
}

CQChartsSankeyEdgeObj *
CQChartsSankeyPlot::
addEdgeObj(Edge *edge) const
{
  auto bbox = targetBBox_;

  double xm = bbox.getHeight()*edgeMargin_;
  double ym = bbox.getWidth ()*edgeMargin_;

  BBox nodeRect;

  nodeRect += edge->srcNode ()->rect();
  nodeRect += edge->destNode()->rect();

  BBox rect(nodeRect.getXMin() - xm, nodeRect.getYMin() - ym,
            nodeRect.getXMax() + xm, nodeRect.getYMax() + ym);

  auto *edgeObj = createEdgeObj(rect, edge);

  edgeObj->connectDataChanged(this, SLOT(updateSlot()));

  // add model indices
  for (const auto &modelInd : edge->modelInds())
    edgeObj->addModelInd(normalizedModelIndex(modelInd));

  edge->setObj (edgeObj);
  edge->setLine(isEdgeLine());

  return edgeObj;
}

void
CQChartsSankeyPlot::
updateGraphMaxDepth(const Nodes &nodes) const
{
  // calc max depth (source or dest) depending on align for pos calc
  bool set = false;

  graph_->setMaxNodeDepth(0);

  auto updateNodeDepth = [&](int depth) {
    if (! set) {
      graph_->setMaxNodeDepth(depth);

      set = true;
    }
    else
      graph_->setMaxNodeDepth(std::max(graph_->maxNodeDepth(), depth));
  };

  for (const auto &node : nodes) {
    int srcDepth  = node->srcDepth ();
    int destDepth = node->destDepth();

    if      (align() == Align::SRC || align() == Align::SRC_ALL)
      updateNodeDepth(srcDepth);
    else if (align() == Align::DEST || align() == Align::DEST_ALL)
      updateNodeDepth(destDepth);
    else {
      updateNodeDepth(srcDepth);
      updateNodeDepth(destDepth);
    }
  }
}

bool
CQChartsSankeyPlot::
adjustNodes(bool placed, bool force) const
{
  bool changed = false;

  if (graph_) {
    auto nodes = graph_->placeableNodes();

    if (adjustGraphNodes(nodes, placed, force))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
adjustGraphNodes(const Nodes &nodes, bool placed, bool force) const
{
  if (! force && ! isAdjustNodes())
    return false;

  initPosNodesMap(nodes);

  //---

  int numPasses = adjustIterations();

  for (int pass = 0; pass < numPasses; ++pass) {
    if (! adjustNodeCenters(placed))
      break;
  }

  //---

  bool spread    = true;
  bool constrain = (this->spread() != Spread::NONE);

  (void) removeOverlaps(spread, constrain, force);

  //---

  reorderNodeEdges(nodes);

  //---

#if 0
  adjustEdgeOverlaps();
#endif

  return true;
}

void
CQChartsSankeyPlot::
initPosNodesMap(const Nodes &nodes) const
{
  // get nodes by pos
  graph_->resetPosNodes();

  for (const auto &node : nodes)
    graph_->addPosNode(node);
}

bool
CQChartsSankeyPlot::
adjustNodeCenters(bool placed, bool force) const
{
  if (! force && ! isAdjustCenters())
    return false;

  bool changed = false;

  if (placed)
    placeEdges(); /* reset */

  if (align() == Align::LARGEST) {
    int minPos = this->minPos();
    int maxPos = this->maxPos();

    int maxN       = 0;
    int maxNodePos = minPos;

    for (int pos = minPos; pos <= maxPos; ++pos) {
      if (! graph_->hasPosNodes(pos)) continue;

      const auto &nodes = graph_->posNodes(pos);

      if (int(nodes.size()) > maxN) {
        maxN       = int(nodes.size());
        maxNodePos = pos;
      }
    }

    for (int pos = maxNodePos - 1; pos >= minPos; --pos) {
      if (adjustPosNodes(pos, placed, /*useSrc*/false, /*useDest*/true))
        changed = true;
    }

    for (int pos = maxNodePos + 1; pos <= maxPos; ++pos) {
      if (adjustPosNodes(pos, placed, /*useSrc*/true, /*useDest*/false))
        changed = true;
    }

    return changed;
  }
  else {
    if (adjustNodeCentersLtoR(placed, force))
      changed = true;

    if (adjustNodeCentersRtoL(placed, force))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
adjustNodeCentersLtoR(bool placed, bool force) const
{
  if (! force && ! isAdjustCenters())
    return false;

  if (align() == Align::DEST || align() == Align::DEST_ALL)
    return false;

  // adjust nodes so centered on src nodes
  bool changed = false;

  int minPos = this->minPos();
  int maxPos = this->maxPos();

  // second to last minus one (last if SRC align)
  int startPos = minPos + 1;
  int endPos   = maxPos - 1;

  if (align() == Align::SRC || align() == Align::SRC_ALL)
    ++endPos;

  for (int pos = startPos; pos <= endPos; ++pos) {
    if (adjustPosNodes(pos, placed))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
adjustNodeCentersRtoL(bool placed, bool force) const
{
  if (! force && ! isAdjustCenters())
    return false;

  if (align() == Align::SRC || align() == Align::SRC_ALL)
    return false;

  // adjust nodes so centered on src nodes
  bool changed = false;

  int minPos = this->minPos();
  int maxPos = this->maxPos();

  // last minus one to second (first if DEST align)
  int startPos = minPos + 1;
  int endPos   = maxPos - 1;

  if (align() == Align::DEST || align() == Align::DEST_ALL)
    startPos = minPos;

  for (int pos = endPos; pos >= startPos; --pos) {
    if (adjustPosNodes(pos, placed))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
adjustPosNodes(int pos, bool placed, bool useSrc, bool useDest) const
{
  assert(useSrc || useDest);

  if (! graph_->hasPosNodes(pos))
    return false;

  bool changed = false;

  const auto &nodes = graph_->posNodes(pos);

  for (const auto &node : nodes) {
    if (adjustNode(node, placed, useSrc, useDest))
      changed = true;
  }

  return changed;
}

#if 0
bool
CQChartsSankeyPlot::
adjustEdgeOverlaps(bool force) const
{
  if (! force && ! isAdjustEdgeOverlaps())
    return false;

  //---

  auto hasNode = [&](Node *node, const Nodes &nodes) {
    for (auto &node1 : nodes)
      if (node == node1)
        return true;

    return false;
  };

  int posNodesDepth = int(graph_->posNodesMap().size());

  // find first pos with nodes
  int pos1 = 0;

  while (pos1 <= posNodesDepth && ! graph_->hasPosNodes(pos1))
    ++pos1;

  if (pos1 > posNodesDepth)
    return false;

  int pos2 = pos1 + 1;

  while (pos2 <= posNodesDepth) {
    // find next pos with nodes
    while (pos2 <= posNodesDepth && ! graph_->hasPosNodes(pos2))
      ++pos2;

    if (pos2 > posNodesDepth)
      break;

    // get nodes at each pos
    const auto &nodes1 = graph_->posNodes(pos1);
    const auto &nodes2 = graph_->posNodes(pos2);

    // get edges between nodes
    Edges edges;

    for (const auto &node1 : nodes1) {
      for (const auto &edge1 : node1->destEdges()) {
        auto *destNode1 = edge1->destNode();

        if (hasNode(destNode1, nodes2))
          edges.push_back(edge1);
      }
    }

    // check edges for overlaps
    int numEdges = int(edges.size());

    for (int i1 = 0; i1 < numEdges - 1; ++i1) {
      auto *edge1 = edges[i1];

      for (int i2 = i1 + 1; i2 < numEdges; ++i2) {
        auto *edge2 = edges[i2];

        if (edge1->overlaps(edge2)) {
          auto *srcObj1  = edge1->srcNode ();
          auto *destObj1 = edge1->destNode();
          auto *srcObj2  = edge2->srcNode ();
          auto *destObj2 = edge2->destNode();

          std::cerr << "Overlap Edges : " <<
            "[" << srcObj1 ->name().toStdString() << ", " <<
                   destObj1->name().toStdString() << "] " <<
            "[" << srcObj2 ->name().toStdString() << ", " <<
                   destObj2->name().toStdString() << "]\n";
        }
      }
    }

    pos1 = pos2++;
  }

  return true;
}
#endif

bool
CQChartsSankeyPlot::
removeOverlaps(bool spread, bool constrain, bool force) const
{
  if (! force && ! isRemoveOverlaps())
    return false;

  bool changed = false;

  auto nodes = graph_->placeableNodes();

  initPosNodesMap(nodes);

  for (const auto &posNodes : graph_->posNodesMap()) {
    if (removePosOverlaps(posNodes.first, posNodes.second, spread, constrain))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
removePosOverlaps(int pos, const Nodes &nodes, bool spread, bool constrain) const
{
  auto perpMargin = pixelToWindowPerpSize(minNodeMargin(), isHorizontal());

  //---

  auto removeNodesOverlaps = [&](bool increasing) {
    // get nodes sorted by perp pos (min->max or max->min)
    PosNodeMap posNodeMap;

    createPosNodeMap(pos, nodes, posNodeMap, increasing);

#if 0
    std::cerr << "CQChartsSankeyPlot::removePosOverlaps " << pos << " " << increasing << "\n";
    for (const auto &posNode : posNodeMap) {
      std::cerr << posNode.first.y << " " << posNode.first.pathId << " " <<
                   posNode.second->name().toStdString() << std::endl;
    }
#endif

    //---

    // remove overlaps between nodes
    bool changed = false;

    Node *node1 = nullptr;

    for (const auto &posNode : posNodeMap) {
      auto *node2 = posNode.second;

      if (node1) {
        const auto &rect1 = node1->rect();
        const auto &rect2 = node2->rect();

        if (increasing) {
          if (isHorizontal()) {
            // if node2 overlaps node1 (not above) then move up
            if (rect2.getYMin() <= rect1.getYMax() + perpMargin) {
              double dy = rect1.getYMax() + perpMargin - rect2.getYMin();

              if (std::abs(dy) > 1E-6) {
                node2->moveBy(Point(0.0, dy));
                changed = true;
              }
            }
          }
          else {
            // if node2 overlaps node1 (not right) then move left
            if (rect2.getXMin() <= rect1.getXMax() + perpMargin) {
              double dx = rect1.getXMax() + perpMargin - rect2.getXMin();

              if (std::abs(dx) > 1E-6) {
                node2->moveBy(Point(dx, 0.0));
                changed = true;
              }
            }
          }
        }
        else {
          if (isHorizontal()) {
            // if node2 overlaps node1 (not below) then move down
            if (rect2.getYMax() >= rect1.getYMin() - perpMargin) {
              double dy = rect1.getYMin() - perpMargin - rect2.getYMax();

              if (std::abs(dy) > 1E-6) {
                node2->moveBy(Point(0.0, dy));
                changed = true;
              }
            }
          }
          else {
            // if node2 overlaps node1 (not left) then move right
            if (rect2.getXMax() >= rect1.getXMin() - perpMargin) {
              double dx = rect1.getXMin() - perpMargin - rect2.getXMax();

              if (std::abs(dx) > 1E-6) {
                node2->moveBy(Point(dx, 0.0));
                changed = true;
              }
            }
          }
        }
      }

      node1 = node2;
    }

    return changed;
  };

  //---

  bool spread1 = spread;
  bool center  = false;

  if (spread1) {
    if      (this->spread() == Spread::NONE) {
      spread1 = false;
      center  = true;
    }
    else if (this->spread() == Spread::FIRST)
      spread1 = (pos == minPos());
    else if (this->spread() == Spread::LAST)
      spread1 = (pos == maxPos());
    else if (this->spread() == Spread::FIRST_LAST)
      spread1 = (pos == minPos() || pos == maxPos());
  }

  //---

  bool changed = false;

  if (removeNodesOverlaps(/*increasing*/false))
    changed = true;

  // move nodes back inside bbox (needed ?)
  if (spread1) {
    if (spreadPosNodes(pos, nodes))
      changed = true;
  }

  if (constrain) {
    if (constrainPosNodes(pos, nodes, center))
      changed = true;
  }

  if (removeNodesOverlaps(/*increasing*/true))
    changed = true;

  // move nodes back inside bbox (needed ?)
  if (spread1) {
    if (spreadPosNodes(pos, nodes))
      changed = true;
  }

  if (constrain) {
    if (constrainPosNodes(pos, nodes, center))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
spreadNodes() const
{
  if (this->spread() == Spread::NONE)
    return false;

  bool changed = false;

  auto nodes = graph_->placeableNodes();

  initPosNodesMap(nodes);

  for (const auto &posNodes : graph_->posNodesMap()) {
    int pos = posNodes.first;

    if      (this->spread() == Spread::FIRST) {
      if (pos != minPos()) continue;
    }
    else if (this->spread() == Spread::LAST) {
      if (pos != maxPos()) continue;
    }
    else if (this->spread() == Spread::FIRST_LAST) {
      if (pos != minPos() && pos != maxPos()) continue;
    }

    if (spreadPosNodes(pos, posNodes.second))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
spreadPosNodes(int pos, const Nodes &nodes) const
{
  PosNodeMap posNodeMap;

  createPosNodeMap(pos, nodes, posNodeMap, /*increasing*/false);

  BBox spreadBBox;

  Node *node1 = nullptr, *node2 = nullptr;

  for (const auto &posNode : posNodeMap) {
    node2 = posNode.second;

    if (! node1) node1 = node2;

    spreadBBox += node2->rect();
  }

  if (! node1 || ! node2)
    return false;

  //---

  auto bbox = targetBBox_;

  if (isHorizontal()) {
    double dy1 = node1->rect().getHeight()/2.0; // top
    double dy2 = node2->rect().getHeight()/2.0; // bottom

    if (! spreadBBox.isValid() || (spreadBBox.getHeight() - dy1 - dy2) <= 0.0)
      return false;

    double ymin = bbox.getYMin() + dy2;
    double ymax = bbox.getYMax() - dy1;

    double dy      = ymin - node2->rect().getYMid();
    double boxSize = (ymax - ymin)/(spreadBBox.getHeight() - dy1 - dy2);

    if (CMathUtil::realEq(dy, 0.0) && CMathUtil::realEq(boxSize, 1.0))
      return false;

    for (const auto &posNode : posNodeMap) {
      auto *node = posNode.second;

      node->moveBy(Point(0.0, dy));

      double y1 = boxSize*(node->rect().getYMid() - ymin) + ymin;

      node->moveBy(Point(0.0, y1 - node->rect().getYMid()));
    }
  }
  else {
    double dx1 = node1->rect().getWidth()/2.0; // right
    double dx2 = node2->rect().getWidth()/2.0; // left

    if (! spreadBBox.isValid() || (spreadBBox.getWidth() - dx1 - dx2) <= 0.0)
      return false;

    double xmin = bbox.getXMin() + dx2;
    double xmax = bbox.getXMax() - dx1;

    double dx      = xmin - node2->rect().getXMid();
    double boxSize = (xmax - xmin)/(spreadBBox.getWidth() - dx1 - dx2);

    if (CMathUtil::realEq(dx, 0.0) && CMathUtil::realEq(boxSize, 1.0))
      return false;

    for (const auto &posNode : posNodeMap) {
      auto *node = posNode.second;

      node->moveBy(Point(dx, 0.0));

      double x1 = boxSize*(node->rect().getXMid() - xmin) + xmin;

      node->moveBy(Point(x1 - node->rect().getXMid(), 0.0));
    }
  }

  return true;
}

bool
CQChartsSankeyPlot::
constrainNodes(bool center) const
{
  bool changed = false;

  auto nodes = graph_->placeableNodes();

  initPosNodesMap(nodes);

  for (const auto &posNodes : graph_->posNodesMap()) {
    if (constrainPosNodes(posNodes.first, posNodes.second, center))
      changed = true;
  }

  return changed;
}

bool
CQChartsSankeyPlot::
constrainPosNodes(int pos, const Nodes &nodes, bool center) const
{
  PosNodeMap posNodeMap;

  createPosNodeMap(pos, nodes, posNodeMap, /*increasing*/false);

  BBox constrainBBox;

  Node *node1 = nullptr, *node2 = nullptr;

  for (const auto &posNode : posNodeMap) {
    node2 = posNode.second;

    if (! node1) node1 = node2;

    constrainBBox += node2->rect();
  }

  if (! node1 || ! node2)
    return false;

  auto bbox = targetBBox_;

  if (isHorizontal()) {
    double dy1 = node2->rect().getYMin() - bbox.getYMin();
    double dy2 = bbox.getYMax() - node1->rect().getYMax();

    double dy = 0.0;

    if (dy1 >= 0 && dy2 >= 0) {
      if (! center)
        return false;

      dy = bbox.getYMid() - constrainBBox.getYMid();
    }
    else {
      if      (dy1 < 0 && dy2 < 0) {
        dy = bbox.getYMid() - constrainBBox.getYMid();
      }
      else if (dy1 < 0) {
        dy = -dy1;
      }
      else {
        dy = dy2;
      }
    }

    if (CMathUtil::realEq(dy, 0.0))
      return false;

    for (const auto &posNode : posNodeMap) {
      auto *node = posNode.second;

      node->moveBy(Point(0.0, dy));
    }
  }
  else {
    double dx1 = node2->rect().getXMin() - bbox.getXMin();
    double dx2 = bbox.getXMax() - node1->rect().getXMax();

    double dx = 0.0;

    if (dx1 >= 0 && dx2 >= 0) {
      if (! center)
        return false;

      dx = bbox.getXMid() - constrainBBox.getXMid();
    }
    else {
      if      (dx1 < 0 && dx2 < 0) {
        dx = bbox.getXMid() - constrainBBox.getXMid();
      }
      else if (dx1 < 0) {
        dx = -dx1;
      }
      else {
        dx = dx2;
      }
    }

    if (CMathUtil::realEq(dx, 0.0))
      return false;

    for (const auto &posNode : posNodeMap) {
      auto *node = posNode.second;

      node->moveBy(Point(dx, 0.0));
    }
  }

  return true;
}

bool
CQChartsSankeyPlot::
reorderNodeEdges(const Nodes &nodes, bool force) const
{
  if (! force && ! isReorderEdges())
    return false;

  //---

  bool changed = false;

  // sort node edges nodes by bbox
  for (const auto &node : nodes) {
    PosEdgeMap srcPosEdgeMap;

    createPosEdgeMap(node->srcEdges(), srcPosEdgeMap, /*isSrc*/true);

    if (srcPosEdgeMap.size() > 1) {
      Edges srcEdges;

      for (const auto &srcPosNode : srcPosEdgeMap)
        srcEdges.push_back(srcPosNode.second);

      node->setSrcEdges(srcEdges);

      changed = true;
    }

    //---

    PosEdgeMap destPosEdgeMap;

    createPosEdgeMap(node->destEdges(), destPosEdgeMap, /*isSrc*/false);

    if (destPosEdgeMap.size() > 1) {
      Edges destEdges;

      for (const auto &destPosNode : destPosEdgeMap)
        destEdges.push_back(destPosNode.second);

      node->setDestEdges(destEdges);

      changed = true;
    }
  }

  return changed;
}

void
CQChartsSankeyPlot::
createPosNodeMap(int pos, const Nodes &nodes, PosNodeMap &posNodeMap, bool increasing) const
{
  assert(pos >= 0);

  auto bbox = targetBBox_;

  for (const auto &node : nodes) {
    if (! node->isVisible()) continue;

    const auto &rect = node->rect();
    if (! rect.isValid()) continue;

    double dist;

    // use distance from bottom/left (increasing) or distance from top/right (decreasing)
    if (isHorizontal())
      dist = (increasing ? rect.getYMid() - bbox.getYMin() : bbox.getYMax() - rect.getYMid());
    else
      dist = (increasing ? rect.getXMid() - bbox.getXMin() : bbox.getXMax() - rect.getXMid());

#ifdef CQCHARTS_GRAPH_PATH_ID
    NodePerpPos perpPos(dist, node->id(), increasing ? node->minPathId() : -node->minPathId());
#else
    NodePerpPos perpPos(dist, node->id());
#endif

    auto p = posNodeMap.find(perpPos);
    assert(p == posNodeMap.end());

    posNodeMap[perpPos] = node;
  }
}

void
CQChartsSankeyPlot::
createPosEdgeMap(const Edges &edges, PosEdgeMap &posEdgeMap, bool isSrc) const
{
  auto bbox = targetBBox_;

  for (const auto &edge : edges) {
    auto *node = (isSrc ? edge->srcNode() : edge->destNode());
    if (! node->isVisible()) continue;

    const auto &rect = node->rect();
    if (! rect.isValid()) continue;

    // use distance from top/right (decreasing)
    double dist;

    if (isHorizontal())
      dist = bbox.getYMax() - rect.getYMid();
    else
      dist = bbox.getXMax() - rect.getXMid();

#ifdef CQCHARTS_GRAPH_PATH_ID
    NodePerpPos perpPos(dist, edge->id(), -node->minPathId());
#else
    NodePerpPos perpPos(dist, edge->id());
#endif

    auto p = posEdgeMap.find(perpPos);
    assert(p == posEdgeMap.end());

    posEdgeMap[perpPos] = edge;
  }
}

bool
CQChartsSankeyPlot::
adjustNode(Node *node, bool placed, bool useSrc, bool useDest) const
{
  assert(useSrc || useDest);

  // get bounds of source edges
  BBox srcBBox;

  if (useSrc) {
    for (const auto &edge : node->srcEdges()) {
      if (edge->isSelf()) continue;

      auto *srcNode = edge->srcNode();
      if (! srcNode->isVisible()) continue;

      if (! placed)
        srcBBox += srcNode->rect();
      else {
        if (srcNode->hasDestEdgeRect(edge)) {
#ifdef CQCHARTS_GRAPH_PATH_ID
          srcNode->adjustPathIdSrcDestRects();
#endif

          //---

          auto srcRect = srcNode->destEdgeRect(edge);

          if (srcRect.isSet())
            srcBBox += srcRect;
          else
            srcBBox += srcNode->rect();
        }
        else
          srcBBox += srcNode->rect();
      }
    }
  }

  //---

  // get bounds of dest edges
  BBox destBBox;

  if (useDest) {
    for (const auto &edge : node->destEdges()) {
      if (edge->isSelf()) continue;

      auto *destNode = edge->destNode();
      if (! destNode->isVisible()) continue;

      if (! placed)
        destBBox += destNode->rect();
      else {
        if (destNode->hasSrcEdgeRect(edge)) {
#ifdef CQCHARTS_GRAPH_PATH_ID
          destNode->adjustPathIdSrcDestRects();
#endif

          //---

          auto destRect = destNode->srcEdgeRect(edge);

          if (destRect.isSet())
            destBBox += destRect;
          else
            destBBox += destNode->rect();
        }
        else
          destBBox += destNode->rect();
      }
    }
  }

  //---

  // calc average perp position
  double midPerpPos = 0.0;

  if (isHorizontal()) {
    if      (srcBBox.isValid() && destBBox.isValid())
      midPerpPos = CMathUtil::avg(srcBBox.getYMid(), destBBox.getYMid());
    else if (srcBBox.isValid())
      midPerpPos = srcBBox.getYMid();
    else if (destBBox.isValid())
      midPerpPos = destBBox.getYMid();
    else
      return false;
  }
  else {
    if      (srcBBox.isValid() && destBBox.isValid())
      midPerpPos = CMathUtil::avg(srcBBox.getXMid(), destBBox.getXMid());
    else if (srcBBox.isValid())
      midPerpPos = srcBBox.getXMid();
    else if (destBBox.isValid())
      midPerpPos = destBBox.getXMid();
    else
      return false;
  }

  //---

  // move node to average
  if (isHorizontal()) {
    double dy = midPerpPos - node->rect().getYMid();

    if (std::abs(dy) < 1E-6) // better tolerance ?
      return false;

    node->moveBy(Point(0.0, dy));
  }
  else {
    double dx = midPerpPos - node->rect().getXMid();

    if (std::abs(dx) < 1E-6) // better tolerance ?
      return false;

    node->moveBy(Point(dx, 0.0));
  }

  return true;
}

//---

CQChartsSankeyPlotNode *
CQChartsSankeyPlot::
findNode(const QString &name, bool create) const
{
  auto p = nameNodeMap_.find(name);

  if (p != nameNodeMap_.end())
    return (*p).second;

  if (! create)
    return nullptr;

  auto *node = createNode(name);

  return node;
}

CQChartsSankeyPlotNode *
CQChartsSankeyPlot::
createNode(const QString &name) const
{
  auto *node = new Node(this, name);

  node->setId(int(nameNodeMap_.size()));

  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  auto p1 = th->nameNodeMap_.emplace_hint(th->nameNodeMap_.end(), name, node);

  assert(node == (*p1).second);

  th->indNodeMap_[node->id()] = node;

  node->setName(name);

  return node;
}

CQChartsSankeyPlotEdge *
CQChartsSankeyPlot::
createEdge(const OptReal &value, Node *srcNode, Node *destNode) const
{
  assert(srcNode && destNode);

  auto *edge = new Edge(this, value, srcNode, destNode);

  auto *th = const_cast<CQChartsSankeyPlot *>(this);

  th->edges_.push_back(edge);

  edge->setId(int(th->edges_.size()));

  return edge;
}

//---

bool
CQChartsSankeyPlot::
keyPress(int key, int modifier)
{
  if      (key == Qt::Key_A) {
    if (adjustNodes(/*placed*/false, /*force*/true))
      drawObjs();
  }
  else if (key == Qt::Key_C) {
    if (adjustNodeCenters(/*placed*/false, /*force*/true))
      drawObjs();
  }
  else if (key == Qt::Key_L) {
    if (adjustNodeCentersLtoR(/*placed*/false, /*force*/true))
      drawObjs();
  }
  else if (key == Qt::Key_R) {
    if (adjustNodeCentersRtoL(/*placed*/false, /*force*/true))
      drawObjs();
  }
  else if (key == Qt::Key_O) {
    bool spread    = false;
    bool constrain = true;
    bool force     = true;

    if (removeOverlaps(spread, constrain, force))
      drawObjs();
  }
  else if (key == Qt::Key_I) {
    if (constrainNodes())
      drawObjs();
  }
  else if (key == Qt::Key_S) {
    if (spreadNodes())
      drawObjs();
  }
  else if (key == Qt::Key_F) {
    if (fitToBBox())
      drawObjs();
  }
  else if (key == Qt::Key_P) {
    placeGraph(/*placed*/false);

    placeEdges(); /* reset */

    drawObjs();
  }
  else if (key == Qt::Key_0) {
    printStats();
  }
#if 0
  else if (key == Qt::Key_V) {
    adjustEdgeOverlaps(/*force*/true);
  }
#endif
  else {
    return CQChartsPlot::keyPress(key, modifier);
  }

  return true;
}

void
CQChartsSankeyPlot::
printStats()
{
  using NameData = std::map<QString, QString>;
  using NodeData = std::map<Node *, NameData>;

  NodeData nodeData;

  for (const auto &idNode : indNodeMap_) {
    auto *node = idNode.second;
    if (! node->isVisible()) continue;

    if (node->srcEdges().empty())
      nodeData[node]["No Src" ] = "";

    if (node->destEdges().empty())
      nodeData[node]["No Dest"] = "";
  }

  for (auto &pn : nodeData) {
    auto       *node     = pn.first;
    const auto &nameData = pn.second;

    for (auto &pd : nameData) {
      std::cerr << node->name().toStdString() << " " << pd.first.toStdString() << "\n";
    }
  }
}

//---

void
CQChartsSankeyPlot::
addDrawText(PaintDevice *device, const QString &str, const Point &point,
            const TextOptions &textOptions, const Point &targetPoint,
            bool centered) const
{
  placer_->addDrawText(device, str, point, textOptions, targetPoint, /*margin*/0, centered);
}

//---

CQChartsSankeyNodeObj *
CQChartsSankeyPlot::
createNodeObj(const BBox &rect, Node *node, const ColorInd &ig, const ColorInd &iv) const
{
  return new NodeObj(this, rect, node, ig, iv);
}

CQChartsSankeyEdgeObj *
CQChartsSankeyPlot::
createEdgeObj(const BBox &rect, Edge *edge) const
{
  return new EdgeObj(this, rect, edge);
}

//---

bool
CQChartsSankeyPlot::
hasForeground() const
{
  if (! isLayerActive(CQChartsLayer::Type::FOREGROUND))
    return false;

  return true;
}

void
CQChartsSankeyPlot::
execDrawForeground(PaintDevice *device) const
{
  CQChartsPlot::execDrawForeground(device);
}

//---

CQChartsPlotCustomControls *
CQChartsSankeyPlot::
createCustomControls()
{
  auto *controls = new CQChartsSankeyPlotCustomControls(charts());

  controls->init();

  controls->setPlot(this);

  controls->updateWidgets();

  return controls;
}

//------

CQChartsSankeyPlotNode::
CQChartsSankeyPlotNode(const SankeyPlot *sankeyPlot, const QString &str) :
 sankeyPlot_(sankeyPlot), str_(str)
{
}

CQChartsSankeyPlotNode::
~CQChartsSankeyPlotNode()
{
  assert(! obj_);
}

void
CQChartsSankeyPlotNode::
addSrcEdge(Edge *edge, bool primary)
{
  edge->destNode()->setParent(edge->srcNode());

  srcEdges_.push_back(edge);

  srcDepth_ = -1;

  if (! primary)
    nonPrimaryEdges_.push_back(edge);
}

void
CQChartsSankeyPlotNode::
addDestEdge(Edge *edge, bool primary)
{
  edge->destNode()->parent_ = edge->srcNode();

  destEdges_.push_back(edge);

  destDepth_ = -1;

  if (! primary)
    nonPrimaryEdges_.push_back(edge);
}

#ifdef CQCHARTS_GRAPH_PATH_ID
void
CQChartsSankeyPlotNode::
sortPathIdEdges(bool force)
{
  if (! force && ! sankeyPlot()->isSortPathIdEdges())
    return;

  //---

  using PathIdEdges = std::map<int, Edges>;

  PathIdEdges srcPathIdEdges, destPathIdEdges;

  for (auto &edge : srcEdges_)
    srcPathIdEdges[-edge->pathId()].push_back(edge);

  for (auto &edge : destEdges_)
    destPathIdEdges[-edge->pathId()].push_back(edge);

  srcEdges_.clear();

  for (const auto &pe : srcPathIdEdges)
    for (const auto &edge : pe.second)
      srcEdges_.push_back(edge);

  destEdges_.clear();

  for (const auto &pe : destPathIdEdges)
    for (const auto &edge : pe.second)
      destEdges_.push_back(edge);
}
#endif

#ifdef CQCHARTS_GRAPH_PATH_ID
int
CQChartsSankeyPlotNode::
minPathId() const
{
  int  minPathId    = -1;
  bool minPathIdSet = false;

  auto updateMinPathId = [&](int pathId) {
    if (pathId < 0) return;

    if (! minPathIdSet || pathId < minPathId) {
      minPathId    = pathId;
      minPathIdSet = true;
    }
  };

  for (const auto &edge : srcEdges_)
    updateMinPathId(edge->pathId());

  for (const auto &edge : destEdges_)
    updateMinPathId(edge->pathId());

  return minPathId;
}
#endif

bool
CQChartsSankeyPlotNode::
hasDestNode(Node *destNode) const
{
  for (auto &destEdge : destEdges()) {
    if (destEdge->destNode() == destNode)
      return true;
  }

  return false;
}

int
CQChartsSankeyPlotNode::
srcDepth() const
{
  if (depth() >= 0)
    return depth() - 1;

  // use visited to detect loops
  NodeSet visited;

  visited.insert(this);

  return calcSrcDepth(visited);
}

int
CQChartsSankeyPlotNode::
calcSrcDepth(NodeSet &visited) const
{
  // already calculated
  if (srcDepth_ >= 0)
    return srcDepth_;

  //---

  auto *th = const_cast<CQChartsSankeyPlotNode *>(this);

  if (srcEdges_.empty()) {
    th->srcDepth_ = 0;
  }
  else {
    int depth = 0;

    for (const auto &edge : srcEdges_) {
      if (edge->isSelf()) continue;

      auto *node = edge->srcNode();

      auto p = visited.find(node);

      if (! sankeyPlot_->isNewVisited()) {
        if (p == visited.end()) {
          visited.insert(node);

          depth = std::max(depth, node->calcSrcDepth(visited));
        }
        else {
          depth = std::max(depth, int(visited.size() - 1));
        //depth = std::max(depth, node->srcDepth());
        }
      }
      else {
        if (p == visited.end()) {
          auto visited2 = visited;

          visited2.insert(node);

          depth = std::max(depth, node->calcSrcDepth(visited2));
        }
      }
    }

    th->srcDepth_ = depth + 1;
  }

  return srcDepth_;
}

int
CQChartsSankeyPlotNode::
destDepth() const
{
  if (depth() >= 0)
    return depth() + 1;

  // use visited to detect loops
  NodeSet visited;

  visited.insert(this);

  return calcDestDepth(visited);
}

int
CQChartsSankeyPlotNode::
calcDestDepth(NodeSet &visited) const
{
  // already calculated
  if (destDepth_ >= 0)
    return destDepth_;

  //---

  auto *th = const_cast<CQChartsSankeyPlotNode *>(this);

  if (destEdges_.empty()) {
    th->destDepth_ = 0;
  }
  else {
    int depth = 0;

    for (const auto &edge : destEdges_) {
      if (edge->isSelf()) continue;

      auto *node = edge->destNode();

      auto p = visited.find(node);

      if (! sankeyPlot_->isNewVisited()) {
        if (p == visited.end()) {
          visited.insert(node);

          depth = std::max(depth, node->calcDestDepth(visited));
        }
        else {
          depth = std::max(depth, int(visited.size() - 1));
        //depth = std::max(depth, node->destDepth());
        }
      }
      else {
        if (p == visited.end()) {
          auto visited2 = visited;

          visited2.insert(node);

          depth = std::max(depth, node->calcDestDepth(visited2));
        }
      }
    }

    th->destDepth_ = depth + 1;
  }

  return destDepth_;
}

const CQChartsSankeyPlotNode::BBox &
CQChartsSankeyPlotNode::
rect() const
{
  if (obj_) {
    assert(obj_->rect() == rect_);
  }

  return rect_;
}

void
CQChartsSankeyPlotNode::
setRect(const BBox &rect)
{
  assert(rect.isSet());

  rect_ = rect;

  if (obj_) // TODO: assert null or use move by
    obj_->setRect(rect);
}

double
CQChartsSankeyPlotNode::
edgeSum() const
{
  double sum = std::max(srcEdgeSum(), destEdgeSum());

  if (CMathUtil::realEq(sum, 0.0))
    sum = 1.0;

  return sum;
}

double
CQChartsSankeyPlotNode::
srcEdgeSum() const
{
  double value = 0.0;

  for (const auto &edge : srcEdges_) {
    if (edge->hasValue())
      value += edge->value().real();
  }

  return value;
}

double
CQChartsSankeyPlotNode::
destEdgeSum() const
{
  double value = 0.0;

  for (const auto &edge : destEdges_) {
    if (edge->hasValue())
      value += edge->value().real();
  }

  return value;
}

//---

void
CQChartsSankeyPlotNode::
setObj(Obj *obj)
{
  obj_ = obj;
}

//---

void
CQChartsSankeyPlotNode::
moveBy(const Point &delta)
{
  Point delta1(delta.x, delta.y);

  rect_.moveBy(delta1);

  if (obj_)
    obj_->moveBy(delta1);

  //---

  moveSrcEdgeRectsBy (delta1);
  moveDestEdgeRectsBy(delta1);
}

void
CQChartsSankeyPlotNode::
scale(double fx, double fy)
{
  rect_.scale(fx, fy);

  if (obj_)
    obj_->scale(fx, fy);

  //---

  scaleSrcEdgeRectsBy (fx, fy);
  scaleDestEdgeRectsBy(fx, fy);
}

//---

void
CQChartsSankeyPlotNode::
placeEdges(bool reset)
{
  if (! reset) {
    if (! srcEdgeRects().empty() || ! destEdgeRects().empty())
      return;
  }

  //---

#ifdef CQCHARTS_GRAPH_PATH_ID
  if (sankeyPlot()->hasAnyPathId())
    sortPathIdEdges();
#endif

  //---

  double x1 = rect().getXMin();
  double y1 = rect().getYMin();
  double x2 = rect().getXMax();
  double y2 = rect().getYMax();

  clearSrcEdgeRects ();
  clearDestEdgeRects();

  double srcTotal  = srcEdgeSum ();
  double destTotal = destEdgeSum();

  double nodeSize;

  if (sankeyPlot_->isHorizontal())
    nodeSize = y2 - y1;
  else
    nodeSize = x2 - x1;

  if (! sankeyPlot_->useMaxTotals()) {
    if (this->srcEdges().size() == 1) {
      auto *edge = *this->srcEdges().begin();

      setSrcEdgeRect(edge, BBox(x1, y1, x2, y2));
    }
    else {
      double boxSize = (srcTotal > 0.0 ? nodeSize/srcTotal : 0.0);

      double perpPos3 = (sankeyPlot_->isHorizontal() ? y2 : x2); // top/right

      for (const auto &edge : this->srcEdges()) {
        if (! edge->hasValue()) {
          setSrcEdgeRect(edge, BBox());
          continue;
        }

        double perpSize1 = boxSize*edge->value().real();
        double perpPos4  = perpPos3 - perpSize1;

        if (! hasSrcEdgeRect(edge))
          setSrcEdgeRect(edge, sankeyPlot_->isHorizontal() ?
                         BBox(x1, perpPos4, x2, perpPos3) : BBox(perpPos4, y1, perpPos3, y2));

        perpPos3 = perpPos4;
      }
    }

    //---

    if (this->destEdges().size() == 1) {
      auto *edge = *this->destEdges().begin();

      setDestEdgeRect(edge, BBox(x1, y1, x2, y2));
    }
    else {
      double boxSize = (destTotal > 0.0 ? nodeSize/destTotal : 0.0);

      double perpPos3 = (sankeyPlot_->isHorizontal() ? y2 : x2); // top/right

      for (const auto &edge : this->destEdges()) {
        if (! edge->hasValue()) {
          setDestEdgeRect(edge, BBox());
          continue;
        }

        double perpSize1 = boxSize*edge->value().real();
        double perpPos4  = perpPos3 - perpSize1;

        if (! hasDestEdgeRect(edge))
          setDestEdgeRect(edge, sankeyPlot_->isHorizontal() ?
                          BBox(x1, perpPos4, x2, perpPos3) : BBox(perpPos4, y1, perpPos3, y2));

        perpPos3 = perpPos4;
      }
    }
  }
  else {
    double maxTotal = std::max(srcTotal, destTotal);

    double boxSize = (maxTotal > 0.0 ? nodeSize/maxTotal : 0.0);

    double dperp1 = (nodeSize - boxSize*srcTotal )/2.0;
    double dperp2 = (nodeSize - boxSize*destTotal)/2.0;

    double perpPos3 = (sankeyPlot_->isHorizontal() ? y2 - dperp1 : x2 - dperp1); // top/right

    for (const auto &edge : this->srcEdges()) {
      if (! edge->hasValue()) {
        setSrcEdgeRect(edge, BBox());
        continue;
      }

      double perpSize1 = boxSize*edge->value().real();
      double perpPos4  = perpPos3 - perpSize1;

      if (! hasSrcEdgeRect(edge))
        setSrcEdgeRect(edge, sankeyPlot_->isHorizontal() ?
                       BBox(x1, perpPos4, x2, perpPos3) : BBox(perpPos4, y1, perpPos3, y2));

      perpPos3 = perpPos4;
    }

    //---

    perpPos3 = (sankeyPlot_->isHorizontal() ? y2 - dperp2 : x2 - dperp2); // top/right

    for (const auto &edge : this->destEdges()) {
      if (! edge->hasValue()) {
        setDestEdgeRect(edge, BBox());
        continue;
      }

      double perpSize1 = boxSize*edge->value().real();
      double perpPos4  = perpPos3 - perpSize1;

      if (! hasDestEdgeRect(edge))
        setDestEdgeRect(edge, sankeyPlot_->isHorizontal() ?
                        BBox(x1, perpPos4, x2, perpPos3) : BBox(perpPos4, y1, perpPos3, y2));

      perpPos3 = perpPos4;
    }
  }
}

void
CQChartsSankeyPlotNode::
setSrcEdgeRect(Edge *edge, const BBox &bbox)
{
  srcEdgeRect_[edge] = bbox;

#ifdef CQCHARTS_GRAPH_PATH_ID
  if (edge->pathId() >= 0)
    srcPathIdRect_[edge->pathId()] = bbox;
#endif
}

void
CQChartsSankeyPlotNode::
setDestEdgeRect(Edge *edge, const BBox &bbox)
{
  destEdgeRect_[edge] = bbox;

#ifdef CQCHARTS_GRAPH_PATH_ID
  if (edge->pathId() >= 0)
    destPathIdRect_[edge->pathId()] = bbox;
#endif
}

#ifdef CQCHARTS_GRAPH_PATH_ID
void
CQChartsSankeyPlotNode::
adjustPathIdSrcDestRects()
{
  if (! sankeyPlot_->useMaxTotals())
    return;

  // align each source path id rect with destination path id rects
  for (const auto &pathIdRect : srcPathIdRect_) {
    int pathId = pathIdRect.first;

    auto p = destPathIdRect_.find(pathId);
    if (p == destPathIdRect_.end()) continue;

    auto srcRect  = pathIdRect.second;
    auto destRect = (*p).second;

    if (! srcRect.isSet() || ! destRect.isSet())
      continue;

    double perpPos1 = srcRect .getMidPerpPos(sankeyPlot_->isHorizontal());
    double perpPos2 = destRect.getMidPerpPos(sankeyPlot_->isHorizontal());

    double dperp = perpPos1 - perpPos2;

    if (CMathUtil::realEq(dperp, 0.0))
      continue;

    if (srcEdgeSum() > destEdgeSum()) {
      if (sankeyPlot_->isHorizontal())
        destRect.moveBy(Point(0.0, dperp));
      else
        destRect.moveBy(Point(dperp, 0.0));

      for (auto &edge : this->destEdges()) {
        if (edge->pathId() == pathId)
          setDestEdgeRect(edge, destRect);
      }
    }
    else {
      if (sankeyPlot_->isHorizontal())
        srcRect.moveBy(Point(0.0, -dperp));
      else
        srcRect.moveBy(Point(-dperp, 0.0));

      for (auto &edge : this->srcEdges()) {
        if (edge->pathId() == pathId)
          setSrcEdgeRect(edge, srcRect);
      }
    }
  }
}
#endif

//---

void
CQChartsSankeyPlotNode::
allSrcNodesAndEdges(NodeSet &nodeSet, EdgeSet &edgeSet) const
{
  if (nodeSet.find(this) != nodeSet.end())
    return;

  nodeSet.insert(this);

  for (const auto &edge : this->srcEdges()) {
    edgeSet.insert(edge);
  }

  for (const auto &edge : this->srcEdges()) {
    auto *node = edge->srcNode();

    node->allSrcNodesAndEdges(nodeSet, edgeSet);
  }
}

void
CQChartsSankeyPlotNode::
allDestNodesAndEdges(NodeSet &nodeSet, EdgeSet &edgeSet) const
{
  if (nodeSet.find(this) != nodeSet.end())
    return;

  nodeSet.insert(this);

  for (const auto &edge : this->destEdges()) {
    edgeSet.insert(edge);
  }

  for (const auto &edge : this->destEdges()) {
    auto *node = edge->destNode();

    node->allDestNodesAndEdges(nodeSet, edgeSet);
  }
}

//---

QColor
CQChartsSankeyPlotNode::
calcColor() const
{
  CQChartsUtil::ColorInd ic;

  if (ngroup() > 0 && group() >= 0 && group() < ngroup())
    ic = CQChartsUtil::ColorInd(group(), ngroup());
  else
    ic = CQChartsUtil::ColorInd(id(), sankeyPlot_->numNodes());

  if (fillColor().isValid())
    return sankeyPlot_->interpColor(fillColor(), ic);
  else
    return sankeyPlot_->interpNodeFillColor(ic);
}

//------

CQChartsSankeyPlotEdge::
CQChartsSankeyPlotEdge(const SankeyPlot *sankeyPlot, const OptReal &value,
                       Node *srcNode, Node *destNode) :
 sankeyPlot_(sankeyPlot), value_(value), srcNode_(srcNode), destNode_(destNode)
{
}

CQChartsSankeyPlotEdge::
~CQChartsSankeyPlotEdge()
{
  assert(! obj_);
}

//---

void
CQChartsSankeyPlotEdge::
setObj(Obj *obj)
{
  obj_ = obj;
}

//---

bool
CQChartsSankeyPlotEdge::
overlaps(const Edge *edge) const
{
  QPainterPath path1, path2;

  if (this->edgePath(path1, /*isLine*/true) && edge->edgePath(path2, /*isLine*/true))
    return path1.intersects(path2);

  //---

  auto p11 = this->srcNode ()->rect().getCenter();
  auto p12 = this->destNode()->rect().getCenter();

  auto p21 = edge->srcNode ()->rect().getCenter();
  auto p22 = edge->destNode()->rect().getCenter();

  Point  pi;
  double mu1, mu2;

  if (! CQChartsUtil::intersectLines(p11, p12, p21, p22, pi, mu1, mu2))
    return false;

  return true;
}

bool
CQChartsSankeyPlotEdge::
edgePath(QPainterPath &path, bool isLine) const
{
  auto *th = const_cast<CQChartsSankeyPlotEdge *>(this);

  path = QPainterPath();

  // get connection rect of source and destination object
  auto *srcObj  = this->srcNode ()->obj();
  auto *destObj = this->destNode()->obj();

  BBox srcRect, destRect;

  if (srcObj && destObj) {
    srcRect  = srcObj ->node()->destEdgeRect(th);
    destRect = destObj->node()->srcEdgeRect (th);
  }
  else {
    th->srcNode ()->placeEdges(/*reset*/false);
    th->destNode()->placeEdges(/*reset*/false);

    srcRect  = this->srcNode ()->destEdgeRect(th);
    destRect = this->destNode()->srcEdgeRect (th);
  }

  if (! srcRect.isSet())
    srcRect = this->srcNode()->rect();

  if (! destRect.isSet())
    destRect = this->destNode()->rect();

  if (! srcRect.isSet() || ! destRect.isSet())
    return false;

  //---

  if (! isLine) {
    auto angle = Angle::fromOrientation(sankeyPlot_->orientation());

    CQChartsDrawUtil::edgePath(path, srcRect, destRect, CQChartsDrawUtil::EdgeType::ARC, angle);
  }
  else {
    auto p1 = srcRect .getCenter();
    auto p2 = destRect.getCenter();

    Angle angle1, angle2;

    if (sankeyPlot_->isHorizontal()) {
      if (p1.x < p2.x) {
        p1.x   = srcRect .getXMax();
        p2.x   = destRect.getXMin();
        angle1 = Angle::degrees(0);
        angle2 = Angle::degrees(180);
      }
      else {
        p1.x   = srcRect .getXMin();
        p2.x   = destRect.getXMax();
        angle1 = Angle::degrees(180);
        angle2 = Angle::degrees(0);
      }

      CQChartsDrawUtil::curvePath(path, p1, p2, CQChartsDrawUtil::EdgeType::ARC, angle1, angle2);
    }
    else {
      if (p1.y < p2.y) {
        p1.y   = srcRect .getYMax();
        p2.y   = destRect.getYMin();
        angle1 = Angle::degrees(90);
        angle2 = Angle::degrees(270);
      }
      else {
        p1.y   = srcRect .getYMin();
        p2.y   = destRect.getYMax();
        angle1 = Angle::degrees(270);
        angle2 = Angle::degrees(90);
      }

      CQChartsDrawUtil::curvePath(path, p1, p2, CQChartsDrawUtil::EdgeType::ARC, angle1, angle2);
    }
  }

  return true;
}

//------

CQChartsSankeyNodeObj::
CQChartsSankeyNodeObj(const SankeyPlot *sankeyPlot, const BBox &rect, Node *node,
                      const ColorInd &ig, const ColorInd &iv) :
 CQChartsPlotObj(const_cast<SankeyPlot *>(sankeyPlot), rect, ColorInd(), ig, iv),
 sankeyPlot_(sankeyPlot), node_(node)
{
  setDetailHint(DetailHint::MAJOR);

  setEditable(true);

  //---

//placeEdges(/*reset*/true);
}

CQChartsSankeyNodeObj::
~CQChartsSankeyNodeObj()
{
  if (node())
    node()->setObj(nullptr);
}

QString
CQChartsSankeyNodeObj::
name() const
{
  return node()->name();
}

void
CQChartsSankeyNodeObj::
setName(const QString &s)
{
  node()->setName(s);
}

double
CQChartsSankeyNodeObj::
value() const
{
  return node()->value().realOr(0.0);
}

void
CQChartsSankeyNodeObj::
setValue(double r)
{
  node()->setValue(CQChartsSankeyPlotNode::OptReal(r));
}

int
CQChartsSankeyNodeObj::
depth() const
{
  return node()->depth();
}

void
CQChartsSankeyNodeObj::
setDepth(int depth)
{
  node()->setDepth(depth);
}

CQChartsColor
CQChartsSankeyNodeObj::
fillColor() const
{
  return node()->fillColor();
}

void
CQChartsSankeyNodeObj::
setFillColor(const CQChartsColor &c)
{
  node()->setFillColor(c);
}

void
CQChartsSankeyNodeObj::
placeEdges(bool reset)
{
  node()->placeEdges(reset);
}

QString
CQChartsSankeyNodeObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(node()->id());
}

QString
CQChartsSankeyNodeObj::
calcTipId() const
{
  Edge *edge = nullptr;

  if      (! node()->srcEdges().empty())
    edge = *node()->srcEdges().begin();
  else if (! node()->destEdges().empty())
    edge = *node()->destEdges().begin();

  auto namedColumn = [&](const QString &name, const QString &defName="") {
    QString headerName;

    if (edge && edge->hasNamedColumn(name))
      headerName = sankeyPlot_->columnHeaderName(edge->namedColumn(name));

    if (headerName == "")
      headerName = (defName.length() ? defName : name);

    return headerName;
  };

  //---

  CQChartsTableTip tableTip;

  auto name = this->name();

  if (name == "")
    name = this->id();

  auto hierName = this->hierName();

  if (hierName != name)
    tableTip.addTableRow("Hier Name", hierName);

  tableTip.addTableRow("Name", name);

  if (node()->hasValue()) {
    auto valueName = (! sankeyPlot_->isEdgeRows() ? namedColumn("Value") : "Value");

    tableTip.addTableRow(valueName, value());
  }

  if (depth() >= 0)
    tableTip.addTableRow(namedColumn("Depth"), depth());

  int ns = int(node()->srcEdges ().size());
  int nd = int(node()->destEdges().size());

  tableTip.addTableRow("Edges", QString("In:%1, Out:%2").arg(ns).arg(nd));

  if (sankeyPlot_->groupColumn().isValid())
    tableTip.addTableRow("Group", node()->group());

  //---

  sankeyPlot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

//---

void
CQChartsSankeyNodeObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = (path.length() ? path + "/" : ""); path1 += propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "hierName")->setDesc("Hierarchical Name");
  model->addProperty(path1, this, "name"    )->setDesc("Name");
  model->addProperty(path1, this, "value"   )->setDesc("Value");
  model->addProperty(path1, this, "depth"   )->setDesc("Depth");
  model->addProperty(path1, this, "color"   )->setDesc("Color");
}

//---

void
CQChartsSankeyNodeObj::
moveBy(const Point &delta)
{
  //std::cerr << "  Move " << node()->str().toStdString() << " by " << delta.y << "\n";
  Point delta1(delta.x, delta.y);

  rect_.moveBy(delta1);
}

void
CQChartsSankeyNodeObj::
scale(double fx, double fy)
{
  rect_.scale(fx, fy);
}

//---

bool
CQChartsSankeyNodeObj::
editPress(const Point &p)
{
  editChanged_ = false;

  editHandles()->setDragPos(p);

  return true;
}

bool
CQChartsSankeyNodeObj::
editMove(const Point &p)
{
  const auto &dragPos  = editHandles()->dragPos();
  const auto &dragSide = editHandles()->dragSide();

  if (dragSide != CQChartsResizeSide::MOVE)
    return false;

  double dx = p.x - dragPos.x;
  double dy = p.y - dragPos.y;

  editHandles()->updateBBox(dx, dy);

  setEditBBox(editHandles()->bbox(), dragSide);

  editHandles()->setDragPos(p);

  auto *graph = sankeyPlot_->graph();

  if (graph)
    graph->updateRect();

  editChanged_ = true;

  const_cast<CQChartsSankeyPlot *>(sankeyPlot())->drawObjs();

  return true;
}

bool
CQChartsSankeyNodeObj::
editMotion(const Point &p)
{
  return editHandles()->selectInside(p);
}

bool
CQChartsSankeyNodeObj::
editRelease(const Point &)
{
  if (editChanged_)
    const_cast<CQChartsSankeyPlot *>(sankeyPlot())->invalidateObjTree();

  return true;
}

void
CQChartsSankeyNodeObj::
setEditBBox(const BBox &bbox, const CQChartsResizeSide &)
{
  assert(bbox.isSet());

  double dx = 0.0, dy = 0.0;

  if (sankeyPlot()->isConstrainMove()) {
    if (sankeyPlot_->isHorizontal())
      dy = bbox.getYMin() - rect_.getYMin();
    else
      dx = bbox.getXMin() - rect_.getXMin();
  }
  else {
    dx = bbox.getXMin() - rect_.getXMin();
    dy = bbox.getYMin() - rect_.getYMin();
  }

  node()->moveBy(Point(dx, dy));
}

//---

void
CQChartsSankeyNodeObj::
getObjSelectIndices(Indices &inds) const
{
  addColumnsSelectIndex(inds, sankeyPlot_->modelColumns());
}

CQChartsSankeyNodeObj::PlotObjs
CQChartsSankeyNodeObj::
getConnected() const
{
  PlotObjs plotObjs;

  for (auto &edgeRect : node()->srcEdgeRects())
    plotObjs.push_back(edgeRect.first->obj());

  for (auto &edgeRect : node()->destEdgeRects())
    plotObjs.push_back(edgeRect.first->obj());

  return plotObjs;
}

//---

void
CQChartsSankeyNodeObj::
draw(PaintDevice *device) const
{
  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  device->setColorNames();

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw node
  if (rect().isSet())
    device->drawRect(rect());

  //---

  device->resetColorNames();

  //---

  // show source and destination nodes on inside
  if (sankeyPlot_->drawLayerType() == CQChartsLayer::Type::MOUSE_OVER) {
    if (sankeyPlot_->mouseColoring() != CQChartsSankeyPlot::ConnectionType::NONE) {
      sankeyPlot_->view()->setDrawLayerType(CQChartsLayer::Type::MOUSE_OVER_EXTRA);

      drawConnectionMouseOver(device, static_cast<int>(sankeyPlot_->mouseColoring()));

      sankeyPlot_->view()->setDrawLayerType(CQChartsLayer::Type::MOUSE_OVER);
    }
  }
}

void
CQChartsSankeyNodeObj::
#ifdef CQCHARTS_GRAPH_PATH_ID
drawConnectionMouseOver(PaintDevice *device, int imouseColoring, int pathId) const
#else
drawConnectionMouseOver(PaintDevice *device, int imouseColoring) const
#endif
{
  auto mouseColoring = static_cast<CQChartsSankeyPlot::ConnectionType>(imouseColoring);

  //---

  auto drawEdgeInside = [&](const Edge *edge) {
    auto *edgeObj = edge->obj(); if (! edgeObj) return;

    bool oldEnabled = edgeObj->setNotificationsEnabled(false);
    edgeObj->setInside(true); edgeObj->draw(device); edgeObj->setInside(false);
    (void) edgeObj->setNotificationsEnabled(oldEnabled);
  };

#ifdef CQCHARTS_GRAPH_PATH_ID
  auto drawNodeInside = [&](const Node *node, bool isSrc) {
#else
  auto drawNodeInside = [&](const Node *node, bool /*isSrc*/) {
#endif
    auto *nodeObj = node->obj(); if (! nodeObj) return;

    bool oldEnabled = nodeObj->setNotificationsEnabled(false);

    nodeObj->setInside(true);

    if (sankeyPlot_->isMouseNodeColoring())
      nodeObj->draw(device);

#ifdef CQCHARTS_GRAPH_PATH_ID
    if (pathId >= 0) {
      Edge *edge = nullptr;

      if (isSrc) {
        for (const auto &edge1 : node->destEdges()) {
          if (edge1->pathId() == pathId) {
            edge = edge1;
            break;
          }
        }
      }
      else {
        for (const auto &edge1 : node->srcEdges()) {
          if (edge1->pathId() == pathId) {
            edge = edge1;
            break;
          }
        }
      }

      if (edge) {
        auto rect = (isSrc ? node->destEdgeRect(edge) : node->srcEdgeRect(edge));

        if (! sankeyPlot()->isNodeTextVisible() && rect.isSet())
          nodeObj->drawFgRect(device, rect);
      }
      else {
        //nodeObj->drawFg(device);
      }
    }
    else
      nodeObj->drawFg(device);
#else
    nodeObj->drawFg(device);
#endif

    nodeObj->setInside(false);

    (void) nodeObj->setNotificationsEnabled(oldEnabled);
  };

  //---

  if      (mouseColoring == CQChartsSankeyPlot::ConnectionType::SRC ||
           mouseColoring == CQChartsSankeyPlot::ConnectionType::SRC_DEST) {
    for (const auto &edge : node()->srcEdges()) {
#ifdef CQCHARTS_GRAPH_PATH_ID
      if (pathId < 0 || edge->pathId() == pathId)
        drawEdgeInside(edge);
#else
      drawEdgeInside(edge);
#endif
    }
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_SRC ||
           mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_SRC_DEST) {
    CQChartsSankeyPlotNode::NodeSet nodeSet;
    CQChartsSankeyPlotNode::EdgeSet edgeSet;

    node()->allSrcNodesAndEdges(nodeSet, edgeSet);

    for (const auto &edge : edgeSet) {
#ifdef CQCHARTS_GRAPH_PATH_ID
      if (pathId < 0 || edge->pathId() == pathId)
        drawEdgeInside(edge);
#else
      drawEdgeInside(edge);
#endif
    }

    for (const auto &node : nodeSet)
      drawNodeInside(node, /*isSrc*/true);
  }

  if      (mouseColoring == CQChartsSankeyPlot::ConnectionType::DEST ||
           mouseColoring == CQChartsSankeyPlot::ConnectionType::SRC_DEST) {
    for (const auto &edge : node()->destEdges()) {
#ifdef CQCHARTS_GRAPH_PATH_ID
      if (pathId < 0 || edge->pathId() == pathId)
        drawEdgeInside(edge);
#else
      drawEdgeInside(edge);
#endif
    }
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_DEST ||
           mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_SRC_DEST) {
    CQChartsSankeyPlotNode::NodeSet nodeSet;
    CQChartsSankeyPlotNode::EdgeSet edgeSet;

    node()->allDestNodesAndEdges(nodeSet, edgeSet);

    for (const auto &edge : edgeSet) {
#ifdef CQCHARTS_GRAPH_PATH_ID
      if (pathId < 0 || edge->pathId() == pathId)
        drawEdgeInside(edge);
#else
      drawEdgeInside(edge);
#endif
    }

    for (const auto &node : nodeSet)
      drawNodeInside(node, /*isSrc*/false);
  }
}

void
CQChartsSankeyNodeObj::
drawFg(PaintDevice *device) const
{
  drawFgRect(device, rect());
}

void
CQChartsSankeyNodeObj::
drawFgRect(PaintDevice *device, const BBox &rect) const
{
  using TextVisibleType = CQChartsSankeyPlot::TextVisibleType;

  bool nameVisible = true;

  if (! sankeyPlot_->isNodeTextVisible()) {
    nameVisible = false;

    if ((uint(sankeyPlot_->nodeTextInsideVisible()) & uint(TextVisibleType::NAME)) &&
        isInside())
      nameVisible = true;

    if ((uint(sankeyPlot_->nodeTextSelectedVisible()) & uint(TextVisibleType::NAME)) &&
        isSelected())
      nameVisible = true;
  }

  if (nameVisible) {
    if (node()->image().isValid())
      drawFgImage(device, rect);
    else
      drawFgText(device, rect);
  }

  //---

  bool valueVisible = sankeyPlot_->isNodeValueLabel();

  if ((uint(sankeyPlot_->nodeTextInsideVisible()) & uint(TextVisibleType::VALUE)) &&
      isInside())
    valueVisible = true;

  if ((uint(sankeyPlot_->nodeTextSelectedVisible()) & uint(TextVisibleType::VALUE)) &&
      isSelected())
    valueVisible = true;

  if (valueVisible)
    drawValueLabel(device, rect);
}

void
CQChartsSankeyNodeObj::
drawFgImage(PaintDevice *device, const BBox &rect) const
{
  using TextPosition = CQChartsSankeyPlot::TextPosition;

  setTextPen(device);

  //---

  double pTextMargin = 4; // pixels

  QFontMetricsF fm(device->font());

  auto iw = sankeyPlot_->pixelToWindowWidth (fm.height())*1.1;
  auto ih = sankeyPlot_->pixelToWindowHeight(fm.height())*1.1;

  BBox irect;

  if (sankeyPlot_->isHorizontal()) {
    double textMargin = sankeyPlot_->pixelToWindowWidth(pTextMargin);

    double yc;

    if (sankeyPlot_->isNodeValueLabel())
      yc = rect.getYMin() + ih/2;
    else
      yc = rect.getYMid();

    double xm = sankeyPlot_->getCalcDataRange().xmid();

    auto align = sankeyPlot_->nodeTextLabelAlign();

    if      (sankeyPlot_->nodeTextLabelPosition() == TextPosition::INTERNAL) {
      if (rect.getXMid() < xm - iw/2.0)
        align = Qt::AlignRight;
      else
        align = Qt::AlignLeft;
    }
    else if (sankeyPlot_->nodeTextLabelPosition() == TextPosition::EXTERNAL) {
      if (rect.getXMid() < xm - iw/2.0)
        align = Qt::AlignLeft;
      else
        align = Qt::AlignRight;
    }

    if      (align & Qt::AlignLeft)
      irect = BBox(rect.getXMin() - textMargin - iw, yc - ih/2,
                   rect.getXMin() - textMargin     , yc + ih/2);
    else if (align & Qt::AlignRight)
      irect = BBox(rect.getXMax() + textMargin     , yc - ih/2,
                   rect.getXMax() + textMargin + iw, yc + ih/2);
    else
      irect = BBox(rect.getXMid() - iw/2.0, yc - ih/2,
                   rect.getXMid() + iw/2.0, yc + ih/2);
  }
  else {
    double textMargin = sankeyPlot_->pixelToWindowHeight(pTextMargin);

    double xc;

    if (sankeyPlot_->isNodeValueLabel())
      xc = rect.getXMin() + iw/2;
    else
      xc = rect.getXMid();

    double ym = sankeyPlot_->getCalcDataRange().ymid();

    auto align = sankeyPlot_->nodeTextLabelAlign();

    if      (sankeyPlot_->nodeTextLabelPosition() == TextPosition::INTERNAL) {
      if (rect.getYMid() < ym - ih/2.0)
        align = Qt::AlignTop;
      else
        align = Qt::AlignBottom;
    }
    else if (sankeyPlot_->nodeTextLabelPosition() == TextPosition::EXTERNAL) {
      if (rect.getYMid() < ym - ih/2.0)
        align = Qt::AlignBottom;
      else
        align = Qt::AlignTop;
    }

    if      (align & Qt::AlignTop)
      irect = BBox(xc - iw/2, rect.getYMax() + textMargin,
                   xc + iw/2, rect.getYMax() + textMargin + ih);
    else if (align & Qt::AlignBottom)
      irect = BBox(xc - iw/2, rect.getYMin() - textMargin - ih,
                   xc + iw/2, rect.getYMin() - textMargin);
    else
      irect = BBox(xc - iw/2, rect.getYMid() - ih/2.0,
                   xc + iw/2, rect.getYMid() + ih/2.0);
  }

  device->drawImageInRect(irect, node()->image());
}

void
CQChartsSankeyNodeObj::
drawFgText(PaintDevice *device, const BBox &rect) const
{
  using TextPosition = CQChartsSankeyPlot::TextPosition;

  double pTextMargin = 4; // pixels

  auto prect = sankeyPlot()->windowToPixel(rect);

  //---

  setTextPen(device);

  //---

  auto str = node()->label();

  if (! str.length())
    str = node()->name();

  if (! str.length())
    return;

  //---

  QFontMetricsF fm(device->font());

  auto ptw = fm.horizontalAdvance(str);

  auto clipLength = sankeyPlot_->lengthPixelWidth(sankeyPlot()->nodeTextClipLength());

  if (clipLength > 0.0)
    ptw = std::min(ptw, clipLength);

  auto tw = sankeyPlot_->pixelToWindowWidth(ptw);

  //---

  Point pt;

  if (sankeyPlot_->isHorizontal()) {
    // align left/right depending on left/right of mid x
    double xm = sankeyPlot_->getCalcDataRange().xmid();

    auto align = sankeyPlot_->nodeTextLabelAlign();

    if      (sankeyPlot_->nodeTextLabelPosition() == TextPosition::INTERNAL) {
      if (rect.getXMid() < xm - tw)
        align = Qt::AlignRight;
      else
        align = Qt::AlignLeft;
    }
    else if (sankeyPlot_->nodeTextLabelPosition() == TextPosition::EXTERNAL) {
      if (rect.getXMid() < xm - tw)
        align = Qt::AlignLeft;
      else
        align = Qt::AlignRight;
    }

    double tx;

    if      (align & Qt::AlignLeft)
      tx = prect.getXMin() - pTextMargin - ptw;
    else if (align & Qt::AlignRight)
      tx = prect.getXMax() + pTextMargin; // left
    else
      tx = prect.getXMid() - ptw/2.0;

    // centered at mid bbox
    double ty = prect.getYMid() + (fm.ascent() - fm.descent())/2;

    pt = sankeyPlot()->pixelToWindow(Point(tx, ty));
  }
  else {
    // align bottom/top depending on top/bottom of mid y
    double ym = sankeyPlot_->getCalcDataRange().ymid();

    auto align = sankeyPlot_->nodeTextLabelAlign();

    if      (sankeyPlot_->nodeTextLabelPosition() == TextPosition::INTERNAL) {
      if (rect.getYMid() < ym - tw)
        align = Qt::AlignTop;
      else
        align = Qt::AlignBottom;
    }
    else if (sankeyPlot_->nodeTextLabelPosition() == TextPosition::EXTERNAL) {
      if (rect.getYMid() < ym - tw)
        align = Qt::AlignBottom;
      else
        align = Qt::AlignTop;
    }

    double ty;

    if      (align & Qt::AlignTop)
      ty = prect.getYMin() - pTextMargin - fm.descent();
    else if (align & Qt::AlignBottom)
      ty = prect.getYMax() + pTextMargin + fm.ascent();
    else
      ty = prect.getYMid() + (fm.ascent() - fm.descent())/2.0;

    // centered at mid bbox
    double tx = prect.getXMid() - ptw/2.0;

    pt = sankeyPlot()->pixelToWindow(Point(tx, ty));
  }

  // only support contrast
  auto textOptions = sankeyPlot()->nodeTextOptions();

//textOptions.angle      = Angle();
  textOptions.align      = Qt::AlignLeft;
  textOptions.clipLength = clipLength;

  if (sankeyPlot_->isAdjustText())
    sankeyPlot_->addDrawText(device, str, pt, textOptions, rect.getCenter());
  else {
    CQChartsDrawUtil::drawTextAtPoint(device, pt, str, textOptions);
  }
}

void
CQChartsSankeyNodeObj::
setTextPen(PaintDevice *device) const
{
  // set font
  sankeyPlot_->setPainterFont(device, sankeyPlot_->nodeTextFont());

  // set text pen
  auto ic = calcColorInd();

  PenBrush penBrush;

  auto c = sankeyPlot_->interpNodeTextColor(ic);

  sankeyPlot_->setPen(penBrush, PenData(true, c, sankeyPlot_->nodeTextAlpha()));

  device->setPen(penBrush.pen);
}

void
CQChartsSankeyNodeObj::
drawValueLabel(PaintDevice *device, const BBox &rect) const
{
  using TextPosition = CQChartsSankeyPlot::TextPosition;

  setTextPen(device);

  //---

  double pTextMargin = 4; // pixels

  auto prect = sankeyPlot()->windowToPixel(rect);

  double value;

  if (node()->hasValue())
    value = this->value();
  else {
    value = node()->edgeSum();
    if (value <= 1) return; // TODO: check value column type
  }

  //---

  QFontMetricsF fm(device->font());

  auto str = QString::number(value);

  auto ptw = fm.horizontalAdvance(str);
  auto tw  = sankeyPlot_->pixelToWindowWidth(ptw);

  Point pt;

  if (sankeyPlot_->isHorizontal()) {
    double xm = sankeyPlot_->getCalcDataRange().xmid();

    auto align = sankeyPlot_->nodeTextValueAlign();

    if      (sankeyPlot_->nodeTextValuePosition() == TextPosition::INTERNAL) {
      if (rect.getXMid() < xm - tw)
        align = Qt::AlignRight;
      else
        align = Qt::AlignLeft;
    }
    else if (sankeyPlot_->nodeTextValuePosition() == TextPosition::EXTERNAL) {
      if (rect.getXMid() < xm - tw)
        align = Qt::AlignLeft;
      else
        align = Qt::AlignRight;
    }

    auto ty = prect.getYMax() - fm.ascent();

    if      (align & Qt::AlignLeft)
      pt = Point(prect.getXMin() - pTextMargin - ptw, ty);
    else if (align & Qt::AlignRight)
      pt = Point(prect.getXMax() + pTextMargin, ty);
    else
      pt = Point(prect.getXMid() - ptw/2.0, ty);
  }
  else {
    double ym = sankeyPlot_->getCalcDataRange().ymid();

    auto align = sankeyPlot_->nodeTextValueAlign();

    if      (sankeyPlot_->nodeTextValuePosition() == TextPosition::INTERNAL) {
      if (rect.getYMid() < ym - tw)
        align = Qt::AlignTop;
      else
        align = Qt::AlignBottom;
    }
    else if (sankeyPlot_->nodeTextValuePosition() == TextPosition::EXTERNAL) {
      if (rect.getYMid() < ym - tw)
        align = Qt::AlignBottom;
      else
        align = Qt::AlignTop;
    }

    auto tx = prect.getXMax() - pTextMargin - ptw;

    if      (align & Qt::AlignTop)
      pt = Point(tx, prect.getYMin() + fm.ascent());
    else if (align & Qt::AlignBottom)
      pt = Point(tx, prect.getYMax() - fm.descent());
    else
      pt = Point(tx, prect.getYMid() + (fm.descent() - fm.descent())/2.0);
  }

  auto textOptions = sankeyPlot()->nodeTextOptions();

  textOptions.align = Qt::AlignLeft | Qt::AlignTop;

  CQChartsDrawUtil::drawTextAtPoint(device, sankeyPlot()->pixelToWindow(pt), str, textOptions);
}

void
CQChartsSankeyNodeObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  // set fill and stroke
  auto ic = calcColorInd();

  QColor bc;

  if (node()->strokeColor().isValid())
    bc = sankeyPlot_->interpColor(node()->strokeColor(), ic);
  else
    bc = sankeyPlot_->interpNodeStrokeColor(ic);

  auto fc = calcFillColor();

  auto fillAlpha   = (node()->fillAlpha  ().isValid() ?
    node()->fillAlpha() : sankeyPlot_->nodeFillAlpha());
  auto fillPattern = (node()->fillPattern().isValid() ?
    node()->fillPattern() : sankeyPlot_->nodeFillPattern());

  auto strokeAlpha = (node()->strokeAlpha().isValid() ?
    node()->strokeAlpha() : sankeyPlot_->nodeStrokeAlpha());
  auto strokeWidth = (node()->strokeWidth().isValid() ?
    node()->strokeWidth() : sankeyPlot_->nodeStrokeWidth());
  auto strokeDash  = (node()->strokeDash ().isValid() ?
    node()->strokeDash () : sankeyPlot_->nodeStrokeDash());

  sankeyPlot_->setPenBrush(penBrush,
    sankeyPlot_->nodePenData  (bc, strokeAlpha, strokeWidth, strokeDash),
    sankeyPlot_->nodeBrushData(fc, fillAlpha, fillPattern));

  if (updateState)
    sankeyPlot_->updateObjPenBrushState(this, penBrush);
}

QColor
CQChartsSankeyNodeObj::
calcFillColor() const
{
  QColor fc;

  if (sankeyPlot_->isSrcColoring()) {
    using Colors = std::vector<QColor>;

    Colors colors;

    for (const auto &edge : node()->srcEdges()) {
      auto *srcNode = edge->srcNode();

      auto c = srcNode->calcColor();

      colors.push_back(c);
    }

    if (! colors.empty())
      fc = CQChartsUtil::blendColors(colors);
    else
      fc = node()->calcColor();
  }
  else {
    auto colorInd = calcColorInd();

    if (fillColor().isValid())
      fc = sankeyPlot_->interpColor(fillColor(), colorInd);
    else if (sankeyPlot_->colorColumn().isValid()) {
      using Colors = std::vector<QColor>;

      Colors colors;

      for (const auto &edge : node()->srcEdges()) {
        auto *edgeObj = edge->obj();

        auto ind  = edgeObj->modelInd();
        auto ind1 = sankeyPlot_->unnormalizeIndex(ind);

        Color color;

        if (sankeyPlot_->colorColumnColor(ind1.row(), ind1.parent(), color)) {
          auto c = sankeyPlot_->interpColor(color, colorInd);

          colors.push_back(c);
        }
      }

      for (const auto &edge : node()->destEdges()) {
        auto *edgeObj = edge->obj();

        auto ind  = edgeObj->modelInd();
        auto ind1 = sankeyPlot_->unnormalizeIndex(ind);

        Color color;

        if (sankeyPlot_->colorColumnColor(ind1.row(), ind1.parent(), color)) {
          auto c = sankeyPlot_->interpColor(color, colorInd);

          colors.push_back(c);
        }
      }

      if (! colors.empty())
        fc = CQChartsUtil::blendColors(colors);
      else
        fc = sankeyPlot_->interpNodeFillColor(colorInd);
    }
    else
      fc = sankeyPlot_->interpNodeFillColor(colorInd);
  }

  return fc;
}

//------

CQChartsSankeyEdgeObj::
CQChartsSankeyEdgeObj(const SankeyPlot *sankeyPlot, const BBox &rect, Edge *edge) :
 CQChartsPlotObj(const_cast<SankeyPlot *>(sankeyPlot), rect), sankeyPlot_(sankeyPlot), edge_(edge)
{
  //setDetailHint(DetailHint::MAJOR);

  if (edge->modelInd().isValid())
    setModelInd(edge->modelInd());
}

CQChartsSankeyEdgeObj::
~CQChartsSankeyEdgeObj()
{
  if (edge_)
    edge_->setObj(nullptr);
}

QString
CQChartsSankeyEdgeObj::
calcId() const
{
  auto *srcObj  = edge()->srcNode ()->obj();
  auto *destObj = edge()->destNode()->obj();

  return QString("%1:%2:%3:%4").arg(typeName()).
           arg(srcObj->calcId()).arg(destObj->calcId()).arg(edge()->id());
}

QString
CQChartsSankeyEdgeObj::
calcTipId() const
{
  auto namedColumn = [&](const QString &name, const QString &defName="") {
    if (edge()->hasNamedColumn(name))
      return sankeyPlot_->columnHeaderName(edge()->namedColumn(name));

    auto headerName = (defName.length() ? defName : name);

    return headerName;
  };

  //---

  CQChartsTableTip tableTip;

  auto *srcObj  = edge()->srcNode ()->obj();
  auto *destObj = edge()->destNode()->obj();

  auto srcName  = srcObj ->hierName();
  auto destName = destObj->hierName();

  if (srcName  == "") srcName  = srcObj ->id();
  if (destName == "") destName = destObj->id();

  //---

  if (sankeyPlot_->isEdgeRows()) {
    tableTip.addTableRow(namedColumn("From"), srcName);
    tableTip.addTableRow(namedColumn("To"  ), destName);
  }
  else {
    tableTip.addTableRow("From", srcName);
    tableTip.addTableRow("To"  , destName);
  }

  if (edge()->hasValue()) {
    auto valueName = (sankeyPlot_->isEdgeRows() ? namedColumn("Value") : "Value");

    tableTip.addTableRow(valueName, edge()->value().real());
  }

#ifdef CQCHARTS_GRAPH_PATH_ID
  if (pathId() >= 0) {
    tableTip.addTableRow(namedColumn("PathId", "Path Id"), pathId());
  }
#endif

  //---

  sankeyPlot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

//---

void
CQChartsSankeyEdgeObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = (path.length() ? path + "/" : ""); path1 += propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "value" )->setDesc("Value");
#ifdef CQCHARTS_GRAPH_PATH_ID
  model->addProperty(path1, this, "pathId")->setDesc("Path Id");
#endif
}

//---

bool
CQChartsSankeyEdgeObj::
inside(const Point &p) const
{
  return edgePath_.contains(p.qpoint());
}

//---

void
CQChartsSankeyEdgeObj::
getObjSelectIndices(Indices &inds) const
{
  addColumnsSelectIndex(inds, sankeyPlot_->modelColumns());
}

CQChartsSankeyEdgeObj::PlotObjs
CQChartsSankeyEdgeObj::
getConnected() const
{
  PlotObjs plotObjs;

  auto *srcObj  = edge()->srcNode ()->obj();
  auto *destObj = edge()->destNode()->obj();

  plotObjs.push_back(srcObj);
  plotObjs.push_back(destObj);

  return plotObjs;
}

//---

void
CQChartsSankeyEdgeObj::
draw(PaintDevice *device) const
{
  // get edge path
  bool rc1 = calcEdgePath(linePath_, /*isLine*/true );
  bool rc2 = calcEdgePath(edgePath_, /*isLine*/false);
  if (! rc1 && ! rc2) return;

  //---

  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw path
  device->setColorNames();

  if (! edge()->isLine())
    device->drawPath(edgePath_);
  else
    device->strokePath(linePath_, penBrush.pen);

  device->resetColorNames();

  //---

  // show source and destination nodes on inside
  if (sankeyPlot()->drawLayerType() == CQChartsLayer::Type::MOUSE_OVER) {
    if (sankeyPlot()->mouseColoring() != CQChartsSankeyPlot::ConnectionType::NONE) {
      sankeyPlot()->view()->setDrawLayerType(CQChartsLayer::Type::MOUSE_OVER_EXTRA);

      drawConnectionMouseOver(device, static_cast<int>(sankeyPlot()->mouseColoring()));

      sankeyPlot()->view()->setDrawLayerType(CQChartsLayer::Type::MOUSE_OVER);
    }
  }

  //---

  using TextVisibleType = CQChartsSankeyPlot::TextVisibleType;

  bool nameVisible = true;

  if (! sankeyPlot_->isEdgeTextVisible()) {
    nameVisible = false;

    if ((uint(sankeyPlot_->edgeTextInsideVisible()) & uint(TextVisibleType::NAME)) &&
        isInside())
      nameVisible = true;

    if ((uint(sankeyPlot_->edgeTextSelectedVisible()) & uint(TextVisibleType::NAME)) &&
        isSelected())
      nameVisible = true;
  }

  if (nameVisible)
    drawFgText(device);

  //---

  bool valueVisible = sankeyPlot_->isEdgeValueLabel();

  if ((uint(sankeyPlot_->edgeTextInsideVisible()) & uint(TextVisibleType::VALUE)) &&
      isInside())
    valueVisible = true;

  if ((uint(sankeyPlot_->edgeTextSelectedVisible()) & uint(TextVisibleType::VALUE)) &&
      isSelected())
    valueVisible = true;

  if (valueVisible) {
    auto p = Point(linePath_.pointAtPercent(0.5));

    drawValueLabel(device, p);
  }
}

void
CQChartsSankeyEdgeObj::
drawValueLabel(PaintDevice *device, const Point &p) const
{
  if (! edge()->hasValue())
    return;

  setTextPen(device);

  auto value = edge()->value().real();

  auto str = QString::number(value);

  auto textOptions = sankeyPlot()->edgeTextOptions();

  textOptions.align = Qt::AlignHCenter | Qt::AlignVCenter;

  CQChartsDrawUtil::drawTextAtPoint(device, p, str, textOptions);
}

void
CQChartsSankeyEdgeObj::
setTextPen(PaintDevice *device) const
{
  // set font
  sankeyPlot_->setPainterFont(device, sankeyPlot_->edgeTextFont());

  // set text pen
  auto ic = calcColorInd();

  PenBrush penBrush;

  auto c = sankeyPlot_->interpEdgeTextColor(ic);

  sankeyPlot_->setPen(penBrush, PenData(true, c, sankeyPlot_->edgeTextAlpha()));

  device->setPen(penBrush.pen);
}

void
CQChartsSankeyEdgeObj::
drawConnectionMouseOver(PaintDevice *device, int imouseColoring) const
{
  auto mouseColoring = static_cast<CQChartsSankeyPlot::ConnectionType>(imouseColoring);

  //---

  auto *edge = this->edge();

  auto drawNodeInside = [&](const Node *node, bool isSrc) {
    auto *nodeObj = node->obj(); if (! nodeObj) return;

    auto rect = (isSrc ? node->destEdgeRect(edge) : node->srcEdgeRect(edge));
    if (! rect.isSet()) return;

    bool oldEnabled = nodeObj->setNotificationsEnabled(false);

    nodeObj->setInside(true);

    if (sankeyPlot()->isMouseNodeColoring())
      nodeObj->draw(device);

    if (! sankeyPlot()->isNodeTextVisible())
      nodeObj->drawFgRect(device, rect);

    nodeObj->setInside(false);

    (void) nodeObj->setNotificationsEnabled(oldEnabled);
  };

  //---

  auto *srcNode  = edge->srcNode ();
  auto *destNode = edge->destNode();

  if      (mouseColoring == CQChartsSankeyPlot::ConnectionType::SRC) {
    drawNodeInside(srcNode , /*isSrc*/true );
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::DEST) {
    drawNodeInside(destNode, /*isSrc*/false);
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::SRC_DEST) {
    drawNodeInside(srcNode , /*isSrc*/true );
    drawNodeInside(destNode, /*isSrc*/false);
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_SRC) {
    drawNodeInside(srcNode , /*isSrc*/true );

#ifdef CQCHARTS_GRAPH_PATH_ID
    auto *srcNodeObj = srcNode->obj();

    if (srcNodeObj)
      srcNodeObj->drawConnectionMouseOver(device,
        static_cast<int>(CQChartsSankeyPlot::ConnectionType::ALL_SRC), pathId());
#endif
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_DEST) {
    drawNodeInside(destNode, /*isSrc*/false);

#ifdef CQCHARTS_GRAPH_PATH_ID
    auto *destNodeObj = destNode->obj();

    if (destNodeObj)
      destNodeObj->drawConnectionMouseOver(device,
        static_cast<int>(CQChartsSankeyPlot::ConnectionType::ALL_DEST), pathId());
#endif
  }
  else if (mouseColoring == CQChartsSankeyPlot::ConnectionType::ALL_SRC_DEST) {
    drawNodeInside(srcNode , /*isSrc*/true );
    drawNodeInside(destNode, /*isSrc*/false);

#ifdef CQCHARTS_GRAPH_PATH_ID
    auto *srcNodeObj = srcNode->obj();

    if (srcNodeObj)
      srcNodeObj->drawConnectionMouseOver(device,
        static_cast<int>(CQChartsSankeyPlot::ConnectionType::ALL_SRC), pathId());

    auto *destNodeObj = destNode->obj();

    if (destNodeObj)
      destNodeObj->drawConnectionMouseOver(device,
        static_cast<int>(CQChartsSankeyPlot::ConnectionType::ALL_DEST), pathId());
#endif
  }
}

bool
CQChartsSankeyEdgeObj::
calcEdgePath(QPainterPath &path, bool isLine) const
{
  return edge()->edgePath(path, isLine);
}

// for edge text
void
CQChartsSankeyEdgeObj::
drawFgText(PaintDevice *device) const
{
  auto str = edge()->label();

  if (! str.length())
    return;

  //---

  // get connection rect of source and destination object
  auto *srcObj  = edge()->srcNode ()->obj();
  auto *destObj = edge()->destNode()->obj();

  bool isSelf = (srcObj == destObj);

  auto srcRect  = srcObj ->node()->destEdgeRect(edge());
  auto destRect = destObj->node()->srcEdgeRect (edge());

  if (! srcRect.isSet() || ! destRect.isSet())
    return;

  auto rect = (isSelf ? srcRect : srcRect + destRect);

  auto prect = sankeyPlot()->windowToPixel(rect);

  //---

  setTextPen(device);

  //---

  QFontMetricsF fm(device->font());

  double pTextMargin = 4; // pixels

  auto ptw = fm.horizontalAdvance(str);

  double tx = prect.getXMid() - pTextMargin - ptw/2.0;
  double ty = prect.getYMid() + (fm.ascent() - fm.descent())/2;

  auto pt = sankeyPlot()->pixelToWindow(Point(tx, ty));

  // only support contrast
  auto textOptions = sankeyPlot()->edgeTextOptions();

  textOptions.angle = CQChartsAngle();
  textOptions.align = Qt::AlignLeft;

  CQChartsDrawUtil::drawTextAtPoint(device, pt, str, textOptions);
}

void
CQChartsSankeyEdgeObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  // set fill and stroke
  auto *srcNode  = edge()->srcNode ();
  auto *destNode = edge()->destNode();

  //---

  ColorInd colorInd;

#ifdef CQCHARTS_GRAPH_PATH_ID
  if (pathId() >= 0) {
    const auto &pathIdMinMax = sankeyPlot()->pathIdMinMax();

    if (pathIdMinMax.isSet())
      colorInd = ColorInd(pathId() - pathIdMinMax.min(),
                          pathIdMinMax.max() - pathIdMinMax.min() + 1);
  }
#endif

  //---

  // calc fill color
  QColor fillColor;

  if (! edge()->fillColor().isValid()) {
    if (sankeyPlot()->blendEdgeColor() == CQChartsSankeyPlot::BlendType::FILL_AVERAGE) {
      auto fillColor1 = srcNode ->obj()->calcFillColor();
      auto fillColor2 = destNode->obj()->calcFillColor();

      fillColor = CQChartsUtil::blendColors(fillColor1, fillColor2, 0.5);
    }
    else
      fillColor = sankeyPlot()->interpEdgeFillColor(colorInd);
  }
  else {
    fillColor = sankeyPlot()->interpColor(edge()->fillColor(), colorInd);
  }

  // calc fill pattern
  auto fillPattern = sankeyPlot()->edgeFillPattern();

  if (sankeyPlot()->blendEdgeColor() == CQChartsSankeyPlot::BlendType::FILL_GRADIENT) {
    auto fillColor1 = srcNode ->obj()->calcFillColor();
    auto fillColor2 = destNode->obj()->calcFillColor();

    fillColor = fillColor1;

    CQChartsDrawUtil::setColorAlpha(fillColor2, sankeyPlot()->edgeFillAlpha());

    fillPattern.setType    (CQChartsFillPattern::Type::LGRADIENT);
    fillPattern.setAltColor(Color(fillColor2));
  }

  //---

  // calc stroke color
  QColor sc;

  if (sankeyPlot()->blendEdgeColor() == CQChartsSankeyPlot::BlendType::STROKE_AVERAGE) {
    int numNodes = sankeyPlot()->numNodes();

    ColorInd ic1(srcNode ->id(), numNodes);
    ColorInd ic2(destNode->id(), numNodes);

    auto sc1 = sankeyPlot()->interpEdgeStrokeColor(ic1);
    auto sc2 = sankeyPlot()->interpEdgeStrokeColor(ic2);

    sc = CQChartsUtil::blendColors(sc1, sc2, 0.5);
  }
  else {
    sc = sankeyPlot()->interpEdgeStrokeColor(colorInd);
  }

  //---

  sankeyPlot()->setPenBrush(penBrush,
    sankeyPlot()->edgePenData(sc), sankeyPlot()->edgeBrushData(fillColor, Alpha(), fillPattern));

  if (updateState)
    sankeyPlot()->updateObjPenBrushState(this, penBrush);
}

//------

CQChartsSankeyPlotGraph::
CQChartsSankeyPlotGraph(const SankeyPlot *sankeyPlot) :
 sankeyPlot_(sankeyPlot)
{
}

void
CQChartsSankeyPlotGraph::
addNode(Node *node)
{
  nodes_.push_back(node);
}

//---

void
CQChartsSankeyPlotGraph::
addDepthNode(int depth, Node *node)
{
  depthNodesMap_[depth].nodes.push_back(node);
}

void
CQChartsSankeyPlotGraph::
addDepthSize(int depth, double size)
{
  depthNodesMap_[depth].size += size;
}

void
CQChartsSankeyPlotGraph::
addPosNode(Node *node)
{
  posNodesMap_[node->pos()].push_back(node);
}

bool
CQChartsSankeyPlotGraph::
hasPosNodes(int pos) const
{
  return (posNodesMap_.find(pos) != posNodesMap_.end());
}

const CQChartsSankeyPlotGraph::Nodes &
CQChartsSankeyPlotGraph::
posNodes(int pos) const
{
  auto p = posNodesMap_.find(pos);
  assert(p != posNodesMap_.end());

  return (*p).second;
}

//---

void
CQChartsSankeyPlotGraph::
setRect(const BBox &rect)
{
  assert(rect.isSet());

  // rect is always from nodes so adjust nodes to give rect
  updateRect();

  double fx = 1.0, fy = 1.0;

  if (sankeyPlot_->isHorizontal())
    fy = (rect_.getHeight() > 0.0 ? rect.getHeight()/rect_.getHeight() : 1.0);
  else
    fx = (rect_.getWidth () > 0.0 ? rect.getWidth ()/rect_.getWidth () : 1.0);

  scale(fx, fy);

  updateRect();

  double dx = 0.0, dy = 0.0;

  if (sankeyPlot_->isHorizontal())
    dy = rect.getYMin() - rect_.getYMin();
  else
    dx = rect.getXMin() - rect_.getXMin();

  moveBy(Point(dx, dy));

  updateRect();
}

void
CQChartsSankeyPlotGraph::
updateRect()
{
  BBox bbox;

  for (const auto &node : nodes())
    bbox += node->rect();

  rect_ = bbox;
}

//---

CQChartsSankeyPlotGraph::Nodes
CQChartsSankeyPlotGraph::
placeableNodes() const
{
  Nodes nodes;

  for (auto &node : this->nodes())
    nodes.push_back(node);

  return nodes;
}

void
CQChartsSankeyPlotGraph::
moveBy(const Point &delta)
{
  Point delta1(delta.x, delta.y);

  rect_.moveBy(delta1);

  for (const auto &node : nodes())
    node->moveBy(delta1);
}

void
CQChartsSankeyPlotGraph::
scale(double fx, double fy)
{
  auto p = rect().getCenter();

  for (const auto &node : nodes()) {
    auto p1 = node->rect().getCenter();

    double xc = p.x + (p1.x - p.x)*fx;
    double yc = p.y + (p1.y - p.y)*fy;

    node->moveBy(Point(xc - p1.x, yc - p1.y));
  }
}

//------

CQChartsSankeyPlotCustomControls::
CQChartsSankeyPlotCustomControls(CQCharts *charts) :
 CQChartsConnectionPlotCustomControls(charts, "sankey")
{
}

void
CQChartsSankeyPlotCustomControls::
init()
{
  addWidgets();

  addOverview();

  addLayoutStretch();

  connectSlots(true);
}

void
CQChartsSankeyPlotCustomControls::
addWidgets()
{
  addConnectionColumnWidgets();

  addColorColumnWidgets("Node/Edge Color");
}

void
CQChartsSankeyPlotCustomControls::
connectSlots(bool b)
{
  CQChartsConnectionPlotCustomControls::connectSlots(b);
}

void
CQChartsSankeyPlotCustomControls::
setPlot(CQChartsPlot *plot)
{
  sankeyPlot_ = dynamic_cast<CQChartsSankeyPlot *>(plot);

  CQChartsConnectionPlotCustomControls::setPlot(plot);
}

void
CQChartsSankeyPlotCustomControls::
updateWidgets()
{
  CQChartsConnectionPlotCustomControls::updateWidgets();
}

CQChartsColor
CQChartsSankeyPlotCustomControls::
getColorValue()
{
  return sankeyPlot_->nodeFillColor();
}

void
CQChartsSankeyPlotCustomControls::
setColorValue(const CQChartsColor &c)
{
  sankeyPlot_->setNodeFillColor(c);
}
