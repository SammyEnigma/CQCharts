#include <CQChartsForceDirectedPlot.h>
#include <CQChartsView.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsAnalyzeModelData.h>
#include <CQChartsModelUtil.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <CQChartsNamePair.h>
#include <CQChartsPaintDevice.h>
#include <CQChartsHtml.h>

#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

CQChartsForceDirectedPlotType::
CQChartsForceDirectedPlotType()
{
}

void
CQChartsForceDirectedPlotType::
addParameters()
{
  // connections are list of node ids/counts
  startParameterGroup("Connection List");

  addColumnParameter("node", "Node", "nodeColumn").setBasic().
    setNumeric().setTip("Node Id Column");
  addColumnParameter("connections", "Connections", "connectionsColumn").setBasic().
    setTip("List of Connection Pairs (Ids from id column and connection count)").setDiscriminator();

  endParameterGroup();

  //---

  // connections are id pairs and counts
  startParameterGroup("Name Pair/Count");

  addColumnParameter("namePair", "Name Pair", "namePairColumn").setBasic().
    setTip("Connected Name Pairs (<name1>/<name2>)").setDiscriminator();
  addColumnParameter("count", "Count", "countColumn").setBasic().
    setNumeric().setTip("Connection Count");

  endParameterGroup();

  //---

  startParameterGroup("General");

  addColumnParameter("name", "Name", "nameColumn").
    setString().setTip("Optional node name");

  addColumnParameter("groupId", "Group Id", "groupIdColumn").
    setNumeric().setTip("Group Id for Color");

  endParameterGroup();

  //---

  CQChartsPlotType::addParameters();
}

QString
CQChartsForceDirectedPlotType::
description() const
{
  auto B    = [](const QString &str) { return CQChartsHtml::Str::bold(str); };
  auto PARM = [](const QString &str) { return CQChartsHtml::Str::angled(str); };
  auto LI   = [](const QString &str) { return CQChartsHtml::Str(str); };
//auto BR   = []() { return CQChartsHtml::Str(CQChartsHtml::Str::Type::BR); };
  auto IMG  = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Force Directed Plot").
    h3("Summary").
     p("Draw connected data using animated nodes connected by springs.").
    h3("Columns").
     p("Connection information can be supplied using:").
     ul({ LI("A list of connections in the " + B("Connections") + " column with the "
             "associated node numbers in the " + B("Node") + " column."),
          LI("A name pair using " + B("NamePair") + " column and a count using the " +
             B("Count") + " column.") }).
     p("The connections column is in the form {{" + PARM("id") + " " + PARM("count") + "} ...}.").
     p("The name pair column is in the form " + PARM("id1") + "/" + PARM("id2")).
     p("The column id is taken from the " + B("Id") + " column and an optional "
       "name for the id can be supplied in the " + B("Name") + " column.").
     p("The group is specified using the " + B("Group") + " column.").
    h3("Styling").
     p("The styling (fill, stroke) of the nodes and edges can be set").
    h3("Limitations").
     p("The plot does not support axes, key or logarithmic scales.").
    h3("Example").
     p(IMG("images/forcedirected.png"));
}

bool
CQChartsForceDirectedPlotType::
isColumnForParameter(CQChartsModelColumnDetails *columnDetails,
                     CQChartsPlotParameter *parameter) const
{
  if      (parameter->name() == "connections") {
    return (columnDetails->type() == CQChartsPlot::ColumnType::CONNECTION_LIST);
  }
  else if (parameter->name() == "namePair") {
    return (columnDetails->type() == CQChartsPlot::ColumnType::NAME_PAIR);
  }

  return CQChartsPlotType::isColumnForParameter(columnDetails, parameter);
}

void
CQChartsForceDirectedPlotType::
analyzeModel(CQChartsModelData *modelData, CQChartsAnalyzeModelData &analyzeModelData)
{
  bool hasNode        = (analyzeModelData.parameterNameColumn.find("node") !=
                         analyzeModelData.parameterNameColumn.end());
  bool hasConnections = (analyzeModelData.parameterNameColumn.find("connections") !=
                         analyzeModelData.parameterNameColumn.end());
  bool hasNamePair    = (analyzeModelData.parameterNameColumn.find("namePair") !=
                         analyzeModelData.parameterNameColumn.end());
  bool hasCount       = (analyzeModelData.parameterNameColumn.find("count") !=
                         analyzeModelData.parameterNameColumn.end());

  if (hasConnections || hasNamePair)
    return;

  CQChartsModelDetails *details = modelData->details();
  if (! details) return;

  CQChartsColumn connectionsColumn;
  CQChartsColumn namePairColumn;
  CQChartsColumn countColumn;
  CQChartsColumn nodeColumn;

  int nc = details->numColumns();

  for (int c = 0; c < nc; ++c) {
    auto columnDetails = details->columnDetails(CQChartsColumn(c));
    if (! columnDetails) continue;

    if      (columnDetails->type() == ColumnType::STRING) {
      if (! connectionsColumn.isValid()) {
        QModelIndex parent;

        bool ok;

        QString str =
          CQChartsModelUtil::modelString(modelData->charts(), modelData->model().data(),
                                         0, columnDetails->column(), parent, ok);
        if (! ok) continue;

        CQChartsConnectionList::Connections connections;

        if (CQChartsConnectionList::stringToConnections(str, connections))
          connectionsColumn = columnDetails->column();
      }

      if (! namePairColumn.isValid()) {
        QModelIndex parent;

        bool ok;

        QString str =
          CQChartsModelUtil::modelString(modelData->charts(), modelData->model().data(),
                                         0, columnDetails->column(), parent, ok);
        if (! ok) continue;

        CQChartsNamePair::Names names;

        if (CQChartsNamePair::stringToNames(str, names))
          namePairColumn = columnDetails->column();
      }
    }
    else if (columnDetails->isNumeric()) {
      if (! countColumn.isValid())
        countColumn = columnDetails->column();

      if (! nodeColumn.isValid())
        nodeColumn = columnDetails->column();
    }
  }

  if (! hasConnections && connectionsColumn.isValid()) {
    analyzeModelData.parameterNameColumn["connections"] = connectionsColumn;

    hasConnections = true;
  }

  if (! hasNamePair && namePairColumn.isValid()) {
    analyzeModelData.parameterNameColumn["namePair"] = namePairColumn;

    hasNamePair = true;
  }

  if (hasConnections && ! hasNode && nodeColumn.isValid())
    analyzeModelData.parameterNameColumn["node"] = nodeColumn;

  if (hasNamePair && ! hasCount && countColumn.isValid())
    analyzeModelData.parameterNameColumn["count"] = countColumn;
}

CQChartsPlot *
CQChartsForceDirectedPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsForceDirectedPlot(view, model);
}

//---

CQChartsForceDirectedPlot::
CQChartsForceDirectedPlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("forcedirected"), model),
 CQChartsObjNodeShapeData<CQChartsForceDirectedPlot>(this),
 CQChartsObjEdgeLineData <CQChartsForceDirectedPlot>(this)
{
  NoUpdate noUpdate(this);

  forceDirected_ = new CQChartsForceDirected;

  setOuterMargin(0, 0, 0, 0);

  setNodeStrokeAlpha(CQChartsAlpha(0.5));
}

CQChartsForceDirectedPlot::
~CQChartsForceDirectedPlot()
{
  delete forceDirected_;
}

//---

void
CQChartsForceDirectedPlot::
setNodeColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(nodeColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsForceDirectedPlot::
setConnectionsColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(connectionsColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsForceDirectedPlot::
setNameColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(nameColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsForceDirectedPlot::
setNamePairColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(namePairColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsForceDirectedPlot::
setCountColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(countColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsForceDirectedPlot::
setGroupIdColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(groupIdColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsForceDirectedPlot::
setRunning(bool b)
{
  running_ = b;
}

void
CQChartsForceDirectedPlot::
setNodeRadius(double r)
{
  CQChartsUtil::testAndSet(nodeRadius_, r, [&]() { drawObjs(); } );
}

void
CQChartsForceDirectedPlot::
setEdgeLinesValueWidth(bool b)
{
  CQChartsUtil::testAndSet(edgeLinesValueWidth_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsForceDirectedPlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  addBaseProperties();

  // columns
  addProp("columns", "nodeColumn"       , "node"      , "Node column");
  addProp("columns", "connectionsColumn", "connection", "Connections column");
  addProp("columns", "nameColumn"       , "name"      , "Name column");

  addProp("columns", "namePairColumn", "namePair", "Name/pair column");
  addProp("columns", "countColumn"   , "count"   , "Count column");

  addProp("columns", "groupIdColumn", "groupId", "Group id column");

  // options
  addProp("options", "running", "", "Is running");

  // node/edge
  addProp("node", "nodeRadius", "radius", "Node radius in pixels")->setMinValue(0.0);

  addFillProperties("node/fill"  , "nodeFill"  , "Node");
  addLineProperties("node/stroke", "nodeStroke", "Node");
  addLineProperties("edge/stroke", "edgeLines" , "Edge");
}

CQChartsGeom::Range
CQChartsForceDirectedPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsForceDirectedPlot::calcRange");

  auto *th = const_cast<CQChartsForceDirectedPlot *>(this);

  //---

  // check columns
  bool columnsValid = true;

  th->clearErrors();

  // node, connections, name/pair, count, name and groupId columns optional
  if (! checkColumn(connectionsColumn(), "Connections", th->connectionsColumnType_))
    columnsValid = false;
  if (! checkColumn(namePairColumn(), "Name/Pair", th->namePairColumnType_))
    columnsValid = false;

  if (! checkColumn(nodeColumn   (), "Node"    )) columnsValid = false;
  if (! checkColumn(countColumn  (), "Count"   )) columnsValid = false;
  if (! checkColumn(nameColumn   (), "Name"    )) columnsValid = false;
  if (! checkColumn(groupIdColumn(), "Group Id")) columnsValid = false;

  if (! columnsValid)
    return CQChartsGeom::Range(0.0, 0.0, 1.0, 1.0);

  //---

  // TODO: calculate good range size from data or auto scale/fit ?
  CQChartsGeom::Range dataRange;

  dataRange.updateRange(-rangeSize_, -rangeSize_);
  dataRange.updateRange( rangeSize_,  rangeSize_);

  return dataRange;
}

bool
CQChartsForceDirectedPlot::
createObjs(PlotObjs &) const
{
  CQPerfTrace trace("CQChartsForceDirectedPlot::createObjs");

  NoUpdate noUpdate(this);

  //---

  if (! idConnections_.empty())
    return false;

  //---

  auto *th = const_cast<CQChartsForceDirectedPlot *>(this);

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsForceDirectedPlot *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      CQChartsModelIndex groupModelInd(data.row, plot_->groupIdColumn(), data.parent);

      bool ok1;

      int group = (int) plot_->modelInteger(groupModelInd, ok1);

      if (! ok1) group = data.row;

      //---

      if (plot_->connectionsColumn().isValid()) {
        ConnectionsData connectionsData;

        if (! plot_->getRowConnections(group, data, connectionsData))
          return State::SKIP;

        plot_->addConnections(connectionsData.node, connectionsData);
      }
      else {
        ConnectionsData connectionsData;
        int             destId, count;

        if (! plot_->getNameConnections(group, data, connectionsData, destId, count))
          return State::SKIP;

        auto *plot = const_cast<CQChartsForceDirectedPlot *>(plot_);

        ConnectionsData &srcConnectionsData = plot->getConnections(connectionsData.node);

        srcConnectionsData = connectionsData;

        CQChartsConnectionList::Connection connection;

        connection.node  = destId;
        connection.count = count;

        (void) plot_->getConnections(connection.node);

        srcConnectionsData.connections.push_back(connection);
      }

      //---

      maxGroup_ = std::max(maxGroup_, group);

      return State::OK;
    }

    int maxGroup() const { return maxGroup_; }

   private:
    const CQChartsForceDirectedPlot* plot_     { nullptr };
    int                              maxGroup_ { 0 };
  };

  th->nameNodeMap_.clear();

  RowVisitor visitor(this);

  visitModel(visitor);

  int maxGroup = visitor.maxGroup();

  //---

  th->nodes_.clear();

  for (const auto &idConnections : idConnections_) {
    int            id    = idConnections.first;
    const QString &name  = idConnections.second.name;
    int            group = idConnections.second.group;

    Springy::Node *node = forceDirected_->newNode();

    QString label = QString("%1:%2").arg(name).arg(group);

    node->setLabel(label);
    node->setMass (nodeMass_);
    node->setValue((1.0*group)/maxGroup);

    th->nodes_[id] = node;
  }

  //---

  for (const auto &idConnections : idConnections_) {
    int                    id          = idConnections.first;
    const ConnectionsData &connections = idConnections.second;

    auto pn = nodes_.find(id);
    assert(pn != nodes_.end());

    Springy::Node *node = (*pn).second;
    assert(node);

    for (const auto &connection : connections.connections) {
      auto pn1 = nodes_.find(connection.node);
      assert(pn1 != nodes_.end());

      Springy::Node *node1 = (*pn1).second;
      assert(node1);

      Springy::Edge *edge = forceDirected_->newEdge(node, node1);

      edge->setLength(1.0/connection.count);
      edge->setValue(connection.count);
    }
  }

  //---

  for (int i = 0; i < initSteps_; ++i)
    forceDirected_->step(stepSize_);

  //---

  return true;
}

bool
CQChartsForceDirectedPlot::
getRowConnections(int group, const ModelVisitor::VisitData &data,
                  ConnectionsData &connectionsData) const
{
  // get node
  CQChartsModelIndex nodeModelInd(data.row, nodeColumn(), data.parent);

  bool ok2;

  int id = (int) modelInteger(nodeModelInd, ok2);

  if (! ok2) id = data.row;

  //---

  // get connections
  CQChartsModelIndex connectionsModelInd(data.row, connectionsColumn(), data.parent);

  bool ok3;

  if (connectionsColumnType_ == ColumnType::CONNECTION_LIST) {
    QVariant connectionsVar = modelValue(connectionsModelInd, ok3);

    connectionsData.connections = connectionsVar.value<CQChartsConnectionList>().connections();
  }
  else {
    QString connectionsStr = modelString(connectionsModelInd, ok3);

    if (! ok3)
      return false;

    decodeConnections(connectionsStr, connectionsData.connections);
  }

  //---

  // get name
  CQChartsModelIndex nameModelInd(data.row, nameColumn(), data.parent);

  bool ok4;

  QString name = modelString(nameModelInd, ok4);

  if (! name.length())
    name = QString("%1").arg(id);

  //---

  // return connections data
  QModelIndex nodeInd  = modelIndex(nodeModelInd);
  QModelIndex nodeInd1 = normalizeIndex(nodeInd);

  connectionsData.ind   = nodeInd1;
  connectionsData.node  = id;
  connectionsData.name  = name;
  connectionsData.group = group;

  return true;
}

bool
CQChartsForceDirectedPlot::
getNameConnections(int group, const ModelVisitor::VisitData &data,
                   ConnectionsData &connections, int &destId, int &count) const
{
  CQChartsModelIndex namePairModelInd(data.row, namePairColumn(), data.parent);

  bool ok2;

  CQChartsNamePair namePair;

  if (namePairColumnType_ == ColumnType::NAME_PAIR) {
    QVariant namePairVar = modelValue(namePairModelInd, ok2);

    if (! ok2)
      return false;

    namePair = namePairVar.value<CQChartsNamePair>();
  }
  else {
    QString namePairStr = modelString(namePairModelInd, ok2);

    if (! ok2)
      return false;

    namePair = CQChartsNamePair(namePairStr);
  }

  if (! namePair.isValid())
    return false;

  //---

  CQChartsModelIndex countModelInd(data.row, countColumn(), data.parent);

  bool ok3;

  count = (int) modelInteger(countModelInd, ok3);

  if (! ok3)
    count = 0;

  //---

  QString srcStr  = namePair.name1();
  QString destStr = namePair.name2();

  int srcId  = getStringId(srcStr);
      destId = getStringId(destStr);

  //---

  QModelIndex nameInd  = modelIndex(namePairModelInd);
  QModelIndex nameInd1 = normalizeIndex(nameInd);

  connections.ind   = nameInd1;
  connections.node  = srcId;
  connections.name  = srcStr;
  connections.group = group;

  return true;
}

const CQChartsForceDirectedPlot::ConnectionsData &
CQChartsForceDirectedPlot::
getConnections(int id) const
{
  auto *th = const_cast<CQChartsForceDirectedPlot *>(this);

  return th->getConnections(id);
}

CQChartsForceDirectedPlot::ConnectionsData &
CQChartsForceDirectedPlot::
getConnections(int id)
{
  auto p = idConnections_.find(id);

  if (p != idConnections_.end())
    return (*p).second;

  //---

  ConnectionsData data;

  data.node = id;

  p = idConnections_.insert(p, IdConnectionsData::value_type(id, data));

  return (*p).second;
}

void
CQChartsForceDirectedPlot::
addConnections(int id, const ConnectionsData &connections) const
{
  auto *th = const_cast<CQChartsForceDirectedPlot *>(this);

  th->idConnections_[id] = connections;
}

bool
CQChartsForceDirectedPlot::
decodeConnections(const QString &str, Connections &connections) const
{
  return CQChartsConnectionList::stringToConnections(str, connections);
}

int
CQChartsForceDirectedPlot::
getStringId(const QString &str) const
{
  auto p = nameNodeMap_.find(str);

  if (p != nameNodeMap_.end())
    return (*p).second;

  //---

  int id = nameNodeMap_.size();

  auto *th = const_cast<CQChartsForceDirectedPlot *>(this);

  auto p1 = th->nameNodeMap_.insert(th->nameNodeMap_.end(), StringIndMap::value_type(str, id));

  return (*p1).second;
}

//---

void
CQChartsForceDirectedPlot::
postUpdateObjs()
{
  startAnimateTimer();
}

void
CQChartsForceDirectedPlot::
animateStep()
{
  if (pressed_ || ! isRunning())
    return;

  forceDirected_->step(stepSize_);

  if (isAutoFit()) {
    double xmin { 0.0 }, ymin { 0.0 }, xmax { 0.0 }, ymax { 0.0 };

    double r = std::max(nodeRadius(), 0.0);

    double xm = pixelToWindowWidth (2*r);
    double ym = pixelToWindowHeight(2*r);

    forceDirected_->calcRange(xmin, ymin, xmax, ymax);

    dataRange_.updateRange(xmin - xm, ymin - ym);
    dataRange_.updateRange(xmax + xm, ymax + ym);

    applyDataRange();
  }

  drawObjs();
}

bool
CQChartsForceDirectedPlot::
selectPress(const CQChartsGeom::Point &p, SelMod /*selMod*/)
{
  Springy::NodePoint nodePoint = forceDirected_->nearest(Springy::Vector(p.x, p.y));

  forceDirected_->setCurrentNode (nodePoint.first );
  forceDirected_->setCurrentPoint(nodePoint.second);

  pressed_ = true;

  drawObjs();

  return true;
}

bool
CQChartsForceDirectedPlot::
selectMove(const CQChartsGeom::Point &p, bool first)
{
  if (pressed_) {
    if (forceDirected_->currentPoint())
      forceDirected_->currentPoint()->setP(Springy::Vector(p.x, p.y));

    drawObjs();

    return true;
  }
  else {
    Springy::NodePoint nodePoint = forceDirected_->nearest(Springy::Vector(p.x, p.y));

    forceDirected_->setCurrentNode (nodePoint.first );
    forceDirected_->setCurrentPoint(nodePoint.second);
  }

  return CQChartsPlot::selectMove(p, first);
}

bool
CQChartsForceDirectedPlot::
selectRelease(const CQChartsGeom::Point &p)
{
  if (forceDirected_->currentPoint())
    forceDirected_->currentPoint()->setP(Springy::Vector(p.x, p.y));

  forceDirected_->setCurrentNode (0);
  forceDirected_->setCurrentPoint(0);

  pressed_ = false;

  drawObjs();

  return true;
}

void
CQChartsForceDirectedPlot::
keyPress(int key, int modifier)
{
  if (key == Qt::Key_S)
    setRunning(! isRunning());
  else
    CQChartsPlot::keyPress(key, modifier);
}

bool
CQChartsForceDirectedPlot::
tipText(const CQChartsGeom::Point &p, QString &tip) const
{
  if (! isRunning()) {
    Springy::NodePoint nodePoint = forceDirected_->nearest(Springy::Vector(p.x, p.y));

    if (! nodePoint.first)
      return false;

    tip = nodePoint.first->label();

    return true;
  }

  return false;
}

void
CQChartsForceDirectedPlot::
draw(QPainter *painter)
{
  drawParts(painter);

  //---

  {
  LockMutex lock(this, "draw");

  UpdateState updateState = this->updateState();

  if (updateState == UpdateState::READY) {
    setGroupedUpdateState(UpdateState::DRAWN);
  }
  }
}

void
CQChartsForceDirectedPlot::
drawParts(QPainter *painter) const
{
  auto *th = const_cast<CQChartsForceDirectedPlot *>(this);

  CQChartsPlotPainter device(th, painter);

  drawDeviceParts(&device);
}

void
CQChartsForceDirectedPlot::
drawDeviceParts(CQChartsPaintDevice *device) const
{
  device->save();

  setClipRect(device);

  // draw edges
  QPen edgePen;

  QColor edgeColor = this->interpEdgeLinesColor(ColorInd());

  setPen(edgePen, true, edgeColor, edgeLinesAlpha(), edgeLinesWidth(), edgeLinesDash());

  for (auto &edge : forceDirected_->edges()) {
    bool isTemp = false;

    auto spring = forceDirected_->spring(edge, isTemp);

    const Springy::Vector &p1 = spring->point1()->p();
    const Springy::Vector &p2 = spring->point2()->p();

    if (isEdgeLinesValueWidth()) {
      QPen edgePen1 = edgePen;

      double w = sqrt(edge->value());

      edgePen1.setWidthF(w);

      device->setPen(edgePen1);
    }
    else
      device->setPen(edgePen);

    device->drawLine(CQChartsGeom::Point(p1.x(), p1.y()),
                     CQChartsGeom::Point(p2.x(), p2.y()));

    if (isTemp)
      delete spring;
  }

  // draw nodes
  double r = std::max(nodeRadius(), 0.0);

  double xm = pixelToWindowWidth (2*r);
  double ym = pixelToWindowHeight(2*r);

  for (auto &node : forceDirected_->nodes()) {
    auto point = forceDirected_->point(node);

    const Springy::Vector &p1 = point->p();

    //---

    QPen   pen;
    QBrush brush;

    QColor pc = interpNodeStrokeColor(ColorInd());
    QColor fc = interpPaletteColor(ColorInd(node->value()), /*scale*/false);

    if (node == forceDirected_->currentNode())
      fc = insideColor(fc);

    setPen(pen, true, pc, nodeStrokeAlpha(), nodeStrokeWidth(), nodeStrokeDash());

    setBrush(brush, true, fc, nodeFillAlpha(), nodeFillPattern());

    device->setPen  (pen);
    device->setBrush(brush);

    //---

    CQChartsGeom::BBox ebbox(p1.x() - xm/2.0, p1.y() - ym/2.0, p1.x() + xm/2, p1.y() + ym/2.0);

    device->drawEllipse(ebbox);
  }

  //---

  device->restore();
}
