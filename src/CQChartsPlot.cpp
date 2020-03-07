#include <CQChartsPlot.h>
#include <CQChartsPlotType.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsKey.h>
#include <CQChartsTitle.h>
#include <CQChartsPlotObj.h>
#include <CQChartsPlotObjTree.h>
#include <CQChartsNoDataObj.h>
#include <CQChartsAnnotation.h>
#include <CQChartsValueSet.h>
#include <CQChartsDisplayRange.h>
#include <CQChartsModelExprMatch.h>
#include <CQChartsModelData.h>
#include <CQChartsModelDetails.h>
#include <CQChartsPlotParameter.h>
#include <CQChartsColumnType.h>
#include <CQChartsModelUtil.h>
#include <CQChartsEditHandles.h>
#include <CQChartsVariant.h>
#include <CQChartsTip.h>
#include <CQChartsPaintDevice.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsHtml.h>
#include <CQChartsEnv.h>
#include <CQCharts.h>
#include <CQChartsWidgetUtil.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQColors.h>
#include <CQColorsTheme.h>
#include <CQColorsPalette.h>
#include <CQPerfMonitor.h>
#include <CQUtil.h>
#include <CQTclUtil.h>

#include <CMathUtil.h>
#include <CMathRound.h>

#include <QApplication>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QTextBrowser>
#include <QPainter>

//------

CQChartsPlot::
CQChartsPlot(CQChartsView *view, CQChartsPlotType *type, const ModelP &model) :
 CQChartsObj(view->charts()),
 CQChartsObjPlotShapeData<CQChartsPlot>(this),
 CQChartsObjDataShapeData<CQChartsPlot>(this),
 CQChartsObjFitShapeData <CQChartsPlot>(this),
 view_(view), type_(type), model_(model)
{
  NoUpdate noUpdate(this);

  //---

  propertyModel_ = new CQPropertyViewModel;

  //---

  preview_ = CQChartsEnv::getInt("CQ_CHARTS_PLOT_PREVIEW", preview_);

  sequential_ = CQChartsEnv::getBool("CQ_CHARTS_SEQUENTIAL", sequential_);

  queueUpdate_ = CQChartsEnv::getBool("CQ_CHARTS_PLOT_QUEUE", queueUpdate_);

  bufferSymbols_ = CQChartsEnv::getInt("CQ_CHARTS_BUFFER_SYMBOLS", bufferSymbols_);

  displayRange_ = new CQChartsDisplayRange();

  displayRange_->setPixelAdjust(0.0);

  bool objTreeWait = CQChartsEnv::getBool("CQ_CHARTS_OBJ_TREE_WAIT", false);

  objTreeData_.tree = new CQChartsPlotObjTree(this, objTreeWait);

  animateData_.tickLen = CQChartsEnv::getInt("CQ_CHARTS_TICK_LEN", animateData_.tickLen);

  debugUpdate_   = CQChartsEnv::getBool("CQ_CHARTS_DEBUG_UPDATE"   , debugUpdate_  );
  debugQuadTree_ = CQChartsEnv::getBool("CQ_CHARTS_DEBUG_QUAD_TREE", debugQuadTree_);

  editHandles_ = new CQChartsEditHandles(view);

  //--

  // plot, data, fit background
  setPlotFilled(true ); setPlotStroked(false);
  setDataFilled(true ); setDataStroked(false);
  setFitFilled (false); setFitStroked (false);

  setDataClip(true);

  setPlotFillColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 0.00));
  setDataFillColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 0.12));
  setFitFillColor (CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 0.08));

  //--

  double vr = CQChartsView::viewportRange();

  viewBBox_      = CQChartsGeom::BBox(0, 0, vr, vr);
  innerViewBBox_ = viewBBox_;

  displayRange_->setPixelAdjust(0.0);

  setPixelRange(CQChartsGeom::BBox(0.0, 0.0,  vr,  vr));

  resetWindowRange();

  //---

  // all layers active except BG_PLOT and FG_PLOT
  initLayer(CQChartsLayer::Type::BACKGROUND , CQChartsBuffer::Type::BACKGROUND, true );
  initLayer(CQChartsLayer::Type::BG_AXES    , CQChartsBuffer::Type::BACKGROUND, true );
  initLayer(CQChartsLayer::Type::BG_KEY     , CQChartsBuffer::Type::BACKGROUND, true );

  initLayer(CQChartsLayer::Type::BG_PLOT    , CQChartsBuffer::Type::MIDDLE    , false);
  initLayer(CQChartsLayer::Type::MID_PLOT   , CQChartsBuffer::Type::MIDDLE    , true );
  initLayer(CQChartsLayer::Type::FG_PLOT    , CQChartsBuffer::Type::MIDDLE    , false);

  initLayer(CQChartsLayer::Type::FG_AXES    , CQChartsBuffer::Type::FOREGROUND, true );
  initLayer(CQChartsLayer::Type::FG_KEY     , CQChartsBuffer::Type::FOREGROUND, true );
  initLayer(CQChartsLayer::Type::TITLE      , CQChartsBuffer::Type::FOREGROUND, true );
  initLayer(CQChartsLayer::Type::ANNOTATION , CQChartsBuffer::Type::FOREGROUND, true );
  initLayer(CQChartsLayer::Type::FOREGROUND , CQChartsBuffer::Type::FOREGROUND, true );

  initLayer(CQChartsLayer::Type::EDIT_HANDLE, CQChartsBuffer::Type::OVERLAY   , true );
  initLayer(CQChartsLayer::Type::BOXES      , CQChartsBuffer::Type::OVERLAY   , true );
  initLayer(CQChartsLayer::Type::SELECTION  , CQChartsBuffer::Type::OVERLAY   , true );
  initLayer(CQChartsLayer::Type::MOUSE_OVER , CQChartsBuffer::Type::OVERLAY   , true );

  //---

  connectModel();

  //---

  ++updatesData_.stateFlag[UpdateState::UPDATE_RANGE    ];
  ++updatesData_.stateFlag[UpdateState::UPDATE_OBJS     ];
  ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS];

  startThreadTimer();
}

CQChartsPlot::
~CQChartsPlot()
{
  CQChartsPlot::clearPlotObjects();

  for (auto &layer : layers_)
    delete layer.second;

  for (auto &buffer : buffers_)
    delete buffer.second;

  for (auto &annotation : annotations())
    delete annotation;

  delete objTreeData_.tree;

  delete propertyModel_;

  delete displayRange_;

  delete titleObj_;
  delete keyObj_;
  delete xAxis_;
  delete yAxis_;

  delete editHandles_;

  delete animateData_.timer;
  delete updateData_.timer;
}

QString
CQChartsPlot::
viewId() const
{
  return view()->id();
}

QString
CQChartsPlot::
typeStr() const
{
  return type_->name();
}

//---

void
CQChartsPlot::
startThreadTimer()
{
  if (isSequential())
    return;

  if (isOverlay() && ! isFirstPlot())
    return;

  if (! updateData_.timer) {
    updateData_.timer = new QTimer(this);

    connect(updateData_.timer, SIGNAL(timeout()), this, SLOT(threadTimerSlot()));
  }

  if (! updateData_.timer->isActive())
    updateData_.timer->start(10);
}

void
CQChartsPlot::
stopThreadTimer()
{
  if (updateData_.timer) {
    if (updateData_.timer->isActive())
      updateData_.timer->stop();
  }
}

//---

void
CQChartsPlot::
setModel(const ModelP &model)
{
  disconnectModel();

  model_ = model;

  connectModel();

  updateRangeAndObjs();

  emit modelChanged();
}

void
CQChartsPlot::
connectModel()
{
  connectDisconnectModel(true);
}

void
CQChartsPlot::
disconnectModel()
{
  connectDisconnectModel(false);
}

void
CQChartsPlot::
connectDisconnectModel(bool isConnect)
{
  if (! model_.data())
    return;

  CQChartsModelData *modelData = getModelData();

  //---

  auto connectDisconnect = [&](bool b, QObject *obj, const char *from, const char *to) {
    CQChartsWidgetUtil::connectDisconnect(b, obj, from, this, to);
  };

  //---

  if (modelData) {
    if (isConnect) {
      if (! modelData->name().length() && this->hasId()) {
        charts()->setModelName(modelData, this->id());

        modelNameSet_ = true;
      }
      else
        modelNameSet_ = false;
    }
    else {
      if (modelNameSet_) {
        charts()->setModelName(modelData, this->id());

        modelNameSet_ = false;
      }
    }

    connectDisconnect(isConnect, modelData, SIGNAL(modelChanged()),
                      SLOT(modelChangedSlot()));

    connectDisconnect(isConnect, modelData, SIGNAL(currentModelChanged()),
                      SLOT(currentModelChangedSlot()));

    connectDisconnect(isConnect, modelData, SIGNAL(selectionChanged(QItemSelectionModel *)),
                      SLOT(selectionSlot(QItemSelectionModel *)));
  }
  else {
    modelNameSet_ = false;

    // TODO: on connect, check if model uses changed columns
    //int column1 = tl.column();
    //int column2 = br.column();

    connectDisconnect(isConnect,
                      model_.data(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                      SLOT(modelChangedSlot()));

    connectDisconnect(isConnect, model_.data(), SIGNAL(layoutChanged()),
                      SLOT(modelChangedSlot()));
    connectDisconnect(isConnect, model_.data(), SIGNAL(modelReset()),
                      SLOT(modelChangedSlot()));

    connectDisconnect(isConnect, model_.data(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                      SLOT(modelChangedSlot()));
    connectDisconnect(isConnect, model_.data(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                      SLOT(modelChangedSlot()));
    connectDisconnect(isConnect, model_.data(), SIGNAL(columnsInserted(QModelIndex,int,int)),
                      SLOT(modelChangedSlot()));
    connectDisconnect(isConnect, model_.data(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                      SLOT(modelChangedSlot()));
  }
}

//---

void
CQChartsPlot::
startAnimateTimer()
{
  if (! animateData_.timer) {
    animateData_.timer = new QTimer;

    connect(animateData_.timer, SIGNAL(timeout()), this, SLOT(animateSlot()));
  }

  animateData_.timer->start(animateData_.tickLen);
}

void
CQChartsPlot::
stopAnimateTimer()
{
  delete animateData_.timer;

  animateData_.timer = nullptr;
}

void
CQChartsPlot::
animateSlot()
{
  interruptRange();
//interruptDraw();

  animateStep();
}

//---

void
CQChartsPlot::
modelChangedSlot()
{
  updateRangeAndObjs();
}

void
CQChartsPlot::
currentModelChangedSlot()
{
  auto *modelData = qobject_cast<CQChartsModelData *>(sender());

  if (! modelData)
    return;

  setModel(modelData->currentModel());
}

//---

void
CQChartsPlot::
selectionSlot(QItemSelectionModel *sm)
{
  QModelIndexList indices = sm->selectedIndexes();
  if (indices.empty()) return;

  //---

  startSelection();

  // deselect all objects
  deselectAllObjs();

  // select objects with matching indices
  for (int i = 0; i < indices.size(); ++i) {
    const QModelIndex &ind = indices[i];

    QModelIndex ind1 = normalizeIndex(ind);

    for (auto &plotObj : plotObjects()) {
      if (plotObj->isSelectIndex(ind1))
        plotObj->setSelected(true);
    }
  }

  endSelection();

  //---

  invalidateOverlay();

  if (selectInvalidateObjs())
    drawObjs();
}

//---

CQCharts *
CQChartsPlot::
charts() const
{
  return view()->charts();
}

QString
CQChartsPlot::
typeName() const
{
  return type()->name();
}

QString
CQChartsPlot::
pathId() const
{
  return view()->id() + ":" + id();
}

//---

bool
CQChartsPlot::
isVisible() const
{
  return CQChartsObj::isVisible();
}

void
CQChartsPlot::
setVisible(bool b)
{
  CQChartsUtil::testAndSet(visible_, b, [&]() {
    if (! isVisible()) {
      if (view()->currentPlot() == this)
        view()->setCurrentPlot(nullptr);
    }
    else {
      if (! view()->currentPlot())
        view()->setCurrentPlot(this);
    }

    if (isVisible() || isOverlay())
      updateRangeAndObjs();

    if (isOverlay(/*checkVisible*/false))
      view()->initOverlayAxes();
  } );
}

void
CQChartsPlot::
setSelected(bool b)
{
  CQChartsUtil::testAndSet(selected_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsPlot::
setUpdatesEnabled(bool b, bool update)
{
  if (b) {
    assert(updatesData_.enabled > 0);

    --updatesData_.enabled;

    if (isUpdatesEnabled()) {
      if (update) {
        // calc range and objs
        if      (updatesData_.updateRangeAndObjs) {
          updateRangeAndObjs();

          drawObjs();
        }
        // calc objs
        else if (updatesData_.updateObjs) {
          updateObjs();

          drawObjs();
        }
        // apply range
        else if (updatesData_.applyDataRange) {
          applyDataRangeAndDraw();
        }
        // draw objs
        else if (updatesData_.invalidateLayers) {
          drawObjs();
        }
      }

      updatesData_.reset();
    }
  }
  else {
    if (updatesData_.enabled == 0) {
      updatesData_.reset();
    }

    ++updatesData_.enabled;
  }
}

//---

void
CQChartsPlot::
updateRange()
{
  if (isQueueUpdate()) {
    if (! isUpdatesEnabled())
      return;

    if (isOverlay() && ! isFirstPlot())
      return firstPlot()->updateRange();

    if (debugUpdate_)
      std::cerr << "updateRange : " << id().toStdString() << "\n";

    ++updatesData_.stateFlag[UpdateState::UPDATE_RANGE    ];
    ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS];

    startThreadTimer();
  }
  else {
    execUpdateRange();
  }
}

void
CQChartsPlot::
updateRangeAndObjs()
{
  if (isQueueUpdate()) {
    if (! isUpdatesEnabled())
      return;

    if (isOverlay() && ! isFirstPlot())
      return firstPlot()->updateRangeAndObjs();

    if (debugUpdate_)
      std::cerr << "updateRangeAndObjs : " << id().toStdString() << "\n";

    ++updatesData_.stateFlag[UpdateState::UPDATE_RANGE    ];
    ++updatesData_.stateFlag[UpdateState::UPDATE_OBJS     ];
    ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS];

    startThreadTimer();
  }
  else {
    execUpdateRangeAndObjs();
  }
}

void
CQChartsPlot::
updateObjs()
{
  if (isQueueUpdate()) {
    if (! isUpdatesEnabled())
      return;

    if (isOverlay() && ! isFirstPlot())
      return firstPlot()->updateObjs();

    if (debugUpdate_)
      std::cerr << "updateObjs : " << id().toStdString() << "\n";

    ++updatesData_.stateFlag[UpdateState::UPDATE_OBJS     ];
    ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS];

    startThreadTimer();
  }
  else
    execUpdateObjs();
}

void
CQChartsPlot::
drawBackground()
{
  if (isQueueUpdate()) {
    if (! isUpdatesEnabled())
      return;

    if (isOverlay() && ! isFirstPlot())
      return firstPlot()->drawBackground();

    if (debugUpdate_)
      std::cerr << "drawBackground : " << id().toStdString() << "\n";

    ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_BACKGROUND];

    startThreadTimer();
  }
  else
    invalidateLayer(CQChartsBuffer::Type::BACKGROUND);
}

void
CQChartsPlot::
drawForeground()
{
  if (isQueueUpdate()) {
    if (! isUpdatesEnabled())
      return;

    if (isOverlay() && ! isFirstPlot())
      return firstPlot()->drawForeground();

    if (debugUpdate_)
      std::cerr << "drawForeground : " << id().toStdString() << "\n";

    ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_FOREGROUND];

    startThreadTimer();
  }
  else {
    invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

    invalidateOverlay();
  }
}

void
CQChartsPlot::
drawObjs()
{
  if (isQueueUpdate()) {
    if (! isUpdatesEnabled())
      return;

    if (isOverlay() && ! isFirstPlot())
      return firstPlot()->drawObjs();

    if (debugUpdate_)
      std::cerr << "drawObjs : " << id().toStdString() << "\n";

    ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS];

    startThreadTimer();
  }
  else
    invalidateLayers();
}

//---

void
CQChartsPlot::
writeScript(CQChartsScriptPainter *device) const
{
  std::string plotId = "plot_" + this->id().toStdString();

  //---

  device->setContext("charts");

  std::ostream &os = device->os();

  //---

  os << "function Charts_" << plotId << "() {\n";
  os << "  this.visible = " << (isVisible() ? 1 : 0) << ";\n";
  os << "  this.objs = [];\n";
  os << "}\n";

  //---

  os << "\n";
  os << "Charts_" << plotId << ".prototype.init = function() {\n";

  int imajor = 0;

  for (const auto &plotObj : plotObjects()) {
    if (! plotObj->isVisible()) continue;

    if (plotObj->detailHint() == CQChartsPlotObj::DetailHint::MAJOR) {
      QString     objId  = QString("obj_") + plotId.c_str() + "_" + plotObj->id();
      std::string objStr = device->encodeObjId(objId).toStdString();

      if (imajor > 0)
        os << "\n";

      os << "  this." << objStr << " = new Charts_" << objStr << "(this);\n";
      os << "  this.objs.push(this." << objStr << ");\n";
      os << "  this." << objStr << ".init();\n";

      ++imajor;
    }
  }

  os << "  this.objs.reverse();\n"; // reverse order for tooltip

  os << "}\n";

  //---

  os << "\n";
  os << "Charts_" << plotId << ".prototype.eventMouseDown = function(e) {\n";
  os << "  if (! this.visible) return;\n";
  os << "  var rect = charts.canvas.getBoundingClientRect();\n";
  os << "  var mouseX = e.clientX - rect.left;\n";
  os << "  var mouseY = e.clientY - rect.top;\n";
  os << "  this.objs.forEach(obj => obj.eventMouseDown(mouseX, mouseY));\n";
  os << "}\n";
  os << "\n";
  os << "Charts_" << plotId << ".prototype.eventMouseMove = function(e) {\n";
  os << "  if (! this.visible) return;\n";
  os << "  var rect = charts.canvas.getBoundingClientRect();\n";
  os << "  var mouseX = e.clientX - rect.left;\n";
  os << "  var mouseY = e.clientY - rect.top;\n";
  os << "  this.objs.forEach(obj => obj.eventMouseMove(mouseX, mouseY));\n";
  os << "}\n";
  os << "\n";
  os << "Charts_" << plotId << ".prototype.eventMouseUp = function(e) {\n";
  os << "  if (! this.visible) return;\n";
  os << "  var rect = charts.canvas.getBoundingClientRect();\n";
  os << "  var mouseX = e.clientX - rect.left;\n";
  os << "  var mouseY = e.clientY - rect.top;\n";
  os << "  this.objs.forEach(obj => obj.eventMouseUp(mouseX, mouseY));\n";
  os << "}\n";

  //---

  device->resetData();

  auto vrect = viewBBox();

  os << "\n";
  os << "Charts_" << plotId << ".prototype.initRange = function() {\n";
  os << "  charts.invertX = " << isInvertX() << ";\n";
  os << "  charts.invertY = " << isInvertY() << ";\n";
  os << "\n";
  os << "  charts.vxmin = " << vrect.getXMin() << ";\n";
  os << "  charts.vymin = " << vrect.getYMin() << ";\n";
  os << "  charts.vxmax = " << vrect.getXMax() << ";\n";
  os << "  charts.vymax = " << vrect.getYMax() << ";\n";

  writeScriptRange(device);

  os << "}\n";

  //---

  // draw
  os << "\n";
  os << "Charts_" << plotId << ".prototype.draw = function() {\n";
  os << "  if (! this.visible) return;\n";
  os << "  this.initRange();\n";

  //---

  // background parts (background, bg axes, bg key)
  if (hasBackgroundLayer()) {
    os << "\n"; drawBackgroundLayer(device);
  }

  //---

  bool bgAxes = hasGroupedBgAxes();
  bool fgAxes = hasGroupedFgAxes();
  bool bgKey  = hasGroupedBgKey();
  bool fgKey  = hasGroupedFgKey();

  //---

  if (bgAxes) { os << "\n"; os << "  this.drawBgAxis();\n"; }
  if (bgKey ) { os << "\n"; os << "  this.drawBgKey ();\n"; }

  //---

  // middle parts (objects and annotations)
  os << "\n"; os << "  this.drawObjs();\n";

  if (hasGroupedAnnotations(CQChartsLayer::Type::ANNOTATION)) {
    os << "\n"; os << "  this.drawAnnotations();\n";
  }

  //---

  // foreground parts (fg axis, fg key, title, annotations, foreground)
  if (fgAxes) { os << "\n"; os << "  this.drawFgAxis();\n"; }
  if (fgKey ) { os << "\n"; os << "  this.drawFgKey ();\n"; }

  //---

  bool title = hasTitle();

  if (title) {
    os << "\n"; os << "  this.drawTitle();\n";
  }

  //---

  if (this->hasForeground()) {
    os << "\n"; execDrawForeground(device);
  }

  //---

  os << "}\n";

  //---

  // plot object procs
  for (const auto &plotObj : plotObjects()) {
    if (! plotObj->isVisible()) continue;

    if (plotObj->detailHint() == CQChartsPlotObj::DetailHint::MAJOR) {
      QString     objId  = QString("obj_") + plotId.c_str() + "_" + plotObj->id();
      std::string objStr = device->encodeObjId(objId).toStdString();

      os << "\n";
      os << "function Charts_" << objStr << "(plot) {\n";
      os << "  this.plot = plot;\n";
      os << "}\n";

      os << "\n";
      os << "Charts_" << objStr << ".prototype.init = function() {\n";
      plotObj->writeScriptData(device);
      os << "}\n";

      //---

      os << "\n";
      os << "Charts_" << objStr << ".prototype.eventMouseDown = function(mouseX, mouseY) {\n";
      os << "  this.plot.initRange();\n";
      os << "  if (this.inside(mouseX, mouseY)) {\n";

      if (view()->scriptSelectProc().length())
        os << "    " << view()->scriptSelectProc().toStdString() << "(this.id);\n";
      else
        os << "    charts.log(this.tipId);\n";

      os << "  }\n";
      os << "}\n";

      os << "\n";
      os << "Charts_" << objStr << ".prototype.eventMouseMove = function(mouseX, mouseY) {\n";
      os << "  this.plot.initRange();\n";
      os << "  var isInside = this.inside(mouseX, mouseY);\n";
      os << "  if (isInside) {\n";
      os << "    if (! charts.mouseTipObj) {\n";
      os << "      charts.mouseTipObj = this;\n";
      os << "      showTooltip(mouseX, mouseY, this.tipId);\n";
      os << "    }\n";
      os << "  }\n";
      os << "  if (isInside != this.isInside) {\n";
      os << "    this.isInside = isInside;\n";
      os << "\n";
      os << "    if (this.isInside) {\n";
      plotObj->writeScriptInsideColor(device, /*isSave*/true);
      os << "    }\n";
      os << "    else {\n";
      plotObj->writeScriptInsideColor(device, /*isSave*/false);
      os << "    }\n";
      os << "    charts.update();\n";
      os << "  }\n";
      os << "}\n";

      os << "\n";
      os << "Charts_" << objStr << ".prototype.eventMouseUp = function(mouseX, mouseY) {\n";
      os << "}\n";

      //---

      os << "\n";
      os << "Charts_" << objStr << ".prototype.inside = function(px, py) {\n";

      if      (plotObj->isPolygon()) {
        if (plotObj->isSolid())
          os << "  return charts.pointInsidePoly(px, py, this.poly);\n";
        else
          os << "  return charts.pointInsidePolyline(px, py, this.poly);\n";
      }
      else if (plotObj->isCircle()) {
        os << "  return charts.pointInsideCircle(px, py, this.xc, this.yc, this.radius);\n";
      }
      else if (plotObj->isArc()) {
        os << "  return charts.pointInsideArc(px, py, this.arc);\n";
      }
      else {
        os << "  return charts.pointInsideRect(px, py, this.xmin, this.ymin, "
              "this.xmax, this.ymax);\n";
      }

      os << "}\n";

      os << "\n";
      os << "Charts_" << objStr << ".prototype.draw = function() {\n";

      plotObj->drawBg(device);
      plotObj->draw  (device);
      plotObj->drawFg(device);

      os << "}\n";
    }
  }

  //---

  // draw objects proc
  device->resetData();

  os << "\n";
  os << "Charts_" << plotId << ".prototype.drawObjs = function() {\n";

  for (const auto &plotObj : plotObjects()) {
    if (! plotObj->isVisible()) continue;

    if (plotObj->detailHint() == CQChartsPlotObj::DetailHint::MAJOR) {
      QString     objId  = QString("obj_") + plotId.c_str() + "_" + plotObj->id();
      std::string objStr = device->encodeObjId(objId).toStdString();

      os << "  this." << objStr << ".draw();\n";
    }
    else {
      plotObj->drawBg(device);
      plotObj->draw  (device);
      plotObj->drawFg(device);
    }
  }

  drawDeviceParts(device);

  os << "}\n";

  //---

  // draw annotations proc
  if (hasGroupedAnnotations(CQChartsLayer::Type::ANNOTATION)) {
    os << "\n";
    os << "Charts_" << plotId << ".prototype.drawAnnotations = function() {\n";
    drawGroupedAnnotations(device, CQChartsLayer::Type::ANNOTATION);
    os << "}\n";
  }

  //---

  // draw axes procs
  if (bgAxes) {
    device->resetData();

    os << "\n";
    os << "Charts_" << plotId << ".prototype.drawBgAxis = function() {\n";
    drawGroupedBgAxes(device);
    os << "}\n";
  }

  if (fgAxes) {
    device->resetData();

    os << "\n";
    os << "Charts_" << plotId << ".prototype.drawFgAxis = function() {\n";
    drawGroupedFgAxes(device);
    os << "}\n";
  }

  //---

  // draw key procs
  if (bgKey) {
    device->resetData();

    os << "\n";
    os << "Charts_" << plotId << ".prototype.drawBgKey = function() {\n";
    drawBgKey(device);
    os << "}\n";
  }

  if (fgKey) {
    device->resetData();

    os << "\n";
    os << "Charts_" << plotId << ".prototype.drawFgKey = function() {\n";
    drawFgKey(device);
    os << "}\n";
  }

  //---

  // draw title proc
  if (title) {
    device->resetData();

    os << "\n";
    os << "Charts_" << plotId << ".prototype.drawTitle = function() {\n";
    drawTitle(device);
    os << "}\n";
  }

  //---

  device->setContext("");
}

void
CQChartsPlot::
writeScriptRange(CQChartsScriptPainter *device) const
{
  std::ostream &os = device->os();

  auto prect = calcPlotRect();

  os << "\n";
  os << "  charts.xmin = " << prect.getXMin() << ";\n";
  os << "  charts.ymin = " << prect.getYMin() << ";\n";
  os << "  charts.xmax = " << prect.getXMax() << ";\n";
  os << "  charts.ymax = " << prect.getYMax() << ";\n";
}

void
CQChartsPlot::
writeSVG(CQChartsSVGPainter *device) const
{
  QString plotId = "plot_" + this->id();

  CQChartsSVGPainter::GroupData groupData;

  groupData.visible = isVisible();

  device->startGroup(plotId, groupData);

  if (hasBackgroundLayer()) {
    drawBackgroundLayer(device);
  }

  if (hasForeground()) {
    execDrawBackground(device);
  }

  if (hasGroupedBgAxes()) {
    device->startGroup("bg_axis");

    drawGroupedBgAxes(device);

    device->endGroup();
  }

  if (hasGroupedBgKey()) {
    device->startGroup("bg_key");

    drawBgKey(device);

    device->endGroup();
  }

  for (const auto &plotObj : plotObjects()) {
    QString objId = QString("obj_") + plotId + "_" + plotObj->id();

    CQChartsSVGPainter::GroupData objGroupData;

    objGroupData.visible   = plotObj->isVisible();
    objGroupData.onclick   = true;
    objGroupData.clickProc = "plotObjClick";
    objGroupData.tipStr    = CQChartsSVGPainter::encodeString(plotObj->tipId());
    objGroupData.hasTip    = plotObj->hasTipId();

    device->startGroup(device->encodeObjId(objId), objGroupData);

    plotObj->drawBg(device);
    plotObj->draw  (device);
    plotObj->drawFg(device);

    device->endGroup();
  }

  if (hasGroupedAnnotations(CQChartsLayer::Type::ANNOTATION)) {
    drawGroupedAnnotations(device, CQChartsLayer::Type::ANNOTATION);
  }

  if (hasForeground()) {
    execDrawForeground(device);
  }

  if (hasGroupedFgAxes()) {
    device->startGroup("fg_axis");

    drawGroupedFgAxes(device);

    device->endGroup();
  }

  if (hasGroupedFgKey()) {
    device->startGroup("fg_key");

    drawFgKey(device);

    device->endGroup();
  }

  if (hasTitle()) {
    device->startGroup("title");

    drawTitle(device);

    device->endGroup();
  }

  device->endGroup();
}

void
CQChartsPlot::
writeHtml(CQChartsHtmlPainter *device) const
{
  if (hasGroupedAnnotations(CQChartsLayer::Type::ANNOTATION)) {
    if (isOverlay()) {
      if (! isFirstPlot())
        return;

      processOverlayPlots([&](const CQChartsPlot *plot) {
        device->setPlot(const_cast<CQChartsPlot *>(plot));

        for (auto &annotation : plot->annotations())
          annotation->writeHtml(device);
      });

      device->setPlot(const_cast<CQChartsPlot *>(this));
    }
    else {
      for (auto &annotation : annotations())
        annotation->writeHtml(device);
    }
  }
}

//---

void
CQChartsPlot::
invalidateOverlay()
{
  invalidateLayer(CQChartsBuffer::Type::OVERLAY);
}

//---

const CQChartsDisplayRange &
CQChartsPlot::
displayRange() const
{
  return *displayRange_;
}

void
CQChartsPlot::
setDisplayRange(const CQChartsDisplayRange &r)
{
  *displayRange_ = r;
}

//---

void
CQChartsPlot::
setDataRange(const CQChartsGeom::Range &r, bool update)
{
  CQChartsUtil::testAndSet(dataRange_, r, [&]() {
    if (debugUpdate_)
      std::cerr << "setDataRange: " << id().toStdString() << " : " << dataRange_ << "\n";

    if (update)
      updateObjs();
  } );
}

void
CQChartsPlot::
resetDataRange(bool updateRange, bool updateObjs)
{
  setDataRange(CQChartsGeom::Range(), /*update*/false);

  if (updateRange)
    this->execUpdateRange();

  if (updateObjs)
    this->updateObjs();
}

//---

double
CQChartsPlot::
dataScaleX() const
{
  if      (isOverlay()) {
    if (! isFirstPlot())
      return firstPlot()->dataScaleX();
  }
  else if (isY1Y2()) {
    if (! isFirstPlot())
      return firstPlot()->dataScaleX();
  }

  return zoomData_.dataScale.x;
}

void
CQChartsPlot::
setDataScaleX(double x)
{
  zoomData_.dataScale.x = x;
}

double
CQChartsPlot::
dataScaleY() const
{
  if      (isOverlay()) {
    if (! isFirstPlot())
      return firstPlot()->dataScaleY();
  }
  else if (isX1X2()) {
    if (! isFirstPlot())
      return firstPlot()->dataScaleY();
  }

  return zoomData_.dataScale.y;
}

void
CQChartsPlot::
setDataScaleY(double y)
{
  zoomData_.dataScale.y = y;
}

double
CQChartsPlot::
dataOffsetX() const
{
  if      (isOverlay()) {
    if (! isFirstPlot())
      return firstPlot()->dataOffsetX();
  }
  else if (isY1Y2()) {
    if (! isFirstPlot())
      return firstPlot()->dataOffsetX();
  }

  return zoomData_.dataOffset.x;
}

void
CQChartsPlot::
setDataOffsetX(double x)
{
  zoomData_.dataOffset.x = x;
}

double
CQChartsPlot::
dataOffsetY() const
{
  if      (isOverlay()) {
    if (! isFirstPlot())
      return firstPlot()->dataOffsetY();
  }
  else if (isX1X2()) {
    if (! isFirstPlot())
      return firstPlot()->dataOffsetY();
  }

  return zoomData_.dataOffset.y;
}

void
CQChartsPlot::
setDataOffsetY(double y)
{
  zoomData_.dataOffset.y = y;
}

//---

void
CQChartsPlot::
setZoomData(const ZoomData &zoomData)
{
  zoomData_ = zoomData;

  applyDataRangeAndDraw();
}

void
CQChartsPlot::
updateDataScaleX(double r)
{
  setDataScaleX(r);

  applyDataRangeAndDraw();
}

void
CQChartsPlot::
updateDataScaleY(double r)
{
  setDataScaleY(r);

  applyDataRangeAndDraw();
}

//---

void
CQChartsPlot::
setXMin(const CQChartsOptReal &r)
{
  CQChartsUtil::testAndSet(xmin_, r, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsPlot::
setXMax(const CQChartsOptReal &r)
{
  CQChartsUtil::testAndSet(xmax_, r, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsPlot::
setYMin(const CQChartsOptReal &r)
{
  CQChartsUtil::testAndSet(ymin_, r, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsPlot::
setYMax(const CQChartsOptReal &r)
{
  CQChartsUtil::testAndSet(ymax_, r, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsPlot::
setEveryEnabled(bool b)
{
  CQChartsUtil::testAndSet(everyData_.enabled, b, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setEveryStart(int i)
{
  CQChartsUtil::testAndSet(everyData_.start, i, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setEveryEnd(int i)
{
  CQChartsUtil::testAndSet(everyData_.end, i, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setEveryStep(int i)
{
  CQChartsUtil::testAndSet(everyData_.step, i, [&]() { updateObjs(); } );
}

//---

void
CQChartsPlot::
setFilterStr(const QString &s)
{
  CQChartsUtil::testAndSet(filterStr_, s, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsPlot::
setTitleStr(const QString &s)
{
  if (title()) {
    // title calls drawForeground
    CQChartsUtil::testAndSet(titleStr_, s, [&]() { title()->setTextStr(titleStr_); } );
  }
}

//---

QString
CQChartsPlot::
xLabel() const
{
  return (mappedXAxis() ? mappedXAxis()->userLabel() : "");
}

void
CQChartsPlot::
setXLabel(const QString &s)
{
  if (mappedXAxis())
    mappedXAxis()->setUserLabel(s);
}

QString
CQChartsPlot::
yLabel() const
{
  return (mappedYAxis() ? mappedYAxis()->userLabel() : "");
}

void
CQChartsPlot::
setYLabel(const QString &s)
{
  if (mappedYAxis())
    mappedYAxis()->setUserLabel(s);
}

//---

void
CQChartsPlot::
setPlotBorderSides(const CQChartsSides &s)
{
  CQChartsUtil::testAndSet(plotBorderSides_, s, [&]() { drawBackground(); } );
}

void
CQChartsPlot::
setPlotClip(bool b)
{
  CQChartsUtil::testAndSet(plotClip_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsPlot::
setDataBorderSides(const CQChartsSides &s)
{
  CQChartsUtil::testAndSet(dataBorderSides_, s, [&]() { drawBackground(); } );
}

void
CQChartsPlot::
setDataClip(bool b)
{
  CQChartsUtil::testAndSet(dataClip_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsPlot::
setFitBorderSides(const CQChartsSides &s)
{
  CQChartsUtil::testAndSet(fitBorderSides_, s, [&]() { drawBackground(); } );
}

//---

void
CQChartsPlot::
setFont(const CQChartsFont &f)
{
  CQChartsUtil::testAndSet(font_, f, [&]() { drawObjs(); } );
}

//---

void
CQChartsPlot::
setDefaultPalette(const CQChartsPaletteName &name)
{
  CQChartsUtil::testAndSet(defaultPalette_, name, [&]() { drawObjs(); } );
}

//---

bool
CQChartsPlot::
isKeyVisible() const
{
  return (key() && key()->isVisible());
}

void
CQChartsPlot::
setKeyVisible(bool b)
{
  if (key())
    key()->setVisible(b);
}

bool
CQChartsPlot::
isKeyVisibleAndNonEmpty() const
{
  return (key() && key()->isVisibleAndNonEmpty());
}

void
CQChartsPlot::
setColorKey(bool b)
{
  CQChartsUtil::testAndSet(colorKey_, b, [&]() {
    resetSetHidden();

    resetKeyItems();

    updateRangeAndObjs();

    updateKeyPosition(/*force*/true);
  });
}

//---

void
CQChartsPlot::
setEqualScale(bool b)
{
  CQChartsUtil::testAndSet(equalScale_, b, [&]() { updateRange(); });
}

void
CQChartsPlot::
setShowBoxes(bool b)
{
  CQChartsUtil::testAndSet(showBoxes_, b, [&]() { invalidateOverlay(); } );
}

void
CQChartsPlot::
setViewBBox(const CQChartsGeom::BBox &bbox)
{
  viewBBox_      = bbox;
  innerViewBBox_ = viewBBox_;

  updateMargins();
}

void
CQChartsPlot::
updateMargins(bool update)
{
  if (isOverlay()) {
    if (! isFirstPlot())
      return firstPlot()->updateMargins(update);

    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->updateMargins(outerMargin());
    });
  }
  else {
    updateMargins(outerMargin());
  }

  updateKeyPosition(/*force*/true);

  if (update)
    drawObjs();
}

void
CQChartsPlot::
updateMargins(const CQChartsPlotMargin &outerMargin)
{
  innerViewBBox_ = outerMargin.adjustViewRange(this, viewBBox(), /*inside*/false);

  setPixelRange(innerViewBBox_);
}

//---

CQChartsGeom::BBox
CQChartsPlot::
calcDataRect() const
{
  return calcDataRange_.bbox();
}

CQChartsGeom::BBox
CQChartsPlot::
outerDataRect() const
{
  return outerDataRange_.bbox();
}

CQChartsGeom::BBox
CQChartsPlot::
dataRect() const
{
  return dataRange_.bbox();
}

//---

CQChartsGeom::BBox
CQChartsPlot::
viewBBox() const
{
  if (isOverlay() && ! isFirstPlot())
    return firstPlot()->viewBBox();

  return viewBBox_;
}

CQChartsGeom::BBox
CQChartsPlot::
innerViewBBox() const
{
  if (isOverlay() && ! isFirstPlot())
    return firstPlot()->innerViewBBox();

  return innerViewBBox_;
}

CQChartsGeom::BBox
CQChartsPlot::
range() const
{
  return dataRect();
}

void
CQChartsPlot::
setRange(const CQChartsGeom::BBox &bbox)
{
  assert(dataScaleX() == 1.0 && dataScaleY() == 1.0);

  auto range = CQChartsUtil::bboxRange(bbox);

  setDataRange(range);

  applyDataRange();
}

double
CQChartsPlot::
aspect() const
{
  CQChartsGeom::Point p1 =
    view()->windowToPixel(CQChartsGeom::Point(viewBBox().getXMin(), viewBBox().getYMin()));
  CQChartsGeom::Point p2 =
    view()->windowToPixel(CQChartsGeom::Point(viewBBox().getXMax(), viewBBox().getYMax()));

  if (p1.y == p2.y)
    return 1.0;

  return fabs(p2.x - p1.x)/fabs(p2.y - p1.y);
}

//---

// inner margin
void
CQChartsPlot::
setInnerMarginLeft(const CQChartsLength &l)
{
  if (l != innerMargin_.left()) {
    innerMargin_.setLeft(l);

    applyDataRangeAndDraw();
  }
}

void
CQChartsPlot::
setInnerMarginTop(const CQChartsLength &t)
{
  if (t != innerMargin_.top()) {
    innerMargin_.setTop(t);

    applyDataRangeAndDraw();
  }
}

void
CQChartsPlot::
setInnerMarginRight(const CQChartsLength &r)
{
  if (r != innerMargin_.right()) {
    innerMargin_.setRight(r);

    applyDataRangeAndDraw();
  }
}

void
CQChartsPlot::
setInnerMarginBottom(const CQChartsLength &b)
{
  if (b != innerMargin_.bottom()) {
    innerMargin_.setBottom(b);

    applyDataRangeAndDraw();
  }
}

void
CQChartsPlot::
setInnerMargin(const CQChartsLength &l, const CQChartsLength &t,
               const CQChartsLength &r, const CQChartsLength &b)
{
  setInnerMargin(CQChartsPlotMargin(l, t, r, b));
}

void
CQChartsPlot::
setInnerMargin(const CQChartsPlotMargin &m)
{
  innerMargin_ = m;

  applyDataRangeAndDraw();
}

//---

void
CQChartsPlot::
setOuterMarginLeft(const CQChartsLength &l)
{
  if (l != outerMargin_.left()) {
    outerMargin_.setLeft(l);

    updateMargins();
  }
}

void
CQChartsPlot::
setOuterMarginTop(const CQChartsLength &t)
{
  if (t != outerMargin_.top()) {
    outerMargin_.setTop(t);

    updateMargins();
  }
}

void
CQChartsPlot::
setOuterMarginRight(const CQChartsLength &r)
{
  if (r != outerMargin_.right()) {
    outerMargin_.setRight(r);

    updateMargins();
  }
}

void
CQChartsPlot::
setOuterMarginBottom(const CQChartsLength &b)
{
  if (b != outerMargin_.bottom()) {
    outerMargin_.setBottom(b);

    updateMargins();
  }
}

void
CQChartsPlot::
setOuterMargin(const CQChartsLength &l, const CQChartsLength &t,
               const CQChartsLength &r, const CQChartsLength &b)
{
  return setOuterMargin(CQChartsPlotMargin(l, t, r, b));
}

void
CQChartsPlot::
setOuterMargin(const CQChartsPlotMargin &m)
{
  outerMargin_ = m;

  updateMargins();
}

//---

bool
CQChartsPlot::
isOverlay(bool checkVisible) const
{
  if (! connectData_.overlay)
    return false;

  if (checkVisible) {
    Plots plots;

    overlayPlots(plots);

    return plots.size() > 1;
  }

  return true;
}

void
CQChartsPlot::
setOverlay(bool b, bool notify)
{
  connectData_.overlay = b;

  if (notify)
    emit connectDataChanged();
}

void
CQChartsPlot::
updateOverlay()
{
  processOverlayPlots([&](CQChartsPlot *plot) {
    plot->stopThreadTimer ();
    plot->startThreadTimer();

    plot->updatesData_.reset();
  });

  applyDataRangeAndDraw();
}

bool
CQChartsPlot::
isX1X2(bool checkVisible) const
{
  if (! connectData_.x1x2)
    return false;

  if (checkVisible) {
    Plots plots;

    overlayPlots(plots);

    return plots.size() > 1;
  }

  return true;
}

void
CQChartsPlot::
setX1X2(bool b, bool notify)
{
  connectData_.x1x2 = b;

  if (notify)
    emit connectDataChanged();
}

bool
CQChartsPlot::
isY1Y2(bool checkVisible) const
{
  if (! connectData_.y1y2)
    return false;

  if (checkVisible) {
    Plots plots;

    overlayPlots(plots);

    return plots.size() > 1;
  }

  return true;
}

void
CQChartsPlot::
setY1Y2(bool b, bool notify)
{
  connectData_.y1y2 = b;

  if (notify)
    emit connectDataChanged();
}

void
CQChartsPlot::
setNextPlot(CQChartsPlot *plot, bool notify)
{
  assert(plot != this && ! connectData_.next);

  connectData_.next = plot;

  if (notify)
    emit connectDataChanged();
}

void
CQChartsPlot::
setPrevPlot(CQChartsPlot *plot, bool notify)
{
  assert(plot != this && ! connectData_.prev);

  connectData_.prev = plot;

  if (notify)
    emit connectDataChanged();
}

CQChartsPlot *
CQChartsPlot::
firstPlot()
{
  if (connectData_.prev)
    return connectData_.prev->firstPlot();

  CQChartsPlot *plot = this;

  while (plot && ! plot->isVisible())
    plot = plot->nextPlot();

  if (! plot)
    plot = this;

  return plot;
}

const CQChartsPlot *
CQChartsPlot::
firstPlot() const
{
  return const_cast<CQChartsPlot *>(this)->firstPlot();
}

CQChartsPlot *
CQChartsPlot::
lastPlot()
{
  if (connectData_.next)
    return connectData_.next->lastPlot();

  CQChartsPlot *plot = this;

  while (plot && ! plot->isVisible())
    plot = plot->prevPlot();

  if (! plot)
    plot = this;

  return plot;
}

const CQChartsPlot *
CQChartsPlot::
lastPlot() const
{
  return const_cast<CQChartsPlot *>(this)->lastPlot();
}

void
CQChartsPlot::
overlayPlots(Plots &plots, bool visibleOnly) const
{
  const CQChartsPlot *plot1 = firstPlot();

  while (plot1) {
    if (! visibleOnly || plot1->isVisible())
      plots.push_back(const_cast<CQChartsPlot *>(plot1));

    plot1 = plot1->nextPlot();
  }
}

void
CQChartsPlot::
x1x2Plots(CQChartsPlot* &plot1, CQChartsPlot* &plot2)
{
  plot1 = firstPlot();
  plot2 = (plot1 ? plot1->nextPlot() : nullptr);
}

void
CQChartsPlot::
y1y2Plots(CQChartsPlot* &plot1, CQChartsPlot* &plot2)
{
  plot1 = firstPlot();
  plot2 = (plot1 ? plot1->nextPlot() : nullptr);
}

void
CQChartsPlot::
resetConnectData(bool notify)
{
  connectData_.reset();

  if (notify)
    emit connectDataChanged();
}

//------

void
CQChartsPlot::
setInvertX(bool b)
{
  if (isOverlay()) {
    // if first plot then set all plots to same invert value
    if (prevPlot())
      return;

    //---

    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->invertX_ = b;
    });
  }
  else {
    invertX_ = b;
  }

  drawObjs();
}

void
CQChartsPlot::
setInvertY(bool b)
{
  if (isOverlay()) {
    // if first plot then set all plots to same invert value
    if (prevPlot())
      return;

    //---

    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->invertY_ = b;
    });
  }
  else {
    invertY_ = b;
  }

  drawObjs();
}

//------

bool
CQChartsPlot::
isLogX() const
{
  // return logX_;
  return (mappedXAxis() && mappedXAxis()->valueType() == CQChartsAxisValueType::Type::LOG);
}

bool
CQChartsPlot::
isLogY() const
{
  // return logY_;
  return (mappedYAxis() && mappedYAxis()->valueType() == CQChartsAxisValueType::Type::LOG);
}

void
CQChartsPlot::
setLogX(bool b)
{
  if (mappedXAxis() && b != isLogX()) {
    mappedXAxis()->setValueType(b ? CQChartsAxisValueType::Type::LOG :
                                    CQChartsAxisValueType::Type::REAL);
    updateRangeAndObjs();
  }
}

void
CQChartsPlot::
setLogY(bool b)
{
  if (mappedYAxis() && b != isLogY()) {
    mappedYAxis()->setValueType(b ? CQChartsAxisValueType::Type::LOG :
                                    CQChartsAxisValueType::Type::REAL);
    updateRangeAndObjs();
  }
}

//------

const CQPropertyViewModel *
CQChartsPlot::
propertyModel() const
{
  return propertyModel_;
}

CQPropertyViewModel *
CQChartsPlot::
propertyModel()
{
  return propertyModel_;
}

void
CQChartsPlot::
addProperties()
{
  addBaseProperties();
}

void
CQChartsPlot::
addBaseProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc, bool hidden=false) {
    CQPropertyViewItem *item = &(this->addProperty(path, this, name, alias)->setDesc(desc));
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  auto addStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc, bool hidden=false) {
    CQPropertyViewItem *item = addProp(path, name, alias, desc, hidden);
    CQCharts::setItemIsStyle(item);
    return item;
  };

  // data
  addProp("", "viewId"  , "view"    , "Parent view id", true);
  addProp("", "typeStr" , "type"    , "Type name"     , true);
  addProp("", "visible" , "visible" , "Plot visible"  , true);
  addProp("", "selected", "selected", "Plot selected" , true);

  // font
  addStyleProp("font", "font", "font", "Base font");

  // columns
  addProp("columns", "idColumn"     , "id"     , "Id column");
  addProp("columns", "tipColumns"   , "tips"   , "Tips columns");
  addProp("columns", "visibleColumn", "visible", "Visible column");
  addProp("columns", "colorColumn"  , "color"  , "Color column");
  addProp("columns", "imageColumn"  , "image"  , "Image column");

  // range
  addProp("range", "viewRect", "view", "View rectangle");
  addProp("range", "dataRect", "data", "Data rectangle");

  addProp("range", "innerViewRect", "innerView", "Inner view rectangle"     , true);
  addProp("range", "calcDataRect" , "calcData" , "Calculated data rectangle", true);
  addProp("range", "outerDataRect", "outerData", "Outer data rectangle"     , true);

  addProp("range", "autoFit", "autoFit", "Auto fit to data");

  if (type()->customXRange()) addProp("range", "xmin", "xmin", "Explicit minimum x value");
  if (type()->customYRange()) addProp("range", "ymin", "ymin", "Explicit minimum y value");
  if (type()->customXRange()) addProp("range", "xmax", "xmax", "Explicit maximum x value");
  if (type()->customYRange()) addProp("range", "ymax", "ymax", "Explicit maximum y value");

  // scaling
  addProp("scaling", "equalScale", "equal", "Equal x/y scaling");

  addProp("scaling/data/scale" , "dataScaleX" , "x", "X data scale" , true);
  addProp("scaling/data/scale" , "dataScaleY" , "y", "Y data scale" , true);
  addProp("scaling/data/offset", "dataOffsetX", "x", "X data offset", true);
  addProp("scaling/data/offset", "dataOffsetY", "y", "Y data offset", true);

  // grouping
  addProp("grouping", "overlay", "", "Overlay plots to shared range"    , true);
  addProp("grouping", "x1x2"   , "", "Independent x axes, shared y axis", true);
  addProp("grouping", "y1y2"   , "", "Independent y axes, shared x axis", true);

  // invert
  addProp("invert", "invertX", "x", "Invert x values");
  addProp("invert", "invertY", "y", "Invert y values");

#if 0
  // log
  if (type()->allowXLog()) addProp("log", "logX", "x", "Use log x axis");
  if (type()->allowYLog()) addProp("log", "logY", "y", "Use log y axis");
#endif

  // debug
  if (CQChartsEnv::getBool("CQ_CHARTS_DEBUG")) {
    addProp("debug", "showBoxes"  , "", "Show object bounding boxes");
    addProp("debug", "followMouse", "", "Enable mouse tracking");
  }

  //------

  // plot box
  QString plotStyleStr       = "plotBox";
  QString plotStyleFillStr   = plotStyleStr + "/fill";
  QString plotStyleStrokeStr = plotStyleStr + "/stroke";

  addProp(plotStyleStr, "plotClip", "clip" , "Clip to plot bounding box");

  addProp(plotStyleStr, "plotShapeData", "shape", "Plot background shape data", true);

  addStyleProp(plotStyleFillStr, "plotFilled", "visible",
               "Plot background bounding box fill visible");

  addFillProperties(plotStyleFillStr, "plotFill", "Plot background");

  addStyleProp(plotStyleStrokeStr, "plotStroked", "visible",
               "Plot background bounding box stroke visible");

  addLineProperties(plotStyleStrokeStr, "plotStroke", "Plot background");

  addStyleProp(plotStyleStrokeStr, "plotBorderSides", "sides",
               "Plot background bounding box stroked sides");

  //---

  // data box
  QString dataStyleStr       = "dataBox";
  QString dataStyleFillStr   = dataStyleStr + "/fill";
  QString dataStyleStrokeStr = dataStyleStr + "/stroke";

  addProp(dataStyleStr, "dataClip", "clip" , "Clip to data bounding box");

  addProp(dataStyleStr, "dataShapeData", "shape", "Data background shape data", true);

  addStyleProp(dataStyleFillStr, "dataFilled", "visible",
               "Data background bounding box fill visible");

  addFillProperties(dataStyleFillStr, "dataFill", "Data background");

  addStyleProp(dataStyleStrokeStr, "dataStroked", "visible",
               "Data background bounding box stroke visible");

  addLineProperties(dataStyleStrokeStr, "dataStroke", "Data background");

  addStyleProp(dataStyleStrokeStr, "dataBorderSides", "sides",
               "Data background bounding box stroked sides");

  //---

  // fit box
  QString fitStyleStr       = "fitBox";
  QString fitStyleFillStr   = fitStyleStr + "/fill";
  QString fitStyleStrokeStr = fitStyleStr + "/stroke";

  addStyleProp(fitStyleFillStr, "fitFilled", "visible",
               "Fit background bounding box fill visible", true);

  addFillProperties(fitStyleFillStr, "fitFill", "Fit background", /*hidden*/true);

  addStyleProp(fitStyleStrokeStr, "fitStroked", "visible",
               "Fit background bounding box stroke visible", true);

  addLineProperties(fitStyleStrokeStr, "fitStroke", "Fit background", /*hidden*/true);

  addStyleProp(fitStyleStrokeStr, "fitBorderSides", "sides",
               "Fit background bounding box stroked sides", true);

  //---

  // margin
  addProp("margins/inner", "innerMarginLeft"  , "left"  , "Size of inner margin at left of plot");
  addProp("margins/inner", "innerMarginTop"   , "top"   , "Size of inner margin at top of plot");
  addProp("margins/inner", "innerMarginRight" , "right" , "Size of inner margin at right of plot");
  addProp("margins/inner", "innerMarginBottom", "bottom", "Size of inner margin at bottom of plot");

  addProp("margins/outer", "outerMarginLeft"  , "left"  , "Size of outer margin at left of plot");
  addProp("margins/outer", "outerMarginTop"   , "top"   , "Size of outer margin at top of plot");
  addProp("margins/outer", "outerMarginRight" , "right" , "Size of outer margin at right of plot");
  addProp("margins/outer", "outerMarginBottom", "bottom", "Size of outer margin at bottom of plot");

  //---

  // every
  addProp("every", "everyEnabled", "enabled", "Enable every row filter"  , true);
  addProp("every", "everyStart"  , "start"  , "Start of every row filter", true);
  addProp("every", "everyEnd"    , "end"    , "End of every row filter"  , true);
  addProp("every", "everyStep"   , "step"   , "Step of every row filter" , true);

  // filter
  addProp("filter", "filterStr", "expression", "Filter expression", true);

  //---

  // xaxis
  if (xAxis()) {
    xAxis()->addProperties(propertyModel(), "xaxis");

    addProperty("xaxis", xAxis(), "userLabel", "userLabel")->
      setDesc("User defined x axis label");
  }

  // yaxis
  if (yAxis()) {
    yAxis()->addProperties(propertyModel(), "yaxis");

    addProperty("yaxis", yAxis(), "userLabel", "userLabel")->
      setDesc("User defined y axis label");
  }

  // key
  if (key()) {
    key()->addProperties(propertyModel(), "key");

    addProp("key", "colorKey", "colorColumn", "Use Color Column for Key");
  }

  // title
  if (title())
    title()->addProperties(propertyModel(), "title");

  // coloring
  addProp("coloring", "defaultPalette", "defaultPalette", "Default palette");
  addProp("coloring", "colorType"     , "type"          , "Color interpolation type");
  addProp("coloring", "colorXStops"   , "xStops"        , "Color x stop coordinates");
  addProp("coloring", "colorYStops"   , "yStops"        , "Color y stop coordinates");

  // scaled font
  addProp("scaledFont", "minScaleFontSize", "minSize", "Min scaled font size", true);
  addProp("scaledFont", "maxScaleFontSize", "maxSize", "Max scaled font size", true);
}

void
CQChartsPlot::
addSymbolProperties(const QString &path, const QString &prefix, const QString &descPrefix)
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  QString prefix1 = (descPrefix.length() ? descPrefix + " symbol" : "Symbol");

  QString strokePath = path + "/stroke";
  QString fillPath   = path + "/fill";

  QString symbolPrefix = (prefix.length() ? prefix + "Symbol" : "symbol");

  addProp(path, symbolPrefix + "Type", "type", prefix1 + " type");
  addProp(path, symbolPrefix + "Size", "size", prefix1 + " size");

  //---

  auto addStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc, bool hidden=false) {
    CQPropertyViewItem *item = addProp(path, name, alias, desc);
    CQCharts::setItemIsStyle(item);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  addStyleProp(fillPath, symbolPrefix + "Filled"     , "visible", prefix1 + " fill visible");
  addStyleProp(fillPath, symbolPrefix + "FillColor"  , "color"  , prefix1 + " fill color");
  addStyleProp(fillPath, symbolPrefix + "FillAlpha"  , "alpha"  , prefix1 + " fill alpha");
  addStyleProp(fillPath, symbolPrefix + "FillPattern", "pattern", prefix1 + " fill pattern", true);

  addStyleProp(strokePath, symbolPrefix + "Stroked"    , "visible", prefix1 + " stroke visible");
  addStyleProp(strokePath, symbolPrefix + "StrokeColor", "color"  , prefix1 + " stroke color");
  addStyleProp(strokePath, symbolPrefix + "StrokeAlpha", "alpha"  , prefix1 + " stroke alpha");
  addStyleProp(strokePath, symbolPrefix + "StrokeWidth", "width"  , prefix1 + " stroke width");
  addStyleProp(strokePath, symbolPrefix + "StrokeDash" , "dash"   , prefix1 + " stroke dash");
}

void
CQChartsPlot::
addLineProperties(const QString &path, const QString &prefix,
                  const QString &descPrefix, bool hidden)
{
  auto addStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc, bool hidden) {
    CQPropertyViewItem *item = this->addProperty(path, this, name, alias);
    item->setDesc(desc);
    CQCharts::setItemIsStyle(item);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  //---

  QString prefix1 = (descPrefix.length() ? descPrefix + " stroke" : "Stroke");

  addStyleProp(path, prefix + "Color", "color", prefix1 + " color", hidden);
  addStyleProp(path, prefix + "Alpha", "alpha", prefix1 + " alpha", hidden);
  addStyleProp(path, prefix + "Width", "width", prefix1 + " width", hidden);
  addStyleProp(path, prefix + "Dash" , "dash" , prefix1 + " dash" , hidden);
}

void
CQChartsPlot::
addFillProperties(const QString &path, const QString &prefix,
                  const QString &descPrefix, bool hidden)
{
  auto addStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc, bool hidden) {
    CQPropertyViewItem *item = this->addProperty(path, this, name, alias);
    item->setDesc(desc);
    CQCharts::setItemIsStyle(item);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  //---

  QString prefix1 = (descPrefix.length() ? descPrefix + " fill" : "Fill");

  addStyleProp(path, prefix + "Color"  , "color"  , prefix1 + " color"  , hidden);
  addStyleProp(path, prefix + "Alpha"  , "alpha"  , prefix1 + " alpha"  , hidden);
  addStyleProp(path, prefix + "Pattern", "pattern", prefix1 + " pattern", true  );
}

void
CQChartsPlot::
addTextProperties(const QString &path, const QString &prefix, const QString &descPrefix,
                  uint valueTypes)
{
  auto addStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc) {
    CQPropertyViewItem *item = this->addProperty(path, this, name, alias);
    item->setDesc(desc);
    CQCharts::setItemIsStyle(item);
    return item;
  };

  //---

  QString prefix1 = (descPrefix.length() ? descPrefix + " text" : "Text");

  // style
  addStyleProp(path, prefix + "Color", "color", prefix1 + " color");
  addStyleProp(path, prefix + "Alpha", "alpha", prefix1 + " alpha");
  addStyleProp(path, prefix + "Font" , "font" , prefix1 + " font");

  // options
  if (valueTypes & CQChartsTextOptions::ValueType::ANGLE)
    addStyleProp(path, prefix + "Angle", "angle", prefix1 + " angle");

  if (valueTypes & CQChartsTextOptions::ValueType::ALIGN)
    addStyleProp(path, prefix + "Align", "align", prefix1 + " align");

  if (valueTypes & CQChartsTextOptions::ValueType::CONTRAST) {
    addStyleProp(path, prefix + "Contrast"     , "contrast"     , prefix1 + " contrast");
    addStyleProp(path, prefix + "ContrastAlpha", "contrastAlpha", prefix1 + " contrast alpha");
  }

  if (valueTypes & CQChartsTextOptions::ValueType::HTML)
    addStyleProp(path, prefix + "Html", "html", prefix1 + " is HTML");

  if (valueTypes & CQChartsTextOptions::ValueType::FORMATTED)
    addStyleProp(path, prefix + "Formatted", "formatted", prefix1 + " formatted to fit box");

  if (valueTypes & CQChartsTextOptions::ValueType::SCALED)
    addStyleProp(path, prefix + "Scaled", "scaled", prefix1 + " scaled to box");
}

void
CQChartsPlot::
addColorMapProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  addProp("mapping/color", "colorMapped"    , "enabled", "Color values mapped");
  addProp("mapping/color", "colorMapMin"    , "min"    , "Color value map min");
  addProp("mapping/color", "colorMapMax"    , "max"    , "Color value map max");
  addProp("mapping/color", "colorMapPalette", "palette", "Color map palette");
}

//---

bool
CQChartsPlot::
setProperties(const QString &properties)
{
  bool rc = true;

  CQChartsNameValues nameValues(properties);

  for (const auto &nv : nameValues.nameValues()) {
    const QString  &name  = nv.first;
    const QVariant &value = nv.second;

    if (! setProperty(name, value))
      rc = false;
  }

  return rc;
}

bool
CQChartsPlot::
setProperty(const QString &name, const QVariant &value)
{
  return propertyModel()->setProperty(this, name, value);
}

bool
CQChartsPlot::
getProperty(const QString &name, QVariant &value) const
{
  return propertyModel()->getProperty(this, name, value);
}

bool
CQChartsPlot::
getTclProperty(const QString &name, QVariant &value) const
{
  return propertyModel()->getTclProperty(this, name, value);
}

bool
CQChartsPlot::
getPropertyDesc(const QString &name, QString &desc, bool hidden) const
{
  const CQPropertyViewItem *item = propertyModel()->propertyItem(this, name, hidden);
  if (! item) return false;

  desc = item->desc();

  return true;
}

bool
CQChartsPlot::
getPropertyType(const QString &name, QString &type, bool hidden) const
{
  const CQPropertyViewItem *item = propertyModel()->propertyItem(this, name, hidden);
  if (! item) return false;

  type = item->typeName();

  return true;
}

bool
CQChartsPlot::
getPropertyUserType(const QString &name, QString &type, bool hidden) const
{
  const CQPropertyViewItem *item = propertyModel()->propertyItem(this, name, hidden);
  if (! item) return false;

  type = item->userTypeName();

  return true;
}

bool
CQChartsPlot::
getPropertyObject(const QString &name, QObject* &object, bool hidden) const
{
  object = nullptr;

  const CQPropertyViewItem *item = propertyModel()->propertyItem(this, name, hidden);
  if (! item) return false;

  object = item->object();

  return true;
}

bool
CQChartsPlot::
getPropertyIsHidden(const QString &name, bool &is_hidden) const
{
  is_hidden = false;

  const CQPropertyViewItem *item = propertyModel()->propertyItem(this, name, /*hidden*/true);
  if (! item) return false;

  is_hidden = CQCharts::getItemIsHidden(item);

  return true;
}

bool
CQChartsPlot::
getPropertyIsStyle(const QString &name, bool &is_style) const
{
  is_style = false;

  const CQPropertyViewItem *item = propertyModel()->propertyItem(this, name, /*hidden*/true);
  if (! item) return false;

  is_style = CQCharts::getItemIsStyle(item);

  return true;
}

CQPropertyViewItem *
CQChartsPlot::
addProperty(const QString &path, QObject *object, const QString &name, const QString &alias)
{
  assert(CQUtil::hasProperty(object, name));

  return propertyModel()->addProperty(path, object, name, alias);
}

void
CQChartsPlot::
propertyItemSelected(QObject *obj, const QString &)
{
  startSelection();

  view()->deselectAll();

  bool changed = false;

  if      (obj == this) {
    setSelected(true);

    view()->setCurrentPlot(this);

    drawObjs();

    changed = true;
  }
  else if (obj == titleObj_) {
    titleObj_->setSelected(true);

    drawForeground();

    changed = true;
  }
  else if (obj == keyObj_) {
    keyObj_->setSelected(true);

    drawBackground();
    drawForeground();

    changed = true;
  }
  else if (obj == xAxis_) {
    xAxis_->setSelected(true);

    drawBackground();
    drawForeground();

    changed = true;
  }
  else if (obj == yAxis_) {
    yAxis_->setSelected(true);

    drawBackground();
    drawForeground();

    changed = true;
  }
  else {
    for (const auto &annotation : annotations()) {
      if (obj == annotation) {
        annotation->setSelected(true);

        drawForeground();

        changed = true;
      }
    }

    if (! changed) {
      for (auto &plotObj : plotObjects()) {
        if (obj == plotObj) {
          plotObj->setSelected(true);

          changed = true;
        }
      }
    }
  }

  endSelection();

  //---

  if (changed)
    invalidateOverlay();
}

void
CQChartsPlot::
getPropertyNames(QStringList &names, bool hidden) const
{
  propertyModel()->objectNames(this, names, hidden);

  auto getObjPropertyNames = [&](std::initializer_list<CQChartsObj *> objs) {
    for (const auto &obj : objs) {
      if (obj)
        propertyModel()->objectNames(obj, names, hidden);
    }
  };

  getObjPropertyNames({title(), xAxis(), yAxis(), key()});
}

void
CQChartsPlot::
getObjectPropertyNames(CQChartsPlotObj *plotObj, QStringList &names) const
{
  names = CQUtil::getPropertyList(plotObj, /*inherited*/ false);
}

void
CQChartsPlot::
hideProperty(const QString &path, QObject *object)
{
  propertyModel()->hideProperty(path, object);
}

//------

void
CQChartsPlot::
addAxes()
{
  addXAxis();
  addYAxis();
}

void
CQChartsPlot::
addXAxis()
{
  xAxis_ = new CQChartsAxis(this, Qt::Horizontal, 0.0, 1.0);

  xAxis_->setObjectName("xaxis");
}

void
CQChartsPlot::
addYAxis()
{
  yAxis_ = new CQChartsAxis(this, Qt::Vertical, 0.0, 1.0);

  yAxis_->setObjectName("yaxis");
}

//------

void
CQChartsPlot::
addKey()
{
  keyObj_ = new CQChartsPlotKey(this);
}

void
CQChartsPlot::
resetKeyItems()
{
  CQPerfTrace trace("CQChartsPlot::resetKeyItems");

  if (isOverlay()) {
    // if first plot then add all chained plot items to this plot's key
    if (prevPlot())
      return;

    //---

    if (key()) {
      key()->clearItems();

      processOverlayPlots([&](CQChartsPlot *plot) {
        plot->doAddKeyItems(key());
      });
    }
  }
  else {
    if (key()) {
      key()->clearItems();

      doAddKeyItems(key());
    }
  }
}

void
CQChartsPlot::
doAddKeyItems(CQChartsPlotKey *key)
{
  // add key items from color column
  if (isColorKey()) {
    if (addColorKeyItems(key))
      return;
  }

  addKeyItems(key);
}

bool
CQChartsPlot::
addColorKeyItems(CQChartsPlotKey *key)
{
  int row = (! key->isHorizontal() ? key->maxRow() : 0);
  int col = (! key->isHorizontal() ? 0 : key->maxCol());

  auto addKeyRow = [&](const QString &name, const QColor &c) {
    auto *keyColor = new CQChartsKeyColorBox(this, ColorInd(), ColorInd(), ColorInd());
    auto *keyText  = new CQChartsKeyText    (this, name, ColorInd());

    keyColor->setColor(c);

    if (! key->isHorizontal()) {
      key->addItem(keyColor, row, 0);
      key->addItem(keyText , row, 1);

      ++row;
    }
    else {
      key->addItem(keyColor, 0, col++);
      key->addItem(keyText , 0, col++);
    }

    return keyColor;
  };

  if (! colorColumn().isValid())
    return false;

  CQChartsModelColumnDetails *columnDetails = this->columnDetails(colorColumn());
  if (! columnDetails) return false;

  auto uniqueValues = columnDetails->uniqueValues();

  for (auto &value : uniqueValues) {
    QString name;

    if (! CQChartsVariant::toString(value, name))
      name = value.toString();

    CQChartsColor color;

    columnValueColor(value, color);

    QColor c = interpColor(color, ColorInd());

    auto *keyColor = addKeyRow(name, c);

    keyColor->setValue(value);
  }

  return true;
}

//------

void
CQChartsPlot::
addTitle()
{
  titleObj_ = new CQChartsTitle(this);

  titleObj_->setTextStr(titleStr());
}

//------

void
CQChartsPlot::
threadTimerSlot()
{
  if (isOverlay()) {
    if (! isFirstPlot())
      return firstPlot()->startThreadTimer();
  }

  //---

  UpdateState updateState = this->updateState();
  UpdateState nextState   = UpdateState::INVALID;
  bool        updateView  = false;

  {
  TryLockMutex lock(this, "threadTimerSlot");

  if (! lock.locked)
    return;

  //---

  if (isUpdatesEnabled())
    nextState = calcNextState();

  //---

  // ensure threads state is up to date (thread may have finished)

  if      (updateState == UpdateState::CALC_RANGE) {
    // check if calc range finished
    if (! updateData_.rangeThread.busy.load()) {
      updateData_.rangeThread.finish(this, debugUpdate_ ? "updateRange" : nullptr);

      // ensure all overlay plot ranges done
      if (isOverlay())
        waitRange();

      // need post update range
      applyDataRange();

      // post update range
      postUpdateRange();

      // reset state
      setGroupedUpdateState(UpdateState::INVALID);
    }
    // update range running so redraw view (busy)
    else {
      updateView = true;
    }
  }
  else if (updateState == UpdateState::CALC_OBJS) {
    // check if calc objs finished
    if (! updateData_.objsThread.busy.load()) {
      updateData_.objsThread.finish(this, debugUpdate_ ? "updateObjs" : nullptr);

      // ensure all overlay plot objs done
      if (isOverlay())
        waitObjs();

      // post update objs
      postUpdateObjs();

      // reset state
      setGroupedUpdateState(UpdateState::INVALID);
    }
    // update objs running so redraw view (busy)
    else {
      updateView = true;
    }
  }
  else if (updateState == UpdateState::DRAW_OBJS) {
    // check if draw objs finished
    if (! updateData_.drawThread.busy.load()) {
      updateData_.drawThread.finish(this, debugUpdate_ ? "drawObjs" : nullptr);

      // ensure all overlay draw done
      if (isOverlay())
        waitDraw();

      // post draw
      postDraw();

      // move to ready state (all done)
      setGroupedUpdateState(UpdateState::READY);

      // need draw
      updateView = true;
    }
    // draw running so redraw view (busy)
    else {
      updateView = true;
    }
  }
  else if (updateState == UpdateState::INVALID) {
    //if (nextState == UpdateState::INVALID) {
    //  ++updatesData_.stateFlag[UpdateState::UPDATE_RANGE    ];
    //  ++updatesData_.stateFlag[UpdateState::UPDATE_OBJS     ];
    //  ++updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS];

    //  nextState = UpdateState::UPDATE_RANGE;
    //}
  }
  else if (updateState == UpdateState::DRAWN) {
    if (! isPlotObjTreeSet())
      objTreeData_.tree->waitTree();
  }
  else {
    updateView = true;

    setGroupedUpdateState(nextState);
  }
  }

  //---

  if (objTreeData_.notify) {
    objTreeData_.notify = false;

    postObjTree();
  }

  //---

  if      (nextState == UpdateState::UPDATE_RANGE) {
    updatesData_.stateFlag[UpdateState::UPDATE_RANGE] = 0;

    this->execUpdateRange();
  }
  else if (nextState == UpdateState::UPDATE_OBJS) {
    // don't update until range calculated
    if (updateState != UpdateState::CALC_RANGE) {
      updatesData_.stateFlag[UpdateState::UPDATE_OBJS] = 0;

      this->execUpdateObjs();
    }
  }
  else if (nextState == UpdateState::UPDATE_DRAW_OBJS) {
    // don't update until range and objs calculated
    if (updateState != UpdateState::CALC_RANGE &&
        updateState != UpdateState::CALC_OBJS) {
      updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS      ] = 0;
      updatesData_.stateFlag[UpdateState::UPDATE_DRAW_BACKGROUND] = 0;
      updatesData_.stateFlag[UpdateState::UPDATE_DRAW_FOREGROUND] = 0;

      this->invalidateLayers();

      updateView = true;
    }
  }
  else if (nextState == UpdateState::UPDATE_DRAW_BACKGROUND) {
    // don't update until objs drawn
    if (updateState != UpdateState::DRAW_OBJS) {
      updatesData_.stateFlag[UpdateState::UPDATE_DRAW_BACKGROUND] = 0;

      this->invalidateLayer(CQChartsBuffer::Type::BACKGROUND);

      this->invalidateOverlay();

      updateView = true;
    }
  }
  else if (nextState == UpdateState::UPDATE_DRAW_FOREGROUND) {
    // don't update until objs drawn
    if (updateState != UpdateState::DRAW_OBJS) {
      updatesData_.stateFlag[UpdateState::UPDATE_DRAW_FOREGROUND] = 0;

      this->invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

      this->invalidateOverlay();

      updateView = true;
    }
  }

  //---

  if (updateView)
    view()->update();
}

CQChartsPlot::UpdateState
CQChartsPlot::
calcNextState() const
{
  auto *th = const_cast<CQChartsPlot *>(this);

  UpdateState nextState = UpdateState::INVALID;

  // check queued updates to determine required state
  if      (th->updatesData_.stateFlag[UpdateState::UPDATE_RANGE] > 0) {
    if (debugUpdate_)
      std::cerr << "UpdateState::UPDATE_RANGE : " <<
        th->updatesData_.stateFlag[UpdateState::UPDATE_RANGE] << "\n";

    nextState = UpdateState::UPDATE_RANGE;
  }
  else if (th->updatesData_.stateFlag[UpdateState::UPDATE_OBJS] > 0) {
    if (debugUpdate_)
      std::cerr << "UpdateState::UPDATE_OBJS : " <<
        th->updatesData_.stateFlag[UpdateState::UPDATE_OBJS] << "\n";

    nextState = UpdateState::UPDATE_OBJS;
  }
  else if (th->updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS] > 0) {
    if (debugUpdate_)
      std::cerr << "UpdateState::UPDATE_DRAW_OBJS : " <<
        th->updatesData_.stateFlag[UpdateState::UPDATE_DRAW_OBJS] << "\n";

    nextState = UpdateState::UPDATE_DRAW_OBJS;
  }
  else if (th->updatesData_.stateFlag[UpdateState::UPDATE_DRAW_BACKGROUND] > 0) {
    if (debugUpdate_)
      std::cerr << "UpdateState::UPDATE_DRAW_BACKGROUND : " <<
        th->updatesData_.stateFlag[UpdateState::UPDATE_DRAW_BACKGROUND] << "\n";

    nextState = UpdateState::UPDATE_DRAW_BACKGROUND;
  }
  else if (th->updatesData_.stateFlag[UpdateState::UPDATE_DRAW_FOREGROUND] > 0) {
    if (debugUpdate_)
      std::cerr << "UpdateState::UPDATE_DRAW_FOREGROUND : " <<
        th->updatesData_.stateFlag[UpdateState::UPDATE_DRAW_FOREGROUND] << "\n";

    nextState = UpdateState::UPDATE_DRAW_FOREGROUND;
  }

  return nextState;
}

void
CQChartsPlot::
setGroupedUpdateState(UpdateState state)
{
  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->setUpdateState(state);
    });
  }
  else {
    setUpdateState(state);
  }
}

void
CQChartsPlot::
setUpdateState(UpdateState state)
{
  updateData_.state.store((int) state);

  if (debugUpdate_) {
    std::cerr << "State: " << id().toStdString() << ": ";

    UpdateState updateState = this->updateState();

    switch (updateState) {
      case UpdateState::CALC_RANGE:       std::cerr << "Calc Range\n"; break;
      case UpdateState::CALC_OBJS:        std::cerr << "Calc Objs\n"; break;
      case UpdateState::DRAW_OBJS:        std::cerr << "Draw Objs\n"; break;
      case UpdateState::READY:            std::cerr << "Ready\n"; break;
      case UpdateState::UPDATE_RANGE:     std::cerr << "Update Range\n"; break;
      case UpdateState::UPDATE_OBJS:      std::cerr << "Update Objs\n"; break;
      case UpdateState::UPDATE_DRAW_OBJS: std::cerr << "Update Draw Objs\n"; break;
      case UpdateState::UPDATE_VIEW:      std::cerr << "Update View\n"; break;
      case UpdateState::DRAWN:            std::cerr << "Drawn\n"; break;
      default:                            std::cerr << "Invalid\n"; break;
    }
  }
}

void
CQChartsPlot::
setInterrupt(bool b)
{
  if (b)
    updateData_.interrupt++;
  else {
    assert(updateData_.interrupt.load() > 0);

    updateData_.interrupt--;
  }
}

bool
CQChartsPlot::
isReady() const
{
  auto updateState = const_cast<CQChartsPlot *>(this)->updateState();

  return (updateState == UpdateState::READY || updateState == UpdateState::DRAWN);
}

//------

void
CQChartsPlot::
clearRangeAndObjs()
{
  resetRange();

  clearPlotObjects();
}

void
CQChartsPlot::
execUpdateRangeAndObjs()
{
  if (! isUpdatesEnabled()) {
    updatesData_.updateRangeAndObjs = true;
    return;
  }

  updateAndApplyRange(/*apply*/true, /*updateObjs*/true);
}

void
CQChartsPlot::
resetRange()
{
  dataRange_.reset();
}

//------

void
CQChartsPlot::
execUpdateRange()
{
  updateAndApplyRange(/*apply*/true, /*updateObjs*/false);
}

void
CQChartsPlot::
updateAndApplyRange(bool apply, bool updateObjs)
{
  CQPerfTrace trace("CQChartsPlot::updateAndApplyRange");

  // update overlay objects
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->updateAndApplyPlotRange(apply, updateObjs);
    });
  }
  else {
    updateAndApplyPlotRange(apply, updateObjs);
  }
}

void
CQChartsPlot::
updateAndApplyPlotRange(bool apply, bool updateObjs)
{
  if (apply) {
    updateAndApplyPlotRange1(updateObjs);
  }
  else {
    updateRangeThread();

    if (updateObjs)
      this->updateObjs();
  }
}

void
CQChartsPlot::
updateAndApplyPlotRange1(bool updateObjs)
{
  if (! isSequential()) {
    LockMutex lock1(this, "updateAndApplyPlotRange");

    // finish current threads
    interruptRange();

    //---

    // start update range thread
    updateData_.updateObjs   = updateObjs;
    updateData_.drawBusy.ind = 0;

    setUpdateState(UpdateState::CALC_RANGE);

    updateData_.rangeThread.start(this, debugUpdate_ ? "updateRange" : nullptr);
    updateData_.rangeThread.future = std::async(std::launch::async, updateRangeASync, this);

    startThreadTimer();
  }
  else {
    updateRangeThread();

    if (updateObjs) {
      // calc range
      applyDataRange();

      postUpdateRange();

      //---

      // add objects
      this->updateObjs();

      postUpdateObjs();
    }

    // draw objects
    drawObjs();

    postDraw();
  }
}

void
CQChartsPlot::
interruptRange()
{
  setInterrupt(true);

  waitRange();
  waitObjs ();
  waitDraw ();

  setInterrupt(false);
}

void
CQChartsPlot::
syncAll()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::syncAll\n";

  syncRange();
  syncObjs ();
  syncDraw ();
}

void
CQChartsPlot::
syncState()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::syncState\n";

  if (isOverlay() && ! isFirstPlot())
    return;

  UpdateState updateState = this->updateState();

  while (updateState == UpdateState::INVALID) {
    if (isInterrupt())
      return;

    if (hasLockId())
      return;

    threadTimerSlot();

    updateState = this->updateState();
  }
}

void
CQChartsPlot::
syncRange()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::syncRange\n";

  syncState();

  waitRange();
}

void
CQChartsPlot::
waitRange()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::waitRange\n";

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->waitRange1();
    });
  }
  else {
    waitRange1();
  }
}

void
CQChartsPlot::
waitRange1()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::waitRange1\n";

  UpdateState updateState = this->updateState();

  while (updateState == UpdateState::CALC_RANGE) {
    if (updateData_.rangeThread.busy.load()) {
      (void) updateData_.rangeThread.future.get();

      assert(! updateData_.rangeThread.future.valid());

      updateData_.rangeThread.finish(this, debugUpdate_ ? "updateRange" : nullptr);

      return;
    }

    if (isInterrupt())
      return;

    if (hasLockId())
      return;

    if (! isFirstPlot())
      break;

    threadTimerSlot();

    updateState = this->updateState();
  }
}

void
CQChartsPlot::
updateRangeASync(CQChartsPlot *plot)
{
  plot->updateRangeThread();
}

void
CQChartsPlot::
updateRangeThread()
{
  CQPerfTrace trace("CQChartsPlot::updateRangeThread");

  //---

  annotationBBox_ = CQChartsGeom::BBox();
  calcDataRange_  = calcRange();
  dataRange_      = adjustDataRange(calcDataRange_);
  outerDataRange_ = dataRange_;

  //---

  if (debugUpdate_)
    std::cerr << "updateRangeThread: " << id().toStdString() << " : " << dataRange_ << "\n";

  updateData_.rangeThread.end(this, debugUpdate_ ? "updateRange" : nullptr);

  if (isOverlay())
    updateOverlayRanges();
}

//------

void
CQChartsPlot::
updateGroupedObjs()
{
  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->execUpdateObjs();
    });
  }
  else {
    execUpdateObjs();
  }
}

void
CQChartsPlot::
execUpdateObjs()
{
  if (! isUpdatesEnabled()) {
    updatesData_.updateObjs = true;
    return;
  }

  //---

  if (! dataRange_.isSet()) {
    execUpdateRangeAndObjs();
    return;
  }

  // update overlay objects
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->updatePlotObjs();
    });
  }
  else {
    updatePlotObjs();
  }
}

void
CQChartsPlot::
updatePlotObjs()
{
  if (! isSequential()) {
    LockMutex lock(this, "updatePlotObjs");

    //---

    UpdateState updateState = this->updateState();

    // if calc range still running then run update objects after finished
    if (updateState == UpdateState::CALC_RANGE) {
      if (isOverlay())
        firstPlot()->updateData_.updateObjs = true;
      //return;
    }

    //---

    // finish update objs thread
    interruptObjs();

    //---

    // start update objs thread
    updateData_.drawBusy.ind = 0;

    setUpdateState(UpdateState::CALC_OBJS);

    updateData_.objsThread.start(this, debugUpdate_ ? "updatePlotObjs" : nullptr);
    updateData_.objsThread.future = std::async(std::launch::async, updateObjsASync, this);

    startThreadTimer();
  }
  else {
    // add objs
    // TODO: non threaded version ?
    updateObjsThread();

    postUpdateObjs();

    //---

    // draw objects
    drawObjs();

    postDraw();
  }
}

void
CQChartsPlot::
interruptObjs()
{
  setInterrupt(true);

  waitObjs();
  waitDraw();

  setInterrupt(false);
}

void
CQChartsPlot::
syncObjs()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::syncObjs\n";

  syncState();

  waitRange();
  waitObjs ();
}

void
CQChartsPlot::
waitObjs()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::waitObjs\n";

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->waitObjs1();
    });
  }
  else {
    waitObjs1();
  }
}

void
CQChartsPlot::
waitObjs1()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::waitObjs1\n";

  UpdateState updateState = this->updateState();

  while (updateState == UpdateState::CALC_OBJS) {
    if (updateData_.objsThread.busy.load()) {
      (void) updateData_.objsThread.future.get();

      assert(! updateData_.objsThread.future.valid());

      updateData_.objsThread.finish(this, debugUpdate_ ? "waitObjs1" : nullptr);

      return;
    }

    if (isInterrupt())
      return;

    if (hasLockId())
      return;

    if (! isFirstPlot())
      break;

    threadTimerSlot();

    updateState = this->updateState();
  }
}

void
CQChartsPlot::
updateObjsASync(CQChartsPlot *plot)
{
  plot->updateObjsThread();
}

void
CQChartsPlot::
updateObjsThread()
{
  CQPerfTrace trace("CQChartsPlot::updateObjsThread");

  //---

  clearPlotObjects();

  initColorColumnData();

  initPlotObjs();

  updateData_.objsThread.end(this, debugUpdate_ ? "updateObjsThread" : nullptr);
}

void
CQChartsPlot::
postInit()
{
}

CQChartsGeom::BBox
CQChartsPlot::
calcDataRange(bool adjust) const
{
  auto bbox = getDataRange();

  // if zoom data, adjust bbox by pan offset, zoom scale
  if (adjust)
    bbox = adjustDataRangeBBox(bbox);

  return bbox;
}

void
CQChartsPlot::
calcDataRanges(CQChartsGeom::BBox &rawRange, CQChartsGeom::BBox &adjustedRange) const
{
  rawRange = getDataRange();

  // if zoom data, adjust bbox by pan offset, zoom scale
  adjustedRange = adjustDataRangeBBox(rawRange);
}

CQChartsGeom::BBox
CQChartsPlot::
adjustDataRangeBBox(const CQChartsGeom::BBox &bbox) const
{
  // get center and x/y sizes
  auto c = bbox.getCenter();

  double bw = bbox.getWidth ();
  double bh = bbox.getHeight();

  // adjust by scale
  double w = 0.5*bw/dataScaleX();
  double h = 0.5*bh/dataScaleY();
  double x = c.x + bw*dataOffsetX();
  double y = c.y + bh*dataOffsetY();

  // calc scaled/offset bbox
  auto bbox1 = CQChartsGeom::BBox(x - w, y - h, x + w, y + h);

  //----

  // save original range
  CQChartsDisplayRange displayRange = this->displayRange();

  // update to calculated range
  auto dataRange = calcDataRange(/*adjust*/false);

  auto *th = const_cast<CQChartsPlot *>(this);

  //th->setWindowRange(dataRange);
  th->displayRange_->setWindowRange(dataRange.getXMin(), dataRange.getYMin(),
                                    dataRange.getXMax(), dataRange.getYMax());

  //--

  // adjust range to margin
  CQChartsGeom::BBox ibbox;

  if (isOverlay()) {
    const CQChartsPlotMargin &innerMargin = firstPlot()->innerMargin();

    ibbox = innerMargin.adjustPlotRange(this, bbox1, /*inside*/true);
  }
  else {
    ibbox = innerMargin().adjustPlotRange(this, bbox1, /*inside*/true);
  }

  //--

  // restore original range
  const_cast<CQChartsPlot *>(this)->setDisplayRange(displayRange);

  return ibbox;
}

CQChartsGeom::BBox
CQChartsPlot::
getDataRange() const
{
  if (dataRange_.isSet())
    return CQChartsUtil::rangeBBox(dataRange_);
  else
    return CQChartsGeom::BBox(0.0, 0.0, 1.0, 1.0);
}

void
CQChartsPlot::
applyDataRangeAndDraw()
{
  applyDataRange();

  drawObjs();

  if (! isOverlay()) {
    if (isX1X2() || isY1Y2()) {
      CQChartsPlot *plot1 = firstPlot();

      while (plot1) {
        if (plot1 != this)
          plot1->drawObjs();

        plot1 = plot1->nextPlot();
      }
    }
  }
}

void
CQChartsPlot::
applyDataRange(bool propagate)
{
  if (propagate) {
    if (isOverlay() && ! isFirstPlot()) {
      return firstPlot()->applyDataRange(propagate);
    }
  }

  //---

  if (! isUpdatesEnabled()) {
    updatesData_.applyDataRange = true;
    return;
  }

  if (! dataRange_.isSet()) {
    updatesData_.applyDataRange = true;
    return;
  }

  //---

  CQChartsGeom::BBox rawRange, adjustedRange;

  if (propagate) {
    if (isOverlay()) {
      if      (isX1X2()) {
        CQChartsPlot *plot1 = firstPlot();

        plot1->calcDataRanges(rawRange, adjustedRange);
      }
      else if (isY1Y2()) {
        CQChartsPlot *plot1 = firstPlot();

        plot1->calcDataRanges(rawRange, adjustedRange);
      }
      else {
        processOverlayPlots([&](CQChartsPlot *plot) {
          //plot->resetDataRange(/*updateRange*/false, /*updateObjs*/false);
          //plot->updateAndApplyRange(/*apply*/false, /*updateObjs*/false);

          CQChartsGeom::BBox rawRange1, adjustedRange1;

          plot->calcDataRanges(rawRange1, adjustedRange1);

          rawRange      += rawRange1;
          adjustedRange += adjustedRange1;
        });
      }
    }
    else {
      calcDataRanges(rawRange, adjustedRange);
    }
  }
  else {
    calcDataRanges(rawRange, adjustedRange);
  }

  if (isOverlay()) {
    if (! propagate) {
      if      (isX1X2()) {
        setWindowRange(adjustedRange);
      }
      else if (isY1Y2()) {
        setWindowRange(adjustedRange);
      }
    }
    else {
      // This breaks X1X2 and Y1Y2 plots (wrong range)
      if (! isX1X2() && ! isY1Y2()) {
        processOverlayPlots([&](CQChartsPlot *plot) {
          plot->setWindowRange(adjustedRange);
        });
      }
    }
  }
  else {
    setWindowRange(adjustedRange);
  }

  if (xAxis())
    xAxis()->setRange(adjustedRange.getXMin(), adjustedRange.getXMax());

  if (yAxis())
    yAxis()->setRange(adjustedRange.getYMin(), adjustedRange.getYMax());

  if (propagate) {
    if (isOverlay()) {
      if      (isX1X2()) {
        auto dataRange1 = CQChartsUtil::bboxRange(rawRange);

        CQChartsPlot *plot1, *plot2;

        x1x2Plots(plot1, plot2);

        if (plot1) {
          plot1->applyDataRange(/*propagate*/false);
        }

        //---

        if (plot2) {
          //plot2->resetDataRange(/*updateRange*/false, /*updateObjs*/false);
          //plot2->updateAndApplyRange(/*apply*/false, /*updateObjs*/false);

          auto bbox2 = plot2->calcDataRange(/*adjust*/false);

          CQChartsGeom::Range dataRange2 =
            CQChartsGeom::Range(bbox2.getXMin(), dataRange1.bottom(),
                                bbox2.getXMax(), dataRange1.top   ());

          plot2->setDataRange(dataRange2, /*update*/false);

          plot2->applyDataRange(/*propagate*/false);
        }
      }
      else if (isY1Y2()) {
        auto dataRange1 = CQChartsUtil::bboxRange(rawRange);

        CQChartsPlot *plot1, *plot2;

        y1y2Plots(plot1, plot2);

        if (plot1) {
          plot1->applyDataRange(/*propagate*/false);
        }

        //---

        if (plot2) {
          //plot2->resetDataRange(/*updateRange*/false, /*updateObjs*/false);
          //plot2->updateAndApplyRange(/*apply*/false, /*updateObjs*/false);

          auto bbox2 = plot2->calcDataRange(/*adjust*/false);

          CQChartsGeom::Range dataRange2 =
            CQChartsGeom::Range(dataRange1.left (), bbox2.getYMin(),
                                dataRange1.right(), bbox2.getYMax());

          plot2->setDataRange(dataRange2, /*update*/false);

          plot2->applyDataRange(/*propagate*/false);
        }
      }
      else {
        auto dataRange1 = CQChartsUtil::bboxRange(rawRange);

        processOverlayPlots([&](CQChartsPlot *plot) {
          plot->setDataRange(dataRange1, /*update*/false);

          plot->applyDataRange(/*propagate*/false);
        });
      }
    }
    else {
      if (isX1X2() || isY1Y2()) {
        auto dataRange1 = CQChartsUtil::bboxRange(rawRange);

        CQChartsPlot *plot1 = firstPlot();

        while (plot1) {
          if (plot1 != this) {
            auto bbox2 = plot1->calcDataRange(/*adjust*/false);

            if (isX1X2()) {
              double ymin = std::min(bbox2.getYMin(), dataRange1.bottom());
              double ymax = std::max(bbox2.getYMax(), dataRange1.top   ());

              CQChartsGeom::Range dataRange1(rawRange.getXMin(), ymin, rawRange.getXMax(), ymax);
              CQChartsGeom::Range dataRange2(bbox2   .getXMin(), ymin, bbox2   .getXMax(), ymax);

              this ->setDataRange(dataRange1, /*update*/false);
              plot1->setDataRange(dataRange2, /*update*/false);

              plot1->applyDataRange(/*propagate*/false);
            }
            else {
              double xmin = std::min(bbox2.getXMin(), dataRange1.left ());
              double xmax = std::max(bbox2.getXMax(), dataRange1.right());

              CQChartsGeom::Range dataRange1(xmin, rawRange.getYMin(), xmax, rawRange.getYMax());
              CQChartsGeom::Range dataRange2(xmin, bbox2   .getYMin(), xmax, bbox2   .getYMax());

              this ->setDataRange(dataRange1, /*update*/false);
              plot1->setDataRange(dataRange2, /*update*/false);

              plot1->applyDataRange(/*propagate*/false);
            }
          }

          plot1 = plot1->nextPlot();
        }
      }
    }
  }

  updateKeyPosition(/*force*/true);

  emit rangeChanged();
}

void
CQChartsPlot::
updateOverlayRanges()
{
  if (! isFirstPlot())
    return firstPlot()->updateOverlayRanges();

  processOverlayPlots([&](CQChartsPlot *plot) {
    if (plot != this) {
      plot->setDataRange(dataRange(), /*update*/false);

      plot->applyDataRange(/*propagate*/false);
    }
  });
}

void
CQChartsPlot::
setPixelRange(const CQChartsGeom::BBox &bbox)
{
  displayRange_->setPixelRange(bbox.getXMin(), bbox.getYMax(), bbox.getXMax(), bbox.getYMin());
}

void
CQChartsPlot::
resetWindowRange()
{
  displayRange_->setWindowRange(0.0, 0.0, 1.0, 1.0);
}

void
CQChartsPlot::
setWindowRange(const CQChartsGeom::BBox &bbox)
{
  displayRange_->setWindowRange(bbox.getXMin(), bbox.getYMin(), bbox.getXMax(), bbox.getYMax());
}

CQChartsGeom::Range
CQChartsPlot::
adjustDataRange(const CQChartsGeom::Range &calcDataRange) const
{
  auto dataRange = calcDataRange;

  // adjust data range to custom values
  if (xmin().isSet()) dataRange.setLeft  (xmin().real());
  if (ymin().isSet()) dataRange.setBottom(ymin().real());
  if (xmax().isSet()) dataRange.setRight (xmax().real());
  if (ymax().isSet()) dataRange.setTop   (ymax().real());

  if (xAxis() && xAxis()->isIncludeZero()) {
    if (dataRange.isSet())
      dataRange.updateRange(0, dataRange.ymid());
  }

  if (yAxis() && yAxis()->isIncludeZero()) {
    if (dataRange.isSet())
      dataRange.updateRange(dataRange.xmid(), 0);
  }

  return dataRange;
}

//------

CQChartsGeom::BBox
CQChartsPlot::
calcGroupedDataRange(bool includeAnnotation) const
{
  CQChartsGeom::BBox bbox;

  if (isOverlay()) {
    processOverlayPlots([&](const CQChartsPlot *plot) {
      auto bbox1 = plot->calcDataRange();
      if (! bbox1.isSet()) return;

      if (bbox1.isSet() && includeAnnotation)
        bbox1 += plot->annotationBBox();

      if (plot != this)
        bbox1 = viewToWindow(plot->windowToView(bbox1));

      if (bbox1.isSet())
        bbox += bbox1;
    });

    if (! bbox.isSet())
      bbox = CQChartsGeom::BBox(0, 0, 1, 1);
  }
  else {
    bbox = calcDataRange();

    if (bbox.isSet() && includeAnnotation)
      bbox += annotationBBox();
    else
      bbox = CQChartsGeom::BBox(0, 0, 1, 1);
  }

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
calcGroupedXAxisRange(const CQChartsAxisSide::Type &side) const
{
  assert(xAxis());

  CQChartsGeom::BBox xbbox;

  if (isOverlay()) {
    processOverlayPlots([&](const CQChartsPlot *plot) {
      if (plot->xAxis() && plot->xAxis()->side() == side) {
        auto xbbox1 = plot->xAxis()->bbox();
        if (! xbbox1.isSet()) return;

        if (plot != this)
          xbbox1 = viewToWindow(plot->windowToView(xbbox1));

        xbbox += xbbox1;
      }
    });
  }
  else {
    if (xAxis()->side() == side)
      xbbox = xAxis()->bbox();
  }

  if (! xbbox.isSet())
    xbbox = CQChartsGeom::BBox(0, 0, 0, 0);

  return xbbox;
}

CQChartsGeom::BBox
CQChartsPlot::
calcGroupedYAxisRange(const CQChartsAxisSide::Type &side) const
{
  assert(yAxis());

  CQChartsGeom::BBox ybbox;

  if (isOverlay()) {
    processOverlayPlots([&](const CQChartsPlot *plot) {
      if (plot->yAxis() && plot->yAxis()->side() == side) {
        auto ybbox1 = plot->yAxis()->bbox();
        if (! ybbox1.isSet()) return;

        if (plot != this)
          ybbox1 = viewToWindow(plot->windowToView(ybbox1));

        ybbox += ybbox1;
      }
    });
  }
  else {
    if (yAxis()->side() == side)
      ybbox = yAxis()->bbox();
  }

  if (! ybbox.isSet())
    ybbox = CQChartsGeom::BBox(0, 0, 0, 0);

  return ybbox;
}

//------

void
CQChartsPlot::
addPlotObject(CQChartsPlotObj *obj)
{
  assert(! objTreeData_.tree->isBusy());

  plotObjs_.push_back(obj);

  // TODO: needed ? Do post thread finished
#if 0
  obj->moveToThread(this->thread());

  obj->setParent(this);
#endif

  //obj->addProperties(propertyModel(), "objects");
}

void
CQChartsPlot::
initGroupedPlotObjs()
{
  CQPerfTrace trace("CQChartsPlot::initGroupedPlotObjs");

  // init overlay plots before draw
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](CQChartsPlot *plot) {
      initPlotRange();

      plot->initPlotObjs();
    });
  }
  else {
    initPlotRange();

    initPlotObjs();
  }
}

bool
CQChartsPlot::
initPlotRange()
{
  if (! dataRange_.isSet()) {
    execUpdateRange();

    if (! dataRange_.isSet())
      return false;
  }

  return true;
}

void
CQChartsPlot::
initPlotObjs()
{
  CQPerfTrace trace("CQChartsPlot::initPlotObjs");

  //---

  bool changed = initObjs();

  // ensure some range defined
  if (! dataRange_.isSet()) {
    dataRange_.updateRange(0, 0);
    dataRange_.updateRange(1, 1);

    changed = true;
  }

  //---

  if (addNoDataObj())
    changed = true;

  //---

  // init search tree
  if (changed)
    objTreeData_.init = true;

  //---

  // auto fit
  if (changed && isAutoFit()) {
    needsAutoFit_ = true;
  }

  //---

  if (changed)
    emit plotObjsAdded();
}

bool
CQChartsPlot::
addNoDataObj()
{
  CQPerfTrace trace("CQChartsPlot::addNoDataObj");

  // if no objects then add a no data object
  noData_ = false;

  if (type()->hasObjs() && plotObjs_.empty()) {
    auto *obj = new CQChartsNoDataObj(this);

    addPlotObject(obj);

    noData_ = true;
  }

  return noData_;
}

bool
CQChartsPlot::
initObjs()
{
  CQPerfTrace trace("CQChartsPlot::initObjs");

  if (! plotObjs_.empty())
    return false;

  //---

  if (! createObjs())
    return false;

  //---

  resetKeyItems();

  return true;
}

bool
CQChartsPlot::
createObjs()
{
  annotationBBox_ = CQChartsGeom::BBox();

  //---

  PlotObjs objs;

  if (! createObjs(objs))
    return false;

  for (auto &obj : objs)
    addPlotObject(obj);

  return true;
}

QString
CQChartsPlot::
columnsHeaderName(const CQChartsColumns &columns) const
{
  QString str;

  for (const auto &column : columns.columns()) {
    QString str1 = columnHeaderName(column);
    if (! str1.length()) continue;

    if (str.length())
      str += ", ";

    str += str1;
  }

  return str;
}

QString
CQChartsPlot::
columnHeaderName(const CQChartsColumn &column) const
{
  auto p = columnNames_.find(column);
  if (p != columnNames_.end()) return (*p).second;

  bool ok;

  QString str = modelHHeaderString(column, ok);
  if (! ok) return "";

  return str;
}

void
CQChartsPlot::
updateColumnNames()
{
  setColumnHeaderName(idColumn   (), "Id"   );
  setColumnHeaderName(colorColumn(), "Color");
  setColumnHeaderName(imageColumn(), "Image");
}

void
CQChartsPlot::
setColumnHeaderName(const CQChartsColumn &column, const QString &def)
{
  bool ok;

  QString str = modelHHeaderString(column, ok);
  if (! str.length()) str = def;

  columnNames_[column] = str;
}

void
CQChartsPlot::
initObjTree()
{
  CQPerfTrace trace("CQChartsPlot::initObjTree");

  if (! isPreview())
    objTreeData_.tree->addObjects();
}

void
CQChartsPlot::
clearPlotObjects()
{
  CQPerfTrace trace("CQChartsPlot::clearPlotObjects");

  objTreeData_.tree->clearObjects();

  PlotObjs plotObjs;

  std::swap(plotObjs, plotObjs_);

#if 0
  for (auto &plotObj : plotObjs)
    propertyModel()->removeProperties("objects/" + plotObj->propertyId());
#endif

  for (auto &plotObj : plotObjs)
    delete plotObj;

  insideObjs_    .clear();
  sizeInsideObjs_.clear();
}

CQChartsGeom::BBox
CQChartsPlot::
findEmptyBBox(double w, double h) const
{
  return objTreeData_.tree->findEmptyBBox(w, h);
}

//------

bool
CQChartsPlot::
updateInsideObjects(const CQChartsGeom::Point &w)
{
  // get objects at point
  Objs objs;

  objsAtPoint(w, objs);

  //---

  // check if changed
  bool changed = false;

  if (objs.size() == insideObjs_.size()) {
    for (const auto &obj : objs) {
      if (insideObjs_.find(obj) == insideObjs_.end()) {
        changed = true;
        break;
      }
    }
  }
  else {
    changed = true;
  }

  //---

  // if changed update inside objects
  if (changed) {
    // reset current inside index
    insideInd_ = 0;

    //---

    // reset inside objects
    if (isOverlay()) {
      processOverlayPlots([&](CQChartsPlot *plot) {
        plot->resetInsideObjs();
      });
    }
    else {
      resetInsideObjs();
    }

    //---

    // set new inside objects (and inside objects sorted by size)
    sizeInsideObjs_.clear();

    for (const auto &obj : objs) {
      insideObjs_.insert(obj);

      sizeInsideObjs_[obj->rect().area()].insert(obj);
    }

    // set current inside obj
    setInsideObject();
  }

  //---

  return changed;
}

void
CQChartsPlot::
resetInsideObjs()
{
  insideObjs_.clear();

  for (auto &obj : plotObjects())
    obj->setInside(false);

  for (auto &annotation : annotations()) {
    annotation->setInside(false);
  }
}

CQChartsObj *
CQChartsPlot::
insideObject() const
{
  // get nth inside object
  int i = 0;

  for (const auto &sizeObjs : sizeInsideObjs_) {
    for (const auto &obj : sizeObjs.second) {
      if (i == insideInd_)
        return obj;

      ++i;
    }
  }

  return nullptr;
}

void
CQChartsPlot::
nextInsideInd()
{
  // cycle to next inside object
  ++insideInd_;

  if (insideInd_ >= int(insideObjs_.size()))
    insideInd_ = 0;
}

void
CQChartsPlot::
prevInsideInd()
{
  // cycle to prev inside object
  --insideInd_;

  if (insideInd_ < 0)
    insideInd_ = int(insideObjs_.size()) - 1;
}

void
CQChartsPlot::
setInsideObject()
{
  CQChartsObj *insideObj = insideObject();

  for (auto &obj : insideObjs_) {
    if (obj == insideObj)
      obj->setInside(true);
  }
}

QString
CQChartsPlot::
insideObjectText() const
{
  QString objText;

  for (const auto &sizeObjs : sizeInsideObjs_) {
    for (const auto &obj : sizeObjs.second) {
      if (obj->isInside()) {
        if (objText != "")
          objText += " ";

        objText += obj->id();
      }
    }
  }

  return objText;
}

//------

void
CQChartsPlot::
clearErrors()
{
  errorData_.clear();

  emit errorsCleared();
}

bool
CQChartsPlot::
hasErrors() const
{
  return errorData_.hasErrors();
}

bool
CQChartsPlot::
addError(const QString &msg)
{
  if (! isPreview()) {
    Error err { msg };

    errorData_.globalErrors.push_back(err);

    // TODO: add to log
    //charts()->errorMsg(msg);

    emit errorAdded();
  }

  return false;
}

bool
CQChartsPlot::
addColumnError(const CQChartsColumn &c, const QString &msg)
{
  if (! isPreview()) {
    ColumnError err { c, msg };

    errorData_.columnErrors.push_back(err);

    // TODO: add to log
    //charts()->errorMsg(msg);

    emit errorAdded();
  }

  return false;
}

bool
CQChartsPlot::
addDataError(const CQChartsModelIndex &ind, const QString &msg)
{
  if (! isPreview()) {
    DataError err { ind, msg };

    errorData_.dataErrors.push_back(err);

    // TODO: add to log
    //charts()->errorMsg(msg);

    emit errorAdded();
  }

  return false;
}

void
CQChartsPlot::
getErrors(QStringList &strs)
{
  if (! errorData_.globalErrors.empty()) {
    for (const auto &error : errorData_.globalErrors) {
      strs << error.msg;
    }
  }

  if (! errorData_.columnErrors.empty()) {
    for (const auto &error : errorData_.columnErrors) {
      QString msg = QString("Column %1 : %2").arg(error.column.toString()).arg(error.msg);

      strs << msg;
    }
  }

  if (! errorData_.dataErrors.empty()) {
    for (const auto &error : errorData_.dataErrors) {
      QString msg = QString("Ind %1 : %2").arg(error.ind.toString()).arg(error.msg);

      strs << msg;
    }
  }
}

void
CQChartsPlot::
addErrorsToWidget(QTextBrowser *text)
{
  CQChartsHtml html;

  if (! errorData_.globalErrors.empty()) {
    html.h2("Global Errors");

    for (const auto &error : errorData_.globalErrors) {
      html.p(error.msg);
    }
  }

  if (! errorData_.columnErrors.empty()) {
    html.h2("Column Errors");

    for (const auto &error : errorData_.columnErrors) {
      QString msg = QString("Column %1 : %2").arg(error.column.toString()).arg(error.msg);

      html.p(msg);
    }
  }

  if (! errorData_.dataErrors.empty()) {
    html.h2("Data Errors");

    for (const auto &error : errorData_.dataErrors) {
      QString msg = QString("Ind %1 : %2").arg(error.ind.toString()).arg(error.msg);

      html.p(msg);
    }
  }

  text->setHtml(html);
}

//------

bool
CQChartsPlot::
selectMousePress(const QPointF &p, SelMod selMod)
{
  if (! isReady()) return false;

  auto w = pixelToWindow(CQChartsGeom::Point(p));

  return selectPress(w, selMod);
}

bool
CQChartsPlot::
selectPress(const CQChartsGeom::Point &w, SelMod selMod)
{
  if (keySelectPress(key(), w, selMod))
    return true;

  if (titleSelectPress(title(), w, selMod))
    return true;

  //---

  if (annotationsSelectPress(w, selMod))
    return true;

  //---

  if (objectsSelectPress(w, selMod))
    return true;

  //---

  return true;
}

bool
CQChartsPlot::
keySelectPress(CQChartsPlotKey *key, const CQChartsGeom::Point &w, SelMod selMod)
{
  // select key
  if (key && key->contains(w)) {
    CQChartsKeyItem *item = key->getItemAt(w);

    if (item) {
      bool handled = item->selectPress(w, selMod);

      if (handled) {
        emit keyItemPressed  (item);
        emit keyItemIdPressed(item->id());

        return true;
      }
    }

    bool handled = key->selectPress(w, selMod);

    if (handled) {
      emit keyPressed  (key);
      emit keyIdPressed(key->id());

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
titleSelectPress(CQChartsTitle *title, const CQChartsGeom::Point &w, SelMod selMod)
{
  // select title
  if (title && title->contains(w)) {
    if (title->selectPress(w, selMod)) {
      emit titlePressed  (title);
      emit titleIdPressed(title->id());

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
annotationsSelectPress(const CQChartsGeom::Point &w, SelMod selMod)
{
  annotationsAtPoint(w, pressAnnotations_);

  for (const auto &annotation : pressAnnotations_) {
    annotation->mousePress(w, selMod);
  }

  for (const auto &annotation : pressAnnotations_) {
    if (! annotation->selectPress(w))
      continue;

    selectOneObj(annotation, /*allObjs*/true);

    drawForeground();

    emit annotationPressed  (annotation);
    emit annotationIdPressed(annotation->id());

    return true;
  }

  return false;
}

CQChartsObj *
CQChartsPlot::
objectsSelectPress(const CQChartsGeom::Point &w, SelMod selMod)
{
  // for replace init all objects to unselected
  // for add/remove/toggle init all objects to current state
  using ObjsSelected = std::map<CQChartsObj*,bool>;

  ObjsSelected objsSelected;

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      for (auto &plotObj : plot->plotObjects()) {
        if (selMod == SelMod::REPLACE)
          objsSelected[plotObj] = false;
        else
          objsSelected[plotObj] = plotObj->isSelected();
      }
    });
  }
  else {
    for (auto &plotObj : plotObjects()) {
      if (selMod == SelMod::REPLACE)
        objsSelected[plotObj] = false;
      else
        objsSelected[plotObj] = plotObj->isSelected();
    }
  }

  //---

  // get object under mouse
  CQChartsObj *selectObj = nullptr;

  if (isFollowMouse()) {
    selectObj = insideObject();

    nextInsideInd();

    setInsideObject();
  }
  else {
    Objs objs;

    objsAtPoint(w, objs);

    if (! objs.empty())
      selectObj = *objs.begin();
  }

  //---

  // change selection depending on selection modifier
  if (selectObj) {
    if      (selMod == SelMod::TOGGLE)
      objsSelected[selectObj] = ! selectObj->isSelected();
    else if (selMod == SelMod::REPLACE)
      objsSelected[selectObj] = true;
    else if (selMod == SelMod::ADD)
      objsSelected[selectObj] = true;
    else if (selMod == SelMod::REMOVE)
      objsSelected[selectObj] = false;

    //---

    //selectObj->selectPress();

    auto *selectPlotObj = dynamic_cast<CQChartsPlotObj *>(selectObj);

    if (selectPlotObj) {
      emit objPressed  (selectPlotObj);
      emit objIdPressed(selectPlotObj->id());
    }

    // potential crash if signals cause objects to be deleted (defer !!!)
  }

  //---

  // select objects and track if selection changed
  bool changed = false;

  auto setObjSelected = [&](CQChartsObj *obj, bool selected) {
    if (! changed) { startSelection(); changed = true; }

    obj->setSelected(selected);
  };

  for (const auto &objSelected : objsSelected) {
    if (objSelected.first->isSelected() == objSelected.second)
      continue;

    if (! objSelected.second)
      setObjSelected(objSelected.first, objSelected.second);
  }

  for (const auto &objSelected : objsSelected) {
    if (objSelected.first->isSelected() == objSelected.second)
      continue;

    if (objSelected.second)
      setObjSelected(objSelected.first, objSelected.second);
  }

  //----

  // update selection if changed
  if (changed) {
    beginSelectIndex();

    for (const auto &objSelected : objsSelected) {
      auto *selectPlotObj = dynamic_cast<CQChartsPlotObj *>(objSelected.first);

      if (! selectPlotObj || ! selectPlotObj->isSelected())
        continue;

      selectPlotObj->addSelectIndices();
    }

    endSelectIndex();

    //---

    invalidateOverlay();

    if (selectInvalidateObjs())
      drawObjs();

    //---

    endSelection();
  }

  //---

  return selectObj;
}

bool
CQChartsPlot::
selectMouseMove(const QPointF &pos, bool first)
{
  if (! isReady()) return false;

  auto p = CQChartsGeom::Point(pos);
  auto w = pixelToWindow(p);

  return selectMove(w, first);
}

bool
CQChartsPlot::
selectMove(const CQChartsGeom::Point &w, bool first)
{
  if (key()) {
    bool handled = key()->selectMove(w);

    if (handled)
      return true;
  }

  //---

  // select annotation
  Annotations annotations;

  annotationsAtPoint(w, annotations);

  for (const auto &annotation : annotations) {
    annotation->mouseMove(w);
  }

  //---

  QString objText;

  if (isFollowMouse()) {
    bool changed = updateInsideObjects(w);

    objText = insideObjectText();

    if (changed)
      invalidateOverlay();
  }

  //---

  if (first) {
    if (objText != "") {
      view()->setStatusText(objText);

      return true;
    }
  }

  //---

  return false;
}

bool
CQChartsPlot::
selectMouseRelease(const QPointF &p)
{
  if (! isReady()) return false;

  auto w = pixelToWindow(CQChartsGeom::Point(p));

  return selectRelease(w);
}

bool
CQChartsPlot::
selectRelease(const CQChartsGeom::Point &w)
{
  // release pressed annotations
  for (const auto &annotation : pressAnnotations_) {
    annotation->mouseRelease(w);
  }

  return true;
}

#if 0
void
CQChartsPlot::
selectObjsAtPoint(const CQChartsGeom::Point &w, Objs &objs)
{
  if (key() && key()->contains(w))
    objs.push_back(key());

  if (title() && title()->contains(w))
    objs.push_back(title());

  Annotations annotations;

  annotationsAtPoint(w, annotations);

  for (const auto &annotation : annotations)
    objs.push_back(annotation);
}
#endif

//------

bool
CQChartsPlot::
editMousePress(const QPointF &pos, bool inside)
{
  if (! isReady()) return false;

  auto p = CQChartsGeom::Point(pos);
  auto w = pixelToWindow(p);

  editing_ = true;

  return editPress(p, w, inside);
}

bool
CQChartsPlot::
editPress(const CQChartsGeom::Point &p, const CQChartsGeom::Point &w, bool inside)
{
  if (isOverlay() && ! isFirstPlot())
    return false;

  //---

  mouseData_.dragObj    = DragObj::NONE;
  mouseData_.pressPoint = p.qpoint();
  mouseData_.movePoint  = mouseData_.pressPoint;
  mouseData_.dragged    = false;

  //---

  // start drag on already selected plot handle
  if (isSelected()) {
    auto v = windowToView(w);

    // to edit must be in handle
    mouseData_.dragSide = editHandles_->inside(v);

    if (mouseData_.dragSide != CQChartsResizeSide::NONE) {
      mouseData_.dragObj = DragObj::PLOT_HANDLE;

      editHandles_->setDragSide(mouseData_.dragSide);
      editHandles_->setDragPos (w);

      invalidateOverlay();

      return true;
    }
  }

  if (keyEditPress(key(), w))
    return true;

  if (axisEditPress(xAxis(), w))
    return true;

  if (axisEditPress(yAxis(), w))
    return true;

  if (annotationsEditPress(w))
    return true;

  //---

  if (keyEditSelect(key(), w))
    return true;

  if (axisEditSelect(xAxis(), w))
    return true;

  if (axisEditSelect(yAxis(), w))
    return true;

  if (titleEditSelect(title(), w))
    return true;

  if (annotationsEditSelect(w))
    return true;

  if (objectsEditSelect(w, inside))
    return true;

  return false;
}

bool
CQChartsPlot::
keyEditPress(CQChartsPlotKey *key, const CQChartsGeom::Point &w)
{
  // start drag on already selected key handle
  if (key && key->isSelected()) {
    mouseData_.dragSide = key->editHandles()->inside(w);

    if (mouseData_.dragSide != CQChartsResizeSide::NONE) {
      mouseData_.dragObj = DragObj::KEY;

      key->editPress(w);

      key->editHandles()->setDragSide(mouseData_.dragSide);
      key->editHandles()->setDragPos (w);

      invalidateOverlay();

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
axisEditPress(CQChartsAxis *axis, const CQChartsGeom::Point &w)
{
  // start drag on already selected axis handle
  if (axis && axis->isSelected()) {
    mouseData_.dragSide = axis->editHandles()->inside(w);

    if (mouseData_.dragSide != CQChartsResizeSide::NONE) {
      mouseData_.dragObj = (axis == xAxis() ? DragObj::XAXIS : DragObj::YAXIS);

      axis->editPress(w);

      axis->editHandles()->setDragSide(mouseData_.dragSide);
      axis->editHandles()->setDragPos (w);

      invalidateOverlay();

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
titleEditPress(CQChartsTitle *title, const CQChartsGeom::Point &w)
{
  // start drag on already selected title handle
  if (title && title->isSelected()) {
    mouseData_.dragSide = title->editHandles()->inside(w);

    if (mouseData_.dragSide != CQChartsResizeSide::NONE) {
      mouseData_.dragObj = DragObj::TITLE;

      title->editPress(w);

      title->editHandles()->setDragSide(mouseData_.dragSide);
      title->editHandles()->setDragPos (w);

      invalidateOverlay();

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
annotationsEditPress(const CQChartsGeom::Point &w)
{
  // start drag on already selected annotation handle
  for (const auto &annotation : annotations()) {
    if (annotation->isSelected()) {
      mouseData_.dragSide = annotation->editHandles()->inside(w);

      if (mouseData_.dragSide != CQChartsResizeSide::NONE) {
        mouseData_.dragObj = DragObj::ANNOTATION;

        annotation->editHandles()->setDragSide(mouseData_.dragSide);
        annotation->editHandles()->setDragPos (w);

        invalidateOverlay();

        return true;
      }
    }
  }

  return false;
}

bool
CQChartsPlot::
keyEditSelect(CQChartsPlotKey *key, const CQChartsGeom::Point &w)
{
  // select/deselect key
  if (key && key->contains(w)) {
    if (! key->isSelected()) {
      selectOneObj(key, /*allObjs*/false);

      return true;
    }

    if (key->editPress(w)) {
      mouseData_.dragObj = DragObj::KEY;

      invalidateOverlay();

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
axisEditSelect(CQChartsAxis *axis, const CQChartsGeom::Point &w)
{
  // select/deselect x axis
  if (axis && axis->contains(w)) {
    if (! axis->isSelected()) {
      selectOneObj(axis, /*allObjs*/false);

      return true;
    }

    if (axis->editPress(w)) {
      mouseData_.dragObj = (axis == xAxis() ? DragObj::XAXIS : DragObj::YAXIS);

      invalidateOverlay();

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
titleEditSelect(CQChartsTitle *title, const CQChartsGeom::Point &w)
{
  // select/deselect title
  if (title && title->contains(w)) {
    if (! title->isSelected()) {
      selectOneObj(title, /*allObjs*/false);

      return true;
    }

    if (title->editPress(w)) {
      mouseData_.dragObj = DragObj::TITLE;

      invalidateOverlay();

      return true;
    }
  }

  return false;
}

bool
CQChartsPlot::
annotationsEditSelect(const CQChartsGeom::Point &w)
{
  Annotations annotations;

  annotationsAtPoint(w, annotations);

  for (const auto &annotation : annotations) {
    if (! annotation->isSelected()) {
      selectOneObj(annotation, /*allObjs*/false);

      return true;
    }

    if (annotation->editPress(w)) {
      mouseData_.dragObj = DragObj::ANNOTATION;

      invalidateOverlay();

      return true;
    }

    return false;
  }

  return false;
}

bool
CQChartsPlot::
objectsEditSelect(const CQChartsGeom::Point &w, bool inside)
{
  auto selectPlot = [&]() {
    startSelection();

    view()->deselectAll();

    setSelected(true);

    endSelection();

    //---

    view()->setCurrentPlot(this);

    invalidateOverlay();
  };

  // select/deselect plot
  // (to select point must be inside a plot object)
  Objs objs;

  objsAtPoint(w, objs);

  if (! objs.empty()) {
    if (! isSelected()) {
      selectPlot();

      return true;
    }

    mouseData_.dragObj = DragObj::PLOT;

    invalidateOverlay();

    return true;
  }

  if (inside) {
    if (contains(w)) {
      if (! isSelected()) {
        selectPlot();

        return true;
      }
    }
  }

  //---

  //view()->deselectAll();

  return false;
}

void
CQChartsPlot::
selectOneObj(CQChartsObj *obj, bool allObjs)
{
  startSelection();

  if (allObjs)
    deselectAllObjs();

  view()->deselectAll();

  obj->setSelected(true);

  endSelection();

  invalidateOverlay();
}

void
CQChartsPlot::
deselectAllObjs()
{
  startSelection();

  //---

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      for (auto &plotObj : plot->plotObjects())
        plotObj->setSelected(false);
    });
  }
  else {
    for (auto &plotObj : plotObjects())
      plotObj->setSelected(false);
  }

  //---

  endSelection();
}

void
CQChartsPlot::
deselectAll()
{
  bool changed = false;

  //---

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->deselectAll1(changed);
    });
  }
  else {
    deselectAll1(changed);
  }

  //---

  if (changed) {
    endSelection();

    invalidateOverlay();
  }
}

void
CQChartsPlot::
deselectAll1(bool &changed)
{
  auto updateChanged = [&] {
    if (! changed) { startSelection(); changed = true; }
  };

  auto deselectObjs = [&](std::initializer_list<CQChartsObj *> objs) {
    for (const auto &obj : objs) {
      if (obj && obj->isSelected()) {
        obj->setSelected(false);

        updateChanged();
      }
    }
  };

  deselectObjs({key(), xAxis(), yAxis(), title()});

  //---

  for (auto &annotation : annotations()) {
    if (annotation->isSelected()) {
      annotation->setSelected(false);

      updateChanged();
    }
  }

  for (auto &plotObj : plotObjects()) {
    if (plotObj->isSelected()) {
      plotObj->setSelected(false);

      updateChanged();
    }
  }

  if (isSelected()) {
    setSelected(false);

    updateChanged();
  }
}

//------

bool
CQChartsPlot::
hasXAxis() const
{
  return type()->hasXAxis();
}

bool
CQChartsPlot::
hasYAxis() const
{
  return type()->hasYAxis();
}

//------

bool
CQChartsPlot::
editMouseMove(const QPointF &pos, bool first)
{
  if (! isReady()) return false;

  auto p = CQChartsGeom::Point(pos);
  auto w = pixelToWindow(p);

  return editMove(p, w, first);
}

bool
CQChartsPlot::
editMove(const CQChartsGeom::Point &p, const CQChartsGeom::Point &w, bool /*first*/)
{
  if (isOverlay() && ! isFirstPlot())
    return false;

  //---

  QPointF lastMovePoint = mouseData_.movePoint;

  mouseData_.movePoint = p.qpoint();

  if (mouseData_.dragObj == DragObj::NONE)
    return false;

  if      (mouseData_.dragObj == DragObj::KEY) {
    if (key()->editMove(w))
      mouseData_.dragged = true;
  }
  else if (mouseData_.dragObj == DragObj::XAXIS) {
    if (xAxis()->editMove(w))
      mouseData_.dragged = true;
  }
  else if (mouseData_.dragObj == DragObj::YAXIS) {
    if (yAxis()->editMove(w))
      mouseData_.dragged = true;
  }
  else if (mouseData_.dragObj == DragObj::TITLE) {
    if (title()->editMove(w))
      mouseData_.dragged = true;
  }
  else if (mouseData_.dragObj == DragObj::ANNOTATION) {
    bool edited = false;

    for (const auto &annotation : annotations()) {
      if (annotation->isSelected()) {
        if (annotation->editMove(w))
          mouseData_.dragged = true;

        edited = true;
      }
    }

    if (! edited)
      return false;
  }
  else if (mouseData_.dragObj == DragObj::PLOT) {
    double dx = mouseData_.movePoint.x() - lastMovePoint.x();
    double dy = lastMovePoint.y() - mouseData_.movePoint.y();

    double dx1 =  view()->pixelToSignedWindowWidth (dx);
    double dy1 = -view()->pixelToSignedWindowHeight(dy);

    if (isOverlay()) {
      processOverlayPlots([&](CQChartsPlot *plot) {
        plot->viewBBox_.moveBy(CQChartsGeom::Point(dx1, dy1));

        if (mouseData_.dragSide == CQChartsResizeSide::MOVE)
          plot->updateMargins(false);
        else
          plot->updateMargins();

        plot->invalidateLayer(CQChartsBuffer::Type::BACKGROUND);
        plot->invalidateLayer(CQChartsBuffer::Type::MIDDLE);
        plot->invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

        plot->invalidateOverlay();
      });
    }
    else {
      viewBBox_.moveBy(CQChartsGeom::Point(dx1, dy1));

      if (mouseData_.dragSide == CQChartsResizeSide::MOVE)
        updateMargins(false);
      else
        updateMargins();

      invalidateLayer(CQChartsBuffer::Type::BACKGROUND);
      invalidateLayer(CQChartsBuffer::Type::MIDDLE);
      invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

      invalidateOverlay();
    }

    if (dx != 0.0 || dy != 0.0)
      mouseData_.dragged = true;
  }
  else if (mouseData_.dragObj == DragObj::PLOT_HANDLE) {
    double dx = mouseData_.movePoint.x() - lastMovePoint.x();
    double dy = lastMovePoint.y() - mouseData_.movePoint.y();

    double dx1 =  view()->pixelToSignedWindowWidth (dx);
    double dy1 = -view()->pixelToSignedWindowHeight(dy);

    if (isOverlay()) {
      processOverlayPlots([&](CQChartsPlot *plot) {
        editHandles_->updateBBox(dx1, dy1);

        plot->viewBBox_ = editHandles_->bbox();

        if (mouseData_.dragSide == CQChartsResizeSide::MOVE)
          plot->updateMargins(false);
        else
          plot->updateMargins();

        plot->invalidateLayer(CQChartsBuffer::Type::BACKGROUND);
        plot->invalidateLayer(CQChartsBuffer::Type::MIDDLE);
        plot->invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

        plot->invalidateOverlay();
      });
    }
    else {
      editHandles_->updateBBox(dx1, dy1);

      viewBBox_ = editHandles_->bbox();

      if (mouseData_.dragSide == CQChartsResizeSide::MOVE)
        updateMargins(false);
      else
        updateMargins();

      invalidateLayer(CQChartsBuffer::Type::BACKGROUND);
      invalidateLayer(CQChartsBuffer::Type::MIDDLE);
      invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

      invalidateOverlay();
    }

    if (dx != 0.0 || dy != 0.0)
      mouseData_.dragged = true;

    view()->update();
  }
  else {
    return false;
  }

  invalidateOverlay();

  return true;
}

bool
CQChartsPlot::
editMouseMotion(const QPointF &pos)
{
  if (! isReady()) return false;

  auto p = CQChartsGeom::Point(pos);
  auto w = pixelToWindow(p);

  return editMotion(p, w);
}

bool
CQChartsPlot::
editMotion(const CQChartsGeom::Point &, const CQChartsGeom::Point &w)
{
  if (isOverlay() && ! isFirstPlot())
    return false;

  //---

  if      (isSelected()) {
    auto v = windowToView(w);

    if (! editHandles_->selectInside(v))
      return true;
  }
  else if (key() && key()->isSelected()) {
    if (! key()->editMotion(w))
      return true;
  }
  else if (xAxis() && xAxis()->isSelected()) {
    if (! xAxis()->editMotion(w))
      return true;
  }
  else if (yAxis() && yAxis()->isSelected()) {
    if (! yAxis()->editMotion(w))
      return true;
  }
  else if (title() && title()->isSelected()) {
    if (! title()->editMotion(w))
      return true;
  }
  else {
    bool edited = false;

    for (const auto &annotation : annotations()) {
      if (annotation->isSelected()) {
        if (annotation->editMotion(w)) {
          edited = true;
          break;
        }
      }
    }

    if (! edited)
      return true;
  }

  invalidateOverlay();

  return true;
}

bool
CQChartsPlot::
editMouseRelease(const QPointF &pos)
{
  if (! isReady()) return false;

  auto p = CQChartsGeom::Point(pos);
  auto w = pixelToWindow(p);

  editing_ = false;

  return editRelease(p, w);
}

bool
CQChartsPlot::
editRelease(const CQChartsGeom::Point & /*p*/, const CQChartsGeom::Point & /*w*/)
{
  if (isOverlay() && ! isFirstPlot())
    return false;

  //---

  mouseData_.dragObj = DragObj::NONE;

  if (mouseData_.dragged)
    drawObjs();

  return true;
}

void
CQChartsPlot::
editMoveBy(const CQChartsGeom::Point &d)
{
  if (isOverlay() && ! isFirstPlot())
    return;

  //---

  auto r = calcDataRect();

  double dw = d.x*r.getWidth ();
  double dh = d.y*r.getHeight();

  CQChartsGeom::Point dp(dw, dh);

  if      (isSelected()) {
  }
  else if (key() && key()->isSelected()) {
    key()->editMoveBy(dp);
  }
  else if (xAxis() && xAxis()->isSelected()) {
    xAxis()->editMoveBy(dp);
  }
  else if (yAxis() && yAxis()->isSelected()) {
    yAxis()->editMoveBy(dp);
  }
  else if (title() && title()->isSelected()) {
    title()->editMoveBy(dp);
  }
  else {
    for (const auto &annotation : annotations()) {
      if (annotation->isSelected()) {
        (void) annotation->editMoveBy(dp);
        break;
      }
    }
  }

  invalidateOverlay();
}

//------

void
CQChartsPlot::
editObjs(Objs &objs)
{
  if (isKeyVisibleAndNonEmpty())
    objs.push_back(key());

  if (title() && title()->isDrawn())
    objs.push_back(title());

  if (xAxis() && xAxis()->isVisible())
    objs.push_back(xAxis());

  if (yAxis() && yAxis()->isVisible())
    objs.push_back(yAxis());
}

bool
CQChartsPlot::
rectSelect(const CQChartsGeom::BBox &r, SelMod selMod)
{
  // for replace init all objects to unselected
  // for add/remove/toggle init all objects to current state
  using ObjsSelected = std::map<CQChartsObj*,bool>;

  ObjsSelected objsSelected;

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      for (auto &plotObj : plot->plotObjects()) {
        if (selMod == SelMod::REPLACE)
          objsSelected[plotObj] = false;
        else
          objsSelected[plotObj] = plotObj->isSelected();
      }
    });
  }
  else {
    for (auto &plotObj : plotObjects()) {
      if (selMod == SelMod::REPLACE)
        objsSelected[plotObj] = false;
      else
        objsSelected[plotObj] = plotObj->isSelected();
    }
  }

  //---

  // get objects inside/touching rectangle
  Objs objs;

  objsIntersectRect(r, objs, view()->isSelectInside());

  //---

  // change selection depending on selection modifier
  for (auto &obj : objs) {
    if      (selMod == SelMod::TOGGLE)
      objsSelected[obj] = ! obj->isSelected();
    else if (selMod == SelMod::REPLACE)
      objsSelected[obj] = true;
    else if (selMod == SelMod::ADD)
      objsSelected[obj] = true;
    else if (selMod == SelMod::REMOVE)
      objsSelected[obj] = false;
  }

  //---

  // select objects and track if selection changed
  bool changed = false;

  auto setObjSelected = [&](CQChartsObj *obj, bool selected) {
    if (! changed) { startSelection(); changed = true; }

    obj->setSelected(selected);
  };

  for (const auto &objSelected : objsSelected) {
    if (objSelected.first->isSelected() == objSelected.second)
      continue;

    if (! objSelected.second)
      setObjSelected(objSelected.first, objSelected.second);
  }

  for (const auto &objSelected : objsSelected) {
    if (objSelected.first->isSelected() == objSelected.second)
      continue;

    if (objSelected.second)
      setObjSelected(objSelected.first, objSelected.second);
  }

  //----

  // update selection if changed
  if (changed) {
    beginSelectIndex();

    for (const auto &objSelected : objsSelected) {
      auto *selectPlotObj = dynamic_cast<CQChartsPlotObj *>(objSelected.first);

      if (! selectPlotObj || ! selectPlotObj->isSelected())
        continue;

      selectPlotObj->addSelectIndices();
    }

    endSelectIndex();

    //---

    invalidateOverlay();

    if (selectInvalidateObjs())
      drawObjs();

    //---

    endSelection();
  }

  //---

  return ! objs.empty();
}

void
CQChartsPlot::
startSelection()
{
  view()->startSelection();
}

void
CQChartsPlot::
endSelection()
{
  view()->endSelection();

  emit selectionChanged();
}

void
CQChartsPlot::
selectedObjs(Objs &objs) const
{
  PlotObjs plotObjs;

  selectedPlotObjs(plotObjs);

  for (auto &plotObj : plotObjs)
    objs.push_back(plotObj);

  for (auto &annotation : annotations()) {
    if (annotation->isSelected())
      objs.push_back(annotation);
  }

  if (key() && key()->isSelected())
    objs.push_back(key());

  if (title() && title()->isSelected())
    objs.push_back(title());

  if (xAxis() && xAxis()->isSelected())
    objs.push_back(xAxis());

  if (yAxis() && yAxis()->isSelected())
    objs.push_back(yAxis());
}

void
CQChartsPlot::
selectedPlotObjs(PlotObjs &plotObjs) const
{
  for (auto &plotObj : plotObjects()) {
    if (plotObj->isSelected())
      plotObjs.push_back(plotObj);
  }
}

void
CQChartsPlot::
setPlotObjTreeSet(bool b)
{
  if (b != objTreeData_.isSet) {
    objTreeData_.isSet = b;

    if (b)
      objTreeData_.notify = true;
  }
}

//------

void
CQChartsPlot::
setXValueColumn(const CQChartsColumn &c)
{
  if (mappedXAxis()) {
    // calls drawBackground and drawForeground
    CQChartsUtil::testAndSet(xValueColumn_, c, [&]() { mappedXAxis()->setColumn(xValueColumn_); } );
  }
}

void
CQChartsPlot::
setYValueColumn(const CQChartsColumn &c)
{
  if (mappedYAxis()) {
    // calls drawBackground and drawForeground
    CQChartsUtil::testAndSet(yValueColumn_, c, [&]() { mappedYAxis()->setColumn(yValueColumn_); } );
  }
}

//------

void
CQChartsPlot::
setIdColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(idColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsPlot::
setTipColumns(const CQChartsColumns &c)
{
  CQChartsUtil::testAndSet(tipColumns_, c, [&]() { resetObjTips(); } );
}

void
CQChartsPlot::
setVisibleColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(visibleColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsPlot::
setImageColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(imageColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsPlot::
setColorColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(colorColumnData_.column, c, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorType(const ColorType &t)
{
  CQChartsUtil::testAndSet(colorColumnData_.colorType, t, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorMapped(bool b)
{
  CQChartsUtil::testAndSet(colorColumnData_.mapped, b, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorMapMin(double r)
{
  CQChartsUtil::testAndSet(colorColumnData_.map_min, r, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorMapMax(double r)
{
  CQChartsUtil::testAndSet(colorColumnData_.map_max, r, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorMapPalette(const QString &s)
{
  CQChartsUtil::testAndSet(colorColumnData_.palette, s, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorXStops(const CQChartsColorStops &s)
{
  CQChartsUtil::testAndSet(colorColumnData_.xStops, s, [&]() { updateObjs(); } );
}

void
CQChartsPlot::
setColorYStops(const CQChartsColorStops &s)
{
  CQChartsUtil::testAndSet(colorColumnData_.yStops, s, [&]() { updateObjs(); } );
}

//------

void
CQChartsPlot::
initColorColumnData()
{
  CQPerfTrace trace("CQChartsPlot::initColorColumnData");

  std::unique_lock<std::mutex> lock(colorMutex_);

  //---

  colorColumnData_.valid = false;

  if (! colorColumn().isValid())
    return;

  //---

  CQChartsModelColumnDetails *columnDetails = this->columnDetails(colorColumn());
  if (! columnDetails) return;

  if (colorColumn().isGroup()) {
    colorColumnData_.data_min = 0.0;
    colorColumnData_.data_max = std::max(numGroups() - 1, 0);
  }
  else {
    if (colorColumnData_.mapped) {
      QVariant minVar = columnDetails->minValue();
      QVariant maxVar = columnDetails->maxValue();

      bool ok;

      colorColumnData_.data_min = CQChartsVariant::toReal(minVar, ok);
      if (! ok) colorColumnData_.data_min = 0.0;

      colorColumnData_.data_max = CQChartsVariant::toReal(maxVar, ok);
      if (! ok) colorColumnData_.data_max = 1.0;
    }

    CQBaseModelType    columnType;
    CQBaseModelType    columnBaseType;
    CQChartsNameValues nameValues;

    (void) CQChartsModelUtil::columnValueType(charts(), model().data(), colorColumn(),
                                              columnType, columnBaseType, nameValues);

    if (columnType == CQBaseModelType::COLOR) {
      CQChartsColumnTypeMgr *columnTypeMgr = charts()->columnTypeMgr();

      const CQChartsColumnColorType *colorType =
        dynamic_cast<const CQChartsColumnColorType *>(columnTypeMgr->getType(columnType));
      assert(colorType);

      colorType->getMapData(charts(), model().data(), colorColumn(), nameValues,
                            colorColumnData_.mapped,
                            colorColumnData_.data_min, colorColumnData_.data_max,
                            colorColumnData_.palette);
    }

    colorColumnData_.modelType = columnType;
  }

  colorColumnData_.valid = true;
}

bool
CQChartsPlot::
columnColor(int row, const QModelIndex &parent, CQChartsColor &color) const
{
  CQChartsModelIndex colorInd(row, colorColumn(), parent);

  return columnColor(colorInd, color);
}

bool
CQChartsPlot::
columnColor(const CQChartsModelIndex &colorInd, CQChartsColor &color) const
{
  if (! colorColumnData_.valid)
    return false;

  // get mode edit value
  bool ok;

  QVariant var = modelValue(colorInd, ok);
  if (! ok || ! var.isValid()) return false;

  return columnValueColor(var, color);
}

bool
CQChartsPlot::
columnValueColor(const QVariant &var, CQChartsColor &color) const
{
  if (colorColumnData_.mapped) {
    if (CQChartsVariant::isNumeric(var)) {
      bool ok;

      double r = CQChartsVariant::toReal(var, ok);
      if (! ok) return false;

      double r1 =
        CMathUtil::map(r, colorColumnData_.data_min, colorColumnData_.data_max, 0.0, 1.0);

      if (r1 < 0.0 || r1 > 1.0)
        return false;

      if (colorColumnData_.palette != "") {
        auto *palette = CQColorsMgrInst->getNamedPalette(colorColumnData_.palette);

        if (palette)
          color = palette->getColor(r1);
      }
      else
        color = CQChartsColor(CQChartsColor::Type::PALETTE_VALUE, r1);
    }
    else {
      if (CQChartsVariant::isColor(var)) {
        bool ok;
        color = CQChartsVariant::toColor(var, ok);
      }
      else {
        CQChartsModelColumnDetails *columnDetails = this->columnDetails(colorColumn());
        if (! columnDetails) return false;

        // use unique index/count of edit values (which may have been converted)
        // not same as CQChartsColumnColorType::userData
        int n = columnDetails->numUnique();
        int i = columnDetails->valueInd(var);

        double r = (n > 1 ? double(i)/(n - 1) : 0.0);

        if (colorColumnData_.palette != "") {
          auto *palette = CQColorsMgrInst->getNamedPalette(colorColumnData_.palette);

          if (palette)
            color = palette->getColor(r);
        }
        else
          color = CQChartsColor(CQChartsColor::Type::PALETTE_VALUE, r);
      }
    }
  }
  else {
    if (CQChartsVariant::isColor(var)) {
      bool ok;
      color = CQChartsVariant::toColor(var, ok);
    }
    else {
      color = CQChartsColor(var.toString());
    }
  }

  return color.isValid();
}

//------

QString
CQChartsPlot::
keyText() const
{
  QString text;

  text = this->titleStr();

  if (! text.length())
    text = this->id();

  return text;
}

//------

QString
CQChartsPlot::
posStr(const CQChartsGeom::Point &w) const
{
  return xStr(w.x) + " " + yStr(w.y);
}

QString
CQChartsPlot::
xStr(double x) const
{
  if (isLogX())
    x = expValue(x);

  if (xValueColumn().isValid())
    return columnStr(xValueColumn(), x);
  else
    return CQChartsUtil::formatReal(x);
}

QString
CQChartsPlot::
yStr(double y) const
{
  if (isLogY())
    y = expValue(y);

  if (yValueColumn().isValid())
    return columnStr(yValueColumn(), y);
  else
    return CQChartsUtil::formatReal(y);
}

QString
CQChartsPlot::
columnStr(const CQChartsColumn &column, double x) const
{
  if (! column.isValid())
    return CQChartsUtil::formatReal(x);

  QAbstractItemModel *model = this->model().data();

  if (! model)
    return CQChartsUtil::formatReal(x);

  QString str;

  if (! CQChartsModelUtil::formatColumnValue(charts(), model, column, x, str))
    return CQChartsUtil::formatReal(x);

  return str;
}

void
CQChartsPlot::
keyPress(int key, int modifier)
{
  bool is_shift = (modifier & Qt::ShiftModifier  );
//bool is_ctrl  = (modifier & Qt::ControlModifier);

  if      (key == Qt::Key_Left || key == Qt::Key_Right ||
           key == Qt::Key_Up   || key == Qt::Key_Down) {
    if (view()->mode() != CQChartsView::Mode::EDIT) {
      if      (key == Qt::Key_Right)
        panLeft(getPanX(is_shift));
      else if (key == Qt::Key_Left)
        panRight(getPanX(is_shift));
      else if (key == Qt::Key_Up)
        panDown(getPanY(is_shift));
      else if (key == Qt::Key_Down)
        panUp(getPanY(is_shift));
    }
    else {
      if      (key == Qt::Key_Right)
        editMoveBy(CQChartsGeom::Point( getMoveX(is_shift), 0));
      else if (key == Qt::Key_Left)
        editMoveBy(CQChartsGeom::Point(-getMoveX(is_shift), 0));
      else if (key == Qt::Key_Up)
        editMoveBy(CQChartsGeom::Point(0, getMoveY(is_shift)));
      else if (key == Qt::Key_Down)
        editMoveBy(CQChartsGeom::Point(0, -getMoveY(is_shift)));
    }
  }
  else if (key == Qt::Key_Plus || key == Qt::Key_Minus) {
    if (key == Qt::Key_Plus)
      zoomIn(getZoomFactor(is_shift));
    else
      zoomOut(getZoomFactor(is_shift));
  }
  else if (key == Qt::Key_Home) {
    zoomFull();
  }
  else if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
    if (key == Qt::Key_Tab)
      cycleNext();
    else
      cyclePrev();
  }
  else if (key == Qt::Key_F1) {
    if (! is_shift)
      cycleNext();
    else
      cyclePrev();
  }
  else
    return;
}

double
CQChartsPlot::
getPanX(bool is_shift) const
{
  // get pan x in view coords
  if (mappedXAxis()) {
    return windowToViewWidth(! is_shift ? mappedXAxis()->minorTickIncrement() :
                                          mappedXAxis()->majorTickIncrement());
  }
  else {
    return (! is_shift ? 0.125 : 0.25);
  }
}

double
CQChartsPlot::
getPanY(bool is_shift) const
{
  // get pan y in view coords
  if (mappedYAxis()) {
    return windowToViewHeight(! is_shift ? mappedYAxis()->minorTickIncrement() :
                                           mappedYAxis()->majorTickIncrement());
  }
  else {
    return (! is_shift ? 0.125 : 0.25);
  }
}

double
CQChartsPlot::
getMoveX(bool is_shift) const
{
  return (! is_shift ? 0.025 : 0.05);
}

double
CQChartsPlot::
getMoveY(bool is_shift) const
{
  return (! is_shift ? 0.025 : 0.05);
}

double
CQChartsPlot::
getZoomFactor(bool is_shift) const
{
  return (! is_shift ? 1.5 : 2.0);
}

void
CQChartsPlot::
updateSlot()
{
  drawObjs();
}

void
CQChartsPlot::
cycleNext()
{
  cycleNextPrev(/*prev*/false);
}

void
CQChartsPlot::
cyclePrev()
{
  cycleNextPrev(/*prev*/true);
}

void
CQChartsPlot::
cycleNextPrev(bool prev)
{
  if (! insideObjs_.empty()) {
    if (! prev)
      nextInsideInd();
    else
      prevInsideInd();

    setInsideObject();

    QString objText = insideObjectText();

    view()->setStatusText(objText);

    invalidateOverlay();
  }
}

void
CQChartsPlot::
panLeft(double f)
{
  if (! allowPanX())
    return;

  double dx = viewToWindowWidth(f)/getDataRange().getWidth();

  auto panX = [&](CQChartsPlot *plot) {
    plot->setDataOffsetX(plot->dataOffsetX() - dx);

    plot->adjustPan();

    plot->applyDataRangeAndDraw();
  };

  panX(this);

#if 0
  if (! isOverlay() && isY1Y2()) {
    CQChartsPlot *plot1, *plot2;

    y1y2Plots(plot1, plot2);

    if (this == plot1) {
      panX(plot2);
    }
    else
      panX(plot1);
  }
#endif

  emit zoomPanChanged();
}

void
CQChartsPlot::
panRight(double f)
{
  if (! allowPanX())
    return;

  double dx = viewToWindowWidth(f)/getDataRange().getWidth();

  auto panX = [&](CQChartsPlot *plot) {
    plot->setDataOffsetX(plot->dataOffsetX() + dx);

    plot->adjustPan();

    plot->applyDataRangeAndDraw();
  };

  panX(this);

/*
  if (! isOverlay() && isY1Y2()) {
    CQChartsPlot *plot1, *plot2;

    y1y2Plots(plot1, plot2);

    if (this == plot1)
      panX(plot2);
    else
      panX(plot1);
  }
*/

  emit zoomPanChanged();
}

void
CQChartsPlot::
panUp(double f)
{
  if (! allowPanY())
    return;

  double dy = viewToWindowHeight(f)/getDataRange().getHeight();

  auto panY = [&](CQChartsPlot *plot) {
    plot->setDataOffsetY(plot->dataOffsetY() + dy);

    plot->adjustPan();

    plot->applyDataRangeAndDraw();
  };

  panY(this);

/*
  if (! isOverlay() && isX1X2()) {
    CQChartsPlot *plot1, *plot2;

    x1x2Plots(plot1, plot2);

    if (this == plot1)
      panY(plot2);
    else
      panY(plot1);
  }
*/

  emit zoomPanChanged();
}

void
CQChartsPlot::
panDown(double f)
{
  if (! allowPanY())
    return;

  double dy = viewToWindowHeight(f)/getDataRange().getHeight();

  auto panY = [&](CQChartsPlot *plot) {
    plot->setDataOffsetY(plot->dataOffsetY() - dy);

    plot->adjustPan();

    plot->applyDataRangeAndDraw();
  };

  panY(this);

#if 0
  if (! isOverlay() && isX1X2()) {
    CQChartsPlot *plot1, *plot2;

    x1x2Plots(plot1, plot2);

    if (this == plot1)
      panY(plot2);
    else
      panY(plot1);
  }
#endif

  emit zoomPanChanged();
}

void
CQChartsPlot::
pan(double dx, double dy)
{
  if (allowPanX())
    setDataOffsetX(dataOffsetX() + dx/getDataRange().getWidth());

  if (allowPanY())
    setDataOffsetY(dataOffsetY() + dy/getDataRange().getHeight());

  adjustPan();

  applyDataRangeAndDraw();

  emit zoomPanChanged();
}

//---

void
CQChartsPlot::
zoomIn(double f)
{
  if (allowZoomX())
    setDataScaleX(dataScaleX()*f);

  if (allowZoomY())
    setDataScaleY(dataScaleY()*f);

  applyDataRangeAndDraw();

  emit zoomPanChanged();
}

void
CQChartsPlot::
zoomOut(double f)
{
  if (allowZoomX())
    setDataScaleX(dataScaleX()/f);

  if (allowZoomY())
    setDataScaleY(dataScaleY()/f);

  applyDataRangeAndDraw();

  emit zoomPanChanged();
}

void
CQChartsPlot::
zoomTo(const CQChartsGeom::BBox &bbox)
{
  auto bbox1 = bbox;

  double w = bbox1.getWidth ();
  double h = bbox1.getHeight();

  if (w < 1E-50 || h < 1E-50) {
    double dataScale = 2*std::min(dataScaleX(), dataScaleY());

    w = dataRange_.xsize()/dataScale;
    h = dataRange_.ysize()/dataScale;
  }

  if (! dataRange_.isSet())
    return;

  auto c = bbox.getCenter();

  double w1 = dataRange_.xsize();
  double h1 = dataRange_.ysize();

  double xscale = w1/w;
  double yscale = h1/h;

  //setDataScaleX(std::min(xscale, yscale));
  //setDataScaleY(std::min(xscale, yscale));

  if (allowZoomX())
    setDataScaleX(xscale);

  if (allowZoomY())
    setDataScaleY(yscale);

  auto c1 = CQChartsGeom::Point(dataRange_.xmid(), dataRange_.ymid());

  double cx = (allowPanX() ? c.x - c1.x : 0.0)/getDataRange().getWidth ();
  double cy = (allowPanY() ? c.y - c1.y : 0.0)/getDataRange().getHeight();

  setDataOffsetX(cx);
  setDataOffsetY(cy);

  applyDataRangeAndDraw();

  emit zoomPanChanged();
}

void
CQChartsPlot::
zoomFull(bool notify)
{
  if (allowZoomX())
    setDataScaleX(1.0);

  if (allowZoomY())
    setDataScaleY(1.0);

  setDataOffsetX(0.0);
  setDataOffsetY(0.0);

  applyDataRangeAndDraw();

  if (notify)
    emit zoomPanChanged();
}

void
CQChartsPlot::
updateTransform()
{
  postResize();

  drawObjs();
}

//------

bool
CQChartsPlot::
tipText(const CQChartsGeom::Point &p, QString &tip) const
{
  if (isOverlay() && ! isFirstPlot())
    return false;

  //---

  int objNum  = 0;
  int numObjs = 0;

  CQChartsObj *tipObj = nullptr;

  if (isFollowMouse()) {
    objNum  = insideInd_;
    numObjs = insideObjs_.size();

    tipObj = insideObject();
  }
  else {
    Objs objs;

    objsAtPoint(p, objs);

    numObjs = objs.size();

    if (numObjs)
      tipObj = *objs.begin();
  }

  if (tipObj) {
    if (tip != "")
      tip += " ";

    tip += tipObj->tipId();

    if (numObjs > 1)
      tip += QString("<br><font color=\"blue\">&nbsp;&nbsp;%1 of %2</font>").
               arg(objNum + 1).arg(numObjs);
  }

  return tip.length();
}

void
CQChartsPlot::
addTipColumns(CQChartsTableTip &tableTip, const QModelIndex &ind) const
{
  for (const auto &c : tipColumns().columns()) {
    if (c.isValid()) {
      CQChartsModelIndex tipModelInd(ind.row(), c, ind.parent());

      QModelIndex tipInd  = modelIndex(tipModelInd);
      QModelIndex tipInd1 = unnormalizeIndex(tipInd);

      CQChartsModelIndex tipModelInd1(tipInd1.row(), c, tipInd1.parent());

      bool ok1, ok2;

      QString name  = modelHHeaderString(c, ok1);
      QString value = modelString(tipModelInd1, ok2);

      if (ok1 && ok2)
        tableTip.addTableRow(name, value);
    }
  }
}

void
CQChartsPlot::
resetObjTips()
{
  for (auto &obj : plotObjects())
    obj->resetTipId();
}

//------

void
CQChartsPlot::
objsAtPoint(const CQChartsGeom::Point &p, Objs &objs) const
{
  PlotObjs plotObjs;

  plotObjsAtPoint(p, plotObjs);

  for (const auto &plotObj : plotObjs)
    objs.push_back(plotObj);

  //---

  Annotations annotations;

  annotationsAtPoint(p, annotations);

  for (const auto &annotation : annotations)
    objs.push_back(annotation);
}

void
CQChartsPlot::
plotObjsAtPoint(const CQChartsGeom::Point &p, PlotObjs &plotObjs) const
{
  if (isOverlay()) {
    processOverlayPlots([&](const CQChartsPlot *plot) {
      auto p1 = p;

      if (plot != this)
        p1 = plot->pixelToWindow(windowToPixel(p));

      plot->objTreeData_.tree->objectsAtPoint(p1, plotObjs);
    });
  }
  else {
    objTreeData_.tree->objectsAtPoint(p, plotObjs);
  }
}

void
CQChartsPlot::
annotationsAtPoint(const CQChartsGeom::Point &p, Annotations &annotations) const
{
  annotations.clear();

  for (const auto &annotation : this->annotations()) {
    if (! annotation->contains(p))
      continue;

    annotations.push_back(annotation);
  }
}

void
CQChartsPlot::
objsIntersectRect(const CQChartsGeom::BBox &r, Objs &objs, bool inside) const
{
  if (isOverlay()) {
    processOverlayPlots([&](const CQChartsPlot *plot) {
      auto r1 = r;

      if (plot != this)
        r1 = windowToPixel(plot->pixelToWindow(r));

      PlotObjs plotObjs;

      plot->objTreeData_.tree->objectsIntersectRect(r1, plotObjs, inside);

      for (const auto &plotObj : plotObjs)
        objs.push_back(plotObj);
    });
  }
  else {
    PlotObjs plotObjs;

    objTreeData_.tree->objectsIntersectRect(r, plotObjs, inside);

    for (const auto &plotObj : plotObjs)
      objs.push_back(plotObj);
  }
}

bool
CQChartsPlot::
objNearestPoint(const CQChartsGeom::Point &p, CQChartsPlotObj* &obj) const
{
  obj = nullptr;

  double tx = dataRange_.xsize()/32.0;
  double ty = dataRange_.ysize()/32.0;

  return objTreeData_.tree->objectNearest(p, tx, ty, obj);
}

//---

void
CQChartsPlot::
preResize()
{
  std::unique_lock<std::mutex> lock(resizeMutex_);

  interruptDraw();
}

void
CQChartsPlot::
postResize()
{
  if (isOverlay() && ! isFirstPlot())
    return;

  applyDataRange();

  if (isEqualScale()) {
    resetDataRange(/*updateRange*/true, /*updateObjs*/false);
  }

#if 0
  // TODO: does obj postResize need range set ?
  for (auto &obj : plotObjects())
    obj->postResize();
#endif

  // TODO: does key position need range set ?
  updateKeyPosition(/*force*/true);

  if (isAutoFit())
    needsAutoFit_ = true;

  drawObjs();
}

void
CQChartsPlot::
updateKeyPosition(bool force)
{
  if (! isKeyVisibleAndNonEmpty())
    return;

  if (isOverlay() && ! isFirstPlot())
    return;

  if (force)
    key()->invalidateLayout();

  if (! dataRange_.isSet())
    return;

  auto bbox = calcDataRange();

  if (! bbox.isSet())
    return;

  key()->updateLocation(bbox);
}

//------

bool
CQChartsPlot::
printLayer(CQChartsLayer::Type type, const QString &filename) const
{
  CQChartsLayer *layer = getLayer(type);

  const CQChartsBuffer *buffer = getBuffer(layer->buffer());

  if (! buffer->image())
    return false;

  buffer->image()->save(filename);

  return true;
}

//------

void
CQChartsPlot::
draw(QPainter *painter)
{
  if (! view()->isBufferLayers()) {
    initGroupedPlotObjs();

    //---

    drawParts(painter);

    //---

    updateAutoFit();

    return;
  }

  //---

  bool        drawLayers  { false };
  UpdateState updateState { UpdateState::INVALID };

  {
  LockMutex lock(this, "draw");

  updateState = this->updateState();

  if      (updateState == UpdateState::READY) {
    drawLayers = true;

    setGroupedUpdateState(UpdateState::DRAWN);
  }
  else if (updateState == UpdateState::DRAWN) {
    drawLayers = true;
  }
  else if (updateState == UpdateState::INVALID) {
    drawLayers = true;
  }
  }

  //---

  if (! isSequential()) {
    if (drawLayers)
      this->drawLayers(painter);
    else {
      this->drawLayers(painter);

      this->drawBusy(painter, updateState);
    }
  }
  else {
    this->drawLayers(painter);
  }
}

void
CQChartsPlot::
updateGroupedDraw()
{
  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->updateDraw();
    });
  }
  else {
    updateDraw();
  }
}

void
CQChartsPlot::
updateDraw()
{
  // only draw first plot for overlay plots
  if (isOverlay() && ! isFirstPlot())
    return;

  //---

  // init object tree(s)
  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      if (plot->objTreeData_.init) {
        plot->objTreeData_.init = false;

        plot->initObjTree();
      }
    });
  }
  else {
    if (objTreeData_.init) {
      objTreeData_.init = false;

      initObjTree();
    }
  }

  //---

  if (! isSequential()) {
    // ignore draw until after calc range and objs finished
    {
      LockMutex lock(this, "draw::updateDraw");

      UpdateState updateState = this->updateState();

      if (updateState == UpdateState::CALC_RANGE)
        return;

      if (updateState == UpdateState::CALC_OBJS)
        return;

      interruptDraw();
    }

    //---

    {
    LockMutex lock(this, "draw::updateDraw");

    getBuffer(CQChartsBuffer::Type::BACKGROUND)->setValid(false);
    getBuffer(CQChartsBuffer::Type::MIDDLE    )->setValid(false);
    getBuffer(CQChartsBuffer::Type::FOREGROUND)->setValid(false);

    updateData_.drawBusy.ind = 0;

    setUpdateState(UpdateState::DRAW_OBJS);

    updateData_.drawThread.start(this, debugUpdate_ ? "drawObjs" : nullptr);
    updateData_.drawThread.future = std::async(std::launch::async, drawASync, this);

    startThreadTimer();
    }
  }
  else {
    // draw objs
    drawThread();

    postDraw();

    view()->update();
  }
}

void
CQChartsPlot::
interruptDraw()
{
  setInterrupt(true);

  waitDraw();

  setInterrupt(false);
}

void
CQChartsPlot::
syncDraw()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::syncDraw\n";

  syncState();

  waitRange();
  waitObjs ();
  waitDraw ();
}

void
CQChartsPlot::
waitDraw()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::waitDraw\n";

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->waitDraw1();
    });
  }
  else {
    waitDraw1();
  }
}

void
CQChartsPlot::
waitDraw1()
{
  if (debugUpdate_)
    std::cerr << "CQChartsPlot::waitDraw1\n";

  UpdateState updateState = this->updateState();

  while (updateState == UpdateState::DRAW_OBJS) {
    if (updateData_.drawThread.busy.load()) {
      (void) updateData_.drawThread.future.get();

      assert(! updateData_.drawThread.future.valid());

      updateData_.drawThread.finish(this, debugUpdate_ ? "drawObjs" : nullptr);

      return;
    }

    if (isInterrupt())
      return;

    if (hasLockId())
      return;

    if (! isFirstPlot())
      break;

    threadTimerSlot();

    updateState = this->updateState();
  }
}

void
CQChartsPlot::
drawASync(CQChartsPlot *plot)
{
  plot->drawThread();
}

void
CQChartsPlot::
drawThread()
{
  CQPerfTrace trace("CQChartsPlot::drawThread");

  //---

  view()->lockPainter(true);

  drawParts(view()->ipainter());

  view()->lockPainter(false);

  updateData_.drawThread.end(this, debugUpdate_ ? "drawThread" : nullptr);
}

void
CQChartsPlot::
drawBusy(QPainter *painter, const UpdateState &updateState) const
{
  CQChartsGeom::Point p1 =
    view()->windowToPixel(CQChartsGeom::Point(viewBBox().getXMin(), viewBBox().getYMin()));
  CQChartsGeom::Point p2 =
    view()->windowToPixel(CQChartsGeom::Point(viewBBox().getXMax(), viewBBox().getYMax()));

  //---

  double x = (p1.x + p2.x)/2.0;
  double y = (p1.y + p2.y)/2.0;

  int ind = updateData_.drawBusy.ind/updateData_.drawBusy.multiple;

  double a  = 0.0;
  double da = 2.0*M_PI/updateData_.drawBusy.count;

  double r1 = 32;
  double r2 = 4;
  double r3 = 10;

  QPen   pen(Qt::NoPen);
  QBrush brush(updateData_.drawBusy.fgColor);

  painter->setBrush(brush);
  painter->setPen(pen);

  for (int i = 0; i < updateData_.drawBusy.count; ++i) {
    int i1 = i - ind;

    if (i1 < 0) i1 += updateData_.drawBusy.count;

    double r = i1*(r2 - r3)/(updateData_.drawBusy.count - 1) + r3;

    CQChartsGeom::Point c(x, y);

    auto p1 = CQChartsGeom::circlePoint(c, r1, a);

    CQChartsGeom::BBox bbox(p1.x - r, p1.y - r, p1.x + r, p1.y + r);

    painter->drawEllipse(bbox.qrect());

    a += da;
  }

  ++updateData_.drawBusy.ind;

  if (updateData_.drawBusy.ind >= updateData_.drawBusy.count*updateData_.drawBusy.multiple)
    updateData_.drawBusy.ind = 0;

  QString text;

  if      (updateState == UpdateState::CALC_RANGE)
    text = "Calc Range";
  else if (updateState == UpdateState::CALC_OBJS)
    text = "Calc Objects";
  else if (updateState == UpdateState::DRAW_OBJS)
    text = "Draw Objects";

  if (text.length()) {
    CQChartsColor color(CQChartsColor::Type::INTERFACE_VALUE, 1.0);

    QColor tc = charts()->interpColor(color, 0.0);

    painter->setPen(tc);

    QFont font = view()->viewFont(updateData_.drawBusy.font);

    QFontMetricsF fm(font);

    double tw = fm.width(text);
    double ta = fm.ascent();

    double tx = x - tw/2.0;
    double ty = y + r1 + r3 + 4 + ta;

    painter->drawText(int(tx), int(ty), text);
  }
}

void
CQChartsPlot::
drawLayers(QPainter *painter) const
{
  for (auto &tb : buffers_) {
    CQChartsBuffer *buffer = tb.second;

    if (buffer->isActive() && buffer->isValid())
      buffer->draw(painter);
  }
}

void
CQChartsPlot::
drawLayer(QPainter *painter, CQChartsLayer::Type type) const
{
  CQChartsLayer *layer = getLayer(type);

  CQChartsBuffer *buffer = getBuffer(layer->buffer());

  buffer->draw(painter, 0, 0);
}

void
CQChartsPlot::
drawParts(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawParts");

  //---

  drawBackgroundParts(painter);

  //---

  drawMiddleParts(painter);

  //---

  drawForegroundParts(painter);

  //---

  drawOverlayParts(painter);
}

void
CQChartsPlot::
drawBackgroundParts(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawBackgroundParts");

  bool bgLayer = hasBackgroundLayer();
  bool bgAxes  = hasGroupedBgAxes();
  bool bgKey   = hasGroupedBgKey();

  if (! bgLayer && ! bgAxes && ! bgKey)
    return;

  //---

  CQChartsBuffer *buffer = getBuffer(CQChartsBuffer::Type::BACKGROUND);
  if (! buffer->isActive()) return;

  QPainter *painter1 = beginPaint(buffer, painter);

  //---

  if (painter1) {
    auto *th = const_cast<CQChartsPlot *>(this);

    CQChartsPlotPainter device(th, painter1);

    drawBackgroundDeviceParts(&device, bgLayer, bgAxes, bgKey);

    //---

    if (debugQuadTree_) {
      painter1->setPen(Qt::black);

      objTreeData_.tree->draw(painter1);
    }
  }

  //---

  endPaint(buffer);
}

void
CQChartsPlot::
drawBackgroundDeviceParts(CQChartsPaintDevice *device, bool bgLayer, bool bgAxes,
                          bool bgKey) const
{
  // draw background (plot/data fill)
  if (bgLayer)
    drawBackgroundLayer(device);

  //---

  // draw axes/key below plot
  if (bgAxes)
    drawGroupedBgAxes(device);

  if (bgKey)
    drawBgKey(device);
}

void
CQChartsPlot::
drawMiddleParts(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawMiddleParts");

  CQChartsBuffer *buffer = getBuffer(CQChartsBuffer::Type::MIDDLE);
  if (! buffer->isActive()) return;

  //---

  bool bg          = hasGroupedObjs(CQChartsLayer::Type::BG_PLOT );
  bool mid         = hasGroupedObjs(CQChartsLayer::Type::MID_PLOT);
  bool fg          = hasGroupedObjs(CQChartsLayer::Type::FG_PLOT );
  bool annotations = hasGroupedAnnotations(CQChartsLayer::Type::ANNOTATION);

  if (! bg && ! mid && ! fg && ! annotations) {
    buffer->clear();
    return;
  }

  //---

  QPainter *painter1 = beginPaint(buffer, painter);

  //---

  if (painter1) {
    auto *th = const_cast<CQChartsPlot *>(this);

    CQChartsPlotPainter device(th, painter1);

    drawMiddleDeviceParts(&device, bg, mid, fg, annotations);
  }

  //---

  endPaint(buffer);
}

void
CQChartsPlot::
drawMiddleDeviceParts(CQChartsPaintDevice *device, bool bg, bool mid, bool fg,
                      bool annotations) const
{
  // draw objects (background, mid, foreground)
  if (bg ) drawGroupedObjs(device, CQChartsLayer::Type::BG_PLOT );
  if (mid) drawGroupedObjs(device, CQChartsLayer::Type::MID_PLOT);
  if (fg ) drawGroupedObjs(device, CQChartsLayer::Type::FG_PLOT );

  // draw annotations
  if (annotations)
    drawGroupedAnnotations(device, CQChartsLayer::Type::ANNOTATION);
}

void
CQChartsPlot::
drawForegroundParts(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawForegroundParts");

  bool fgAxes     = hasGroupedFgAxes();
  bool fgKey      = hasGroupedFgKey();
  bool title      = hasTitle();
  bool foreground = hasForeground();

  if (! fgAxes && ! fgKey && ! title && ! foreground)
    return;

  //---

  CQChartsBuffer *buffer = getBuffer(CQChartsBuffer::Type::FOREGROUND);
  if (! buffer->isActive()) return;

  QPainter *painter1 = beginPaint(buffer, painter);

  //---

  if (painter1) {
    auto *th = const_cast<CQChartsPlot *>(this);

    CQChartsPlotPainter device(th, painter1);

    drawForegroundDeviceParts(&device, fgAxes, fgKey, title, foreground);
  }

  //---

  endPaint(buffer);
}

void
CQChartsPlot::
drawForegroundDeviceParts(CQChartsPaintDevice *device, bool fgAxes, bool fgKey,
                          bool title, bool foreground) const
{
  // draw axes/key above plot
  if (fgAxes)
    drawGroupedFgAxes(device);

  if (fgKey)
    drawFgKey(device);

  //---

  // draw title
  if (title)
    drawTitle(device);

  //---

  // draw foreground
  if (foreground)
    execDrawForeground(device);
}

void
CQChartsPlot::
drawOverlayParts(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawOverlayParts");

  bool sel_objs         = hasGroupedObjs(CQChartsLayer::Type::SELECTION);
  bool sel_annotations  = hasGroupedAnnotations(CQChartsLayer::Type::SELECTION);
  bool boxes            = hasGroupedBoxes();
  bool edit_handles     = hasGroupedEditHandles();
  bool over_objs        = hasGroupedObjs(CQChartsLayer::Type::MOUSE_OVER);
  bool over_annotations = hasGroupedAnnotations(CQChartsLayer::Type::MOUSE_OVER);

  if (! sel_objs && ! sel_annotations && ! boxes &&
      ! edit_handles && ! over_objs && ! over_annotations)
    return;

  //---

  CQChartsBuffer *buffer = getBuffer(CQChartsBuffer::Type::OVERLAY);
  if (! buffer->isActive()) return;

  QPainter *painter1 = beginPaint(buffer, painter);

  //---

  if (painter1) {
    auto *th = const_cast<CQChartsPlot *>(this);

    CQChartsPlotPainter device(th, painter1);

    drawOverlayDeviceParts(&device, sel_objs, sel_annotations, boxes, edit_handles,
                           over_objs, over_annotations);
  }

  //---

  endPaint(buffer);
}

void
CQChartsPlot::
drawOverlayDeviceParts(CQChartsPaintDevice *device, bool sel_objs, bool sel_annotations,
                       bool boxes, bool edit_handles, bool over_objs, bool over_annotations) const
{
  // draw selection
  if (sel_objs)
    drawGroupedObjs(device, CQChartsLayer::Type::SELECTION);

  if (sel_annotations)
    drawGroupedAnnotations(device, CQChartsLayer::Type::SELECTION);

  //---

  // draw debug boxes
  if (boxes)
    drawGroupedBoxes(device);

  //---

  if (edit_handles) {
    if (device->isInteractive()) {
      auto *painter = dynamic_cast<CQChartsViewPlotPainter *>(device);

      drawGroupedEditHandles(painter->painter());
    }
  }

  //---

  // draw mouse over
  if (over_objs)
    drawGroupedObjs(device, CQChartsLayer::Type::MOUSE_OVER);

  if (over_annotations)
    drawGroupedAnnotations(device, CQChartsLayer::Type::MOUSE_OVER);
}

bool
CQChartsPlot::
hasBackgroundLayer() const
{
  // only first plot has background for overlay
  if (isOverlay() && ! isFirstPlot())
    return false;

  //---

  bool hasPlotBackground = (isPlotFilled() || isPlotStroked());
  bool hasDataBackground = (isDataFilled() || isDataStroked());
  bool hasFitBackground  = (isFitFilled () || isFitStroked ());
  bool hasBackground     = this->hasBackground();

  if (! hasPlotBackground && ! hasDataBackground && ! hasFitBackground && ! hasBackground)
    return false;

  if (! isLayerActive(CQChartsLayer::Type::BACKGROUND))
    return false;

  return true;
}

void
CQChartsPlot::
drawBackgroundLayer(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawBackgroundLayer");

  //---

  auto drawBackgroundRect = [&](bool isFilled, bool isStroked, const CQChartsGeom::BBox &rect,
                                const QColor &fillColor, const CQChartsAlpha &fillAlpha,
                                const CQChartsFillPattern &fillPattern,
                                const QColor &strokeColor, const CQChartsAlpha &strokeAlpha,
                                const CQChartsLength &strokeWidth,
                                const CQChartsLineDash &strokeDash, const CQChartsSides &sides) {
    if (isFilled) {
      QBrush brush;

      setBrush(brush, true, fillColor, fillAlpha, fillPattern);

      device->fillRect(rect, brush);
    }

    if (isStroked) {
      QPen pen;

      setPen(pen, true, strokeColor, strokeAlpha, strokeWidth, strokeDash);

      device->setPen(pen);

      drawBackgroundSides(device, rect, sides);
    }
  };

  if (isPlotFilled() || isPlotStroked())
    drawBackgroundRect(isPlotFilled(), isPlotStroked(), calcPlotRect(),
                       interpPlotFillColor(ColorInd()), plotFillAlpha(), plotFillPattern(),
                       interpPlotStrokeColor(ColorInd()), plotStrokeAlpha(),
                       plotStrokeWidth(), plotStrokeDash(), plotBorderSides());

  if (isFitFilled () || isFitStroked())
    drawBackgroundRect(isFitFilled(), isFitStroked(), fitBBox(),
                       interpFitFillColor(ColorInd()), fitFillAlpha(), fitFillPattern(),
                       interpFitStrokeColor(ColorInd()), fitStrokeAlpha(),
                       fitStrokeWidth(), fitStrokeDash(), fitBorderSides());

  if (isDataFilled() || isDataStroked())
    drawBackgroundRect(isDataFilled(), isDataStroked(), displayRangeBBox(),
                       interpDataFillColor(ColorInd()), dataFillAlpha(), dataFillPattern(),
                       interpDataStrokeColor(ColorInd()), dataStrokeAlpha(),
                       dataStrokeWidth(), dataStrokeDash(), dataBorderSides());

  //---

  if (this->hasBackground())
    execDrawBackground(device);
}

bool
CQChartsPlot::
hasBackground() const
{
  return false;
}

void
CQChartsPlot::
execDrawBackground(CQChartsPaintDevice *) const
{
}

void
CQChartsPlot::
drawBackgroundSides(CQChartsPaintDevice *device, const CQChartsGeom::BBox &bbox,
                    const CQChartsSides &sides) const
{
  if (sides.isAll()) {
    device->setBrush(Qt::NoBrush);

    device->drawRect(bbox);
  }
  else {
    if (sides.isTop   ()) device->drawLine(bbox.getUL(), bbox.getUR());
    if (sides.isLeft  ()) device->drawLine(bbox.getUL(), bbox.getLL());
    if (sides.isBottom()) device->drawLine(bbox.getLL(), bbox.getLR());
    if (sides.isRight ()) device->drawLine(bbox.getUR(), bbox.getLR());
  }
}

bool
CQChartsPlot::
hasGroupedBgAxes() const
{
  if (isOverlay()) {
    if (! isFirstPlot())
      return false;

    bool anyAxis = processOverlayPlots([&](const CQChartsPlot *plot) {
      return plot->hasBgAxes();
    }, false);

    if (! anyAxis)
      return false;
  }
  else {
    if (! hasBgAxes())
      return false;
  }

  //---

  if (! isLayerActive(CQChartsLayer::Type::BG_AXES))
    return false;

  return true;
}

bool
CQChartsPlot::
hasBgAxes() const
{
  // just axis grid on background
  bool showXAxis = (xAxis() && xAxis()->isVisible());
  bool showYAxis = (yAxis() && yAxis()->isVisible());

  bool showXGrid = (showXAxis && ! xAxis()->isGridAbove() && xAxis()->isDrawGrid());
  bool showYGrid = (showYAxis && ! yAxis()->isGridAbove() && yAxis()->isDrawGrid());

  if (! showXGrid && ! showYGrid)
    return false;

  return true;
}

void
CQChartsPlot::
drawGroupedBgAxes(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawGroupedBgAxes");

  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      device->setPlot(const_cast<CQChartsPlot *>(plot));

      plot->drawBgAxes(device);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    drawBgAxes(device);
  }
}

void
CQChartsPlot::
drawBgAxes(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawBgAxes");

  bool showXAxis = (xAxis() && xAxis()->isVisible());
  bool showYAxis = (yAxis() && yAxis()->isVisible());

  bool showXGrid = (showXAxis && ! xAxis()->isGridAbove() && xAxis()->isDrawGrid());
  bool showYGrid = (showYAxis && ! yAxis()->isGridAbove() && yAxis()->isDrawGrid());

  //---

  if (showXGrid)
    xAxis()->drawGrid(this, device);

  if (showYGrid)
    yAxis()->drawGrid(this, device);
}

bool
CQChartsPlot::
hasGroupedBgKey() const
{
  if (isOverview())
    return false;

  CQChartsPlotKey *key1 = nullptr;

  if (isOverlay()) {
    // only draw key under first plot - use first plot key (for overlay)
    const CQChartsPlot *plot = firstPlot();

    if (plot)
      key1 = plot->getFirstPlotKey();
  }
  else {
    key1 = this->key();
  }

  //---

  bool showKey = (key1 && ! key1->isAbove());

  if (! showKey)
    return false;

  //---

  if (! isLayerActive(CQChartsLayer::Type::BG_KEY))
    return false;

  return true;
}

void
CQChartsPlot::
drawBgKey(CQChartsPaintDevice *device) const
{
  if (isPreview())
    return;

  CQPerfTrace trace("CQChartsPlot::drawBgKey");

  if (isOverlay()) {
    // only draw key under first plot - use first plot key (for overlay)
    //const CQChartsPlot *plot = firstPlot();

    processOverlayPlots([&](const CQChartsPlot *plot) {
      CQChartsPlotKey *key = plot->key();
      if (! key) return;

      device->setPlot(const_cast<CQChartsPlot *>(plot));

      key->draw(device);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    CQChartsPlotKey *key = this->key();

    if (key)
      key->draw(device);
  }
}

bool
CQChartsPlot::
hasGroupedObjs(const CQChartsLayer::Type &layerType) const
{
  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return false;

    bool anyObjs = processOverlayPlots([&](const CQChartsPlot *plot) {
      return plot->hasObjs(layerType);
    }, false);

    if (! anyObjs)
      return false;
  }
  else {
    if (! hasObjs(layerType))
      return false;
  }

  //---

  if (! isLayerActive(layerType))
    return false;

  return true;
}

void
CQChartsPlot::
drawGroupedObjs(CQChartsPaintDevice *device, const CQChartsLayer::Type &layerType) const
{
  CQPerfTrace trace("CQChartsPlot::drawGroupedObjs");

  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      device->setPlot(const_cast<CQChartsPlot *>(plot));

      plot->execDrawObjs(device, layerType);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    execDrawObjs(device, layerType);
  }
}

bool
CQChartsPlot::
hasObjs(const CQChartsLayer::Type &layerType) const
{
  auto bbox = displayRangeBBox();

  bool anyObjs = false;

  for (const auto &plotObj : plotObjects()) {
    if      (layerType == CQChartsLayer::Type::SELECTION) {
      if (! plotObj->isSelected())
        continue;
    }
    else if (layerType == CQChartsLayer::Type::MOUSE_OVER) {
      if (! plotObj->isInside())
        continue;
    }

    if (! bbox.overlaps(plotObj->rect()))
      continue;

    anyObjs = true;

    break;
  }

  if (! anyObjs)
    return false;

  //---

  if (! isLayerActive(layerType))
    return false;

  return true;
}

void
CQChartsPlot::
execDrawObjs(CQChartsPaintDevice *device, const CQChartsLayer::Type &layerType) const
{
  CQPerfTrace trace("CQChartsPlot::execDrawObjs");

  if (! isVisible())
    return;

  //---

  // set draw layer
  view()->setDrawLayerType(layerType);

  //---

  // init paint (clipped)
  device->save();

  setClipRect(device);

  //---

  if (layerType == CQChartsLayer::Type::MID_PLOT)
    preDrawObjs(device);

  //---

  auto bbox = displayRangeBBox();

  for (const auto &plotObj : plotObjects()) {
    if (! plotObj->isVisible())
      continue;

    // skip unselected objects on selection layer
    if      (layerType == CQChartsLayer::Type::SELECTION) {
      if (! plotObj->isSelected())
        continue;
    }
    // skip non-inside objects on mouse over layer
    else if (layerType == CQChartsLayer::Type::MOUSE_OVER) {
      if (! plotObj->isInside())
        continue;
    }

    //---

    // skip objects not inside plot
    if (! bbox.overlaps(plotObj->rect()))
      continue;

    //---

    // draw object on layer
    if      (layerType == CQChartsLayer::Type::BG_PLOT)
      plotObj->drawBg(device);
    else if (layerType == CQChartsLayer::Type::FG_PLOT)
      plotObj->drawFg(device);
    else if (layerType == CQChartsLayer::Type::MID_PLOT)
      plotObj->draw  (device);
    else if (layerType == CQChartsLayer::Type::SELECTION) {
      plotObj->draw  (device);
      plotObj->drawFg(device);
    }
    else if (layerType == CQChartsLayer::Type::MOUSE_OVER) {
      plotObj->draw  (device);
      plotObj->drawFg(device);
    }

    //---

    // show debug box
    if (showBoxes())
      plotObj->drawDebugRect(device);
  }

  //---

  if (layerType == CQChartsLayer::Type::MID_PLOT)
    postDrawObjs(device);

  //---

  device->restore();
}

bool
CQChartsPlot::
hasGroupedFgAxes() const
{
  if (isOverlay()) {
    if (! isFirstPlot())
      return false;

    bool anyAxis = processOverlayPlots([&](const CQChartsPlot *plot) {
      return plot->hasFgAxes();
    }, false);

    if (! anyAxis)
      return false;
  }
  else {
    if (! hasFgAxes())
      return false;
  }

  //---

  if (! isLayerActive(CQChartsLayer::Type::FG_AXES))
    return false;

  return true;
}

bool
CQChartsPlot::
hasFgAxes() const
{
  bool showXAxis = (xAxis() && xAxis()->isVisible());
  bool showYAxis = (yAxis() && yAxis()->isVisible());

  if (! showXAxis && ! showYAxis)
    return false;

  return true;
}

void
CQChartsPlot::
drawGroupedFgAxes(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawGroupedFgAxes");

  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      device->setPlot(const_cast<CQChartsPlot *>(plot));

      plot->drawFgAxes(device);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    drawFgAxes(device);
  }
}

void
CQChartsPlot::
drawFgAxes(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawFgAxes");

  bool showXAxis = (xAxis() && xAxis()->isVisible());
  bool showYAxis = (yAxis() && yAxis()->isVisible());

  bool showXGrid = (showXAxis && xAxis()->isGridAbove() && xAxis()->isDrawGrid());
  bool showYGrid = (showYAxis && yAxis()->isGridAbove() && yAxis()->isDrawGrid());

  //---

  if (showXGrid)
    xAxis()->drawGrid(this, device);

  if (showYGrid)
    yAxis()->drawGrid(this, device);

  //---

  if (showXAxis)
    xAxis()->draw(this, device);

  if (showYAxis)
    yAxis()->draw(this, device);
}

bool
CQChartsPlot::
hasGroupedFgKey() const
{
  if (isOverview())
    return false;

  CQChartsPlotKey *key1 = nullptr;

  if (isOverlay()) {
    // only draw fg key on last plot - use first plot key (for overlay)
    const CQChartsPlot *plot = lastPlot();

    if (plot)
      key1 = plot->getFirstPlotKey();
  }
  else {
    key1 = this->key();
  }

  //---

  bool showKey = (key1 && key1->isAbove());

  if (! showKey)
    return false;

  //---

  if (! isLayerActive(CQChartsLayer::Type::FG_KEY))
    return false;

  return true;
}

void
CQChartsPlot::
drawFgKey(CQChartsPaintDevice *device) const
{
  if (isPreview())
    return;

  CQPerfTrace trace("CQChartsPlot::drawFgKey");

  if (isOverlay()) {
    // only draw key above last plot - use first plot key (for overlay)
    //const CQChartsPlot *plot = lastPlot();

    processOverlayPlots([&](const CQChartsPlot *plot) {
      CQChartsPlotKey *key = plot->key();
      if (! key) return;

      device->setPlot(const_cast<CQChartsPlot *>(plot));

      key->draw(device);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    CQChartsPlotKey *key = this->key();

    if (key)
      key->draw(device);
  }
}

bool
CQChartsPlot::
hasTitle() const
{
  // only first plot has title for overlay
  if (isOverlay() && ! isFirstPlot())
    return false;

  if (! title() || ! title()->isDrawn())
    return false;

  //---

  if (! isLayerActive(CQChartsLayer::Type::TITLE))
    return false;

  return true;
}

void
CQChartsPlot::
drawTitle(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawTitle");

  title()->draw(device);
}

bool
CQChartsPlot::
hasGroupedAnnotations(const CQChartsLayer::Type &layerType) const
{
  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return false;

    bool anyObjs = processOverlayPlots([&](const CQChartsPlot *plot) {
      return plot->hasAnnotations(layerType);
    }, false);

    if (! anyObjs)
      return false;
  }
  else {
    if (! hasAnnotations(layerType))
      return false;
  }

  //---

  if (! isLayerActive(layerType))
    return false;

  return true;
}

void
CQChartsPlot::
drawGroupedAnnotations(CQChartsPaintDevice *device, const CQChartsLayer::Type &layerType) const
{
  CQPerfTrace trace("CQChartsPlot::drawGroupedAnnotations");

  if (! hasGroupedAnnotations(layerType))
    return;

  //---

  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      device->setPlot(const_cast<CQChartsPlot *>(plot));

      plot->drawAnnotations(device, layerType);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    drawAnnotations(device, layerType);
  }
}

bool
CQChartsPlot::
hasAnnotations(const CQChartsLayer::Type &layerType) const
{
  bool anyObjs = false;

  for (const auto &annotation : annotations()) {
    if      (layerType == CQChartsLayer::Type::SELECTION) {
      if (! annotation->isSelected())
        continue;
    }
    else if (layerType == CQChartsLayer::Type::MOUSE_OVER) {
      if (! annotation->isInside())
        continue;
    }

//  if (! bbox.overlaps(annotation->bbox()))
//    continue;

    anyObjs = true;

    break;
  }

  if (! anyObjs)
    return false;

  //---

  if (! isLayerActive(layerType))
    return false;

  return true;
}

void
CQChartsPlot::
drawAnnotations(CQChartsPaintDevice *device, const CQChartsLayer::Type &layerType) const
{
  CQPerfTrace trace("CQChartsPlot::drawAnnotations");

  // set draw layer
  view()->setDrawLayerType(layerType);

  //---

  for (auto &annotation : annotations()) {
    if      (layerType == CQChartsLayer::Type::SELECTION) {
      if (! annotation->isSelected())
        continue;
    }
    else if (layerType == CQChartsLayer::Type::MOUSE_OVER) {
      if (! annotation->isInside())
        continue;
    }

//  if (! bbox.overlaps(annotation->bbox()))
//    continue;

    annotation->draw(device);
  }
}

bool
CQChartsPlot::
hasForeground() const
{
  return false;
}

void
CQChartsPlot::
execDrawForeground(CQChartsPaintDevice *) const
{
}

bool
CQChartsPlot::
hasGroupedBoxes() const
{
  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return false;

    bool anyObjs = processOverlayPlots([&](const CQChartsPlot *plot) {
      return plot->hasBoxes();
    }, false);

    if (! anyObjs)
      return false;
  }
  else {
    if (! hasBoxes())
      return false;
  }

  //---

  if (! isLayerActive(CQChartsLayer::Type::BOXES))
    return false;

  return true;
}

void
CQChartsPlot::
drawGroupedBoxes(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawGroupedBoxes");

  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      device->setPlot(const_cast<CQChartsPlot *>(plot));

      plot->drawBoxes(device);
    });

    device->setPlot(const_cast<CQChartsPlot *>(this));
  }
  else {
    drawBoxes(device);
  }
}

bool
CQChartsPlot::
hasBoxes() const
{
  if (! showBoxes())
    return false;

  //---

  if (! isLayerActive(CQChartsLayer::Type::BOXES))
    return false;

  return true;
}

void
CQChartsPlot::
drawBoxes(CQChartsPaintDevice *device) const
{
  CQPerfTrace trace("CQChartsPlot::drawBoxes");

  auto bbox = fitBBox();

  drawWindowColorBox(device, bbox);

  drawWindowColorBox(device, dataFitBBox   ());
  drawWindowColorBox(device, axesFitBBox   ());
  drawWindowColorBox(device, keyFitBBox    ());
  drawWindowColorBox(device, titleFitBBox  ());
  drawWindowColorBox(device, annotationBBox());

  //---

  drawWindowColorBox(device, CQChartsUtil::rangeBBox(calcDataRange_ ), Qt::green);
  drawWindowColorBox(device, CQChartsUtil::rangeBBox(dataRange_     ), Qt::green);
  drawWindowColorBox(device, CQChartsUtil::rangeBBox(outerDataRange_), Qt::green);
}

bool
CQChartsPlot::
hasGroupedEditHandles() const
{
  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return false;

    bool anyObjs = processOverlayPlots([&](const CQChartsPlot *plot) {
      return plot->hasEditHandles();
    }, false);

    if (! anyObjs)
      return false;
  }
  else {
    if (! hasEditHandles())
      return false;
  }

  //---

  if (! isLayerActive(CQChartsLayer::Type::EDIT_HANDLE))
    return false;

  return true;
}

void
CQChartsPlot::
drawGroupedEditHandles(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawGroupedEditHandles");

  // for overlay draw all combine objects on common layers
  if (isOverlay()) {
    if (! isFirstPlot())
      return;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      plot->drawEditHandles(painter);
    });
  }
  else {
    drawEditHandles(painter);
  }
}

bool
CQChartsPlot::
hasEditHandles() const
{
  if (view()->mode() != CQChartsView::Mode::EDIT)
    return false;

  //---

  bool selected = (isSelected() ||
                   (title() && title()->isSelected()) ||
                   (xAxis() && xAxis()->isSelected()) ||
                   (yAxis() && yAxis()->isSelected()));

  if (! selected) {
    const CQChartsPlotKey *key1 = getFirstPlotKey();

    selected = (key1 && key1->isSelected());
  }

  if (! selected) {
    for (const auto &annotation : annotations()) {
      if (annotation->isSelected()) {
        selected = true;
        break;
      }
    }
  }

  if (! selected)
    return false;

  //---

  if (! isLayerActive(CQChartsLayer::Type::EDIT_HANDLE))
    return false;

  return true;
}

void
CQChartsPlot::
drawEditHandles(QPainter *painter) const
{
  CQPerfTrace trace("CQChartsPlot::drawEditHandles");

  const CQChartsPlotKey *key1 = getFirstPlotKey();

  if      (isSelected()) {
    const_cast<CQChartsPlot *>(this)->editHandles_->setBBox(this->viewBBox());

    editHandles_->draw(painter);
  }
  else if (title() && title()->isSelected()) {
    title()->drawEditHandles(painter);
  }
  else if (key1 && key1->isSelected()) {
    key1->drawEditHandles(painter);
  }
  else if (xAxis() && xAxis()->isSelected()) {
    xAxis()->drawEditHandles(painter);
  }
  else if (yAxis() && yAxis()->isSelected()) {
    yAxis()->drawEditHandles(painter);
  }
  else {
    for (const auto &annotation : annotations()) {
      if (annotation->isSelected())
        annotation->drawEditHandles(painter);
    }
  }
}

const CQChartsLayer::Type &
CQChartsPlot::
drawLayerType() const
{
  return view()->drawLayerType();
}

//---

CQChartsGeom::BBox
CQChartsPlot::
displayRangeBBox() const
{
  // calc current (zoomed/panned) data range
  double xmin, ymin, xmax, ymax;

  displayRange().getWindowRange(&xmin, &ymin, &xmax, &ymax);

  CQChartsGeom::BBox bbox(xmin, ymin, xmax, ymax);

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
calcDataPixelRect() const
{
  // calc current (zoomed/panned) pixel range
  auto bbox = displayRangeBBox();

  auto pbbox = windowToPixel(bbox);

  return pbbox;
}

CQChartsGeom::BBox
CQChartsPlot::
calcPlotRect() const
{
  return pixelToWindow(calcPlotPixelRect());
}

CQChartsGeom::BBox
CQChartsPlot::
calcPlotPixelRect() const
{
  return view()->windowToPixel(viewBBox());
}

CQChartsGeom::Size
CQChartsPlot::
calcPixelSize() const
{
  auto bbox = calcPlotPixelRect();

  return CQChartsGeom::Size(bbox.getWidth(), bbox.getHeight());
}

//---

void
CQChartsPlot::
updateAutoFit()
{
  // auto fit based on last draw
  if (needsAutoFit_) {
    needsAutoFit_ = false;

    autoFit();
  }
}

void
CQChartsPlot::
autoFit()
{
  if (isOverlay() && ! isFirstPlot())
    return;

  CQPerfTrace trace("CQChartsPlot::autoFit");

  zoomFull(/*notify*/false);

  if (isOverlay()) {
    if (prevPlot())
      return;

    //---

    // combine bboxes of overlay plots
    CQChartsGeom::BBox bbox;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      auto bbox1 = plot->fitBBox();
      auto bbox2 = plot->windowToPixel(bbox1);
      auto bbox3 = pixelToWindow(bbox2);

      bbox += bbox3;
    });

    //---

    // set all overlay plot bboxes
    using BBoxes = std::vector<CQChartsGeom::BBox>;

    BBoxes bboxes;

    processOverlayPlots([&](const CQChartsPlot *plot) {
      auto bbox1 = windowToPixel(bbox);
      auto bbox2 = plot->pixelToWindow(bbox1);

      bboxes.push_back(bbox2);
    });

    int i = 0;

    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->setFitBBox(bboxes[i]);

      ++i;
    });
  }
  else {
    autoFitOne();
  }
}

void
CQChartsPlot::
autoFitOne()
{
#if 0
  for (int i = 0; i < 3; ++i) {
    auto bbox = fitBBox();

    setFitBBox(bbox);

    updateRangeAndObjs();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
  }
#else
  auto bbox = fitBBox();

  setFitBBox(bbox);

  updateRangeAndObjs();
#endif

  emit zoomPanChanged();
}

void
CQChartsPlot::
setFitBBox(const CQChartsGeom::BBox &bbox)
{
  // calc margin so plot box fits in specified box
  auto pbbox = displayRangeBBox();

  double left   = 100.0*(pbbox.getXMin() -  bbox.getXMin())/bbox.getWidth ();
  double bottom = 100.0*(pbbox.getYMin() -  bbox.getYMin())/bbox.getHeight();
  double right  = 100.0*( bbox.getXMax() - pbbox.getXMax())/bbox.getWidth ();
  double top    = 100.0*( bbox.getYMax() - pbbox.getYMax())/bbox.getHeight();

  if (isInvertX()) std::swap(left, right );
  if (isInvertY()) std::swap(top , bottom);

  outerMargin_ = CQChartsPlotMargin(left, top, right, bottom);

  updateMargins();
}

CQChartsGeom::BBox
CQChartsPlot::
fitBBox() const
{
  CQChartsGeom::BBox bbox;

  bbox += dataFitBBox   ();
  bbox += axesFitBBox   ();
  bbox += keyFitBBox    ();
  bbox += titleFitBBox  ();
  bbox += annotationBBox();

  // add margin (TODO: config pixel margin size)
  auto marginSize = pixelToWindowSize(CQChartsGeom::Size(8, 8));

  bbox.expand(-marginSize.width(), -marginSize.height(),
               marginSize.width(),  marginSize.height());

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
dataFitBBox() const
{
  auto bbox = displayRangeBBox();

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
axesFitBBox() const
{
  CQChartsGeom::BBox bbox;

  if (xAxis() && xAxis()->isVisible())
    bbox += xAxis()->fitBBox();

  if (yAxis() && yAxis()->isVisible())
    bbox += yAxis()->fitBBox();

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
keyFitBBox() const
{
  CQChartsGeom::BBox bbox;

  if (isKeyVisibleAndNonEmpty()) {
    auto bbox1 = key()->bbox();

    if (bbox1.isSet()) {
      bbox.add(bbox1.getCenter());

      if (! key()->isPixelWidthExceeded())
        bbox.addX(bbox1);
      if (! key()->isPixelHeightExceeded())
        bbox.addY(bbox1);
    }
  }

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
titleFitBBox() const
{
  CQChartsGeom::BBox bbox;

  if (title() && title()->isVisible())
    bbox += title()->bbox();

  return bbox;
}

CQChartsGeom::BBox
CQChartsPlot::
calcFitPixelRect() const
{
  // calc current (zoomed/panned) pixel range
  auto bbox = fitBBox();

  auto pbbox = windowToPixel(bbox);

  return pbbox;
}

CQChartsGeom::BBox
CQChartsPlot::
annotationBBox() const
{
  if (annotationBBox_.isSet())
    return annotationBBox_;

  annotationBBox_ = calcAnnotationBBox();

  return annotationBBox_;
}

//------

CQChartsArrowAnnotation *
CQChartsPlot::
addArrowAnnotation(const CQChartsPosition &start, const CQChartsPosition &end)
{
  auto *annotation = new CQChartsArrowAnnotation(this, start, end);

  addAnnotation(annotation);

  return annotation;
}

CQChartsEllipseAnnotation *
CQChartsPlot::
addEllipseAnnotation(const CQChartsPosition &center, const CQChartsLength &xRadius,
                     const CQChartsLength &yRadius)
{
  auto *annotation = new CQChartsEllipseAnnotation(this, center, xRadius, yRadius);

  addAnnotation(annotation);

  return annotation;
}

CQChartsImageAnnotation *
CQChartsPlot::
addImageAnnotation(const CQChartsPosition &pos, const QImage &image)
{
  auto *annotation = new CQChartsImageAnnotation(this, pos, image);

  addAnnotation(annotation);

  return annotation;
}

CQChartsImageAnnotation *
CQChartsPlot::
addImageAnnotation(const CQChartsRect &rect, const QImage &image)
{
  auto *annotation = new CQChartsImageAnnotation(this, rect, image);

  addAnnotation(annotation);

  return annotation;
}

CQChartsPieSliceAnnotation *
CQChartsPlot::
addPieSliceAnnotation(const CQChartsPosition &pos, const CQChartsLength &innerRadius,
                      const CQChartsLength &outerRadius, const CQChartsAngle &startAngle,
                      const CQChartsAngle &spanAngle)
{
  auto *annotation = new CQChartsPieSliceAnnotation(this, pos, innerRadius, outerRadius,
                                                    startAngle, spanAngle);

  addAnnotation(annotation);

  return annotation;
}

CQChartsPointAnnotation *
CQChartsPlot::
addPointAnnotation(const CQChartsPosition &pos, const CQChartsSymbol &type)
{
  auto *annotation = new CQChartsPointAnnotation(this, pos, type);

  addAnnotation(annotation);

  return annotation;
}

CQChartsPointSetAnnotation *
CQChartsPlot::
addPointSetAnnotation(const CQChartsPoints &values)
{
  auto *annotation = new CQChartsPointSetAnnotation(this, values);

  addAnnotation(annotation);

  return annotation;
}

CQChartsPolygonAnnotation *
CQChartsPlot::
addPolygonAnnotation(const CQChartsPolygon &points)
{
  auto *annotation = new CQChartsPolygonAnnotation(this, points);

  addAnnotation(annotation);

  return annotation;
}

CQChartsPolylineAnnotation *
CQChartsPlot::
addPolylineAnnotation(const CQChartsPolygon &points)
{
  auto *annotation = new CQChartsPolylineAnnotation(this, points);

  addAnnotation(annotation);

  return annotation;
}

CQChartsRectangleAnnotation *
CQChartsPlot::
addRectangleAnnotation(const CQChartsRect &rect)
{
  auto *annotation = new CQChartsRectangleAnnotation(this, rect);

  addAnnotation(annotation);

  return annotation;
}

CQChartsTextAnnotation *
CQChartsPlot::
addTextAnnotation(const CQChartsPosition &pos, const QString &text)
{
  auto *annotation = new CQChartsTextAnnotation(this, pos, text);

  addAnnotation(annotation);

  return annotation;
}

CQChartsTextAnnotation *
CQChartsPlot::
addTextAnnotation(const CQChartsRect &rect, const QString &text)
{
  auto *annotation = new CQChartsTextAnnotation(this, rect, text);

  addAnnotation(annotation);

  return annotation;
}

CQChartsValueSetAnnotation *
CQChartsPlot::
addValueSetAnnotation(const CQChartsRect &rectangle, const CQChartsReals &values)
{
  auto *annotation = new CQChartsValueSetAnnotation(this, rectangle, values);

  addAnnotation(annotation);

  return annotation;
}

CQChartsButtonAnnotation *
CQChartsPlot::
addButtonAnnotation(const CQChartsPosition &pos, const QString &text)
{
  auto *annotation = new CQChartsButtonAnnotation(this, pos, text);

  addAnnotation(annotation);

  return annotation;
}

void
CQChartsPlot::
addAnnotation(CQChartsAnnotation *annotation)
{
  annotations_.push_back(annotation);

  connect(annotation, SIGNAL(idChanged()), this, SLOT(updateAnnotationSlot()));
  connect(annotation, SIGNAL(dataChanged()), this, SLOT(updateAnnotationSlot()));

  annotation->addProperties(propertyModel(), "annotations");

//emit annotationAdded(annotation->id());
  emit annotationsChanged();
}

CQChartsAnnotation *
CQChartsPlot::
getAnnotationByName(const QString &id) const
{
  for (auto &annotation : annotations()) {
    if (annotation->id() == id)
      return annotation;
  }

  return nullptr;
}

CQChartsAnnotation *
CQChartsPlot::
getAnnotationByInd(int ind) const
{
  for (auto &annotation : annotations()) {
    if (annotation->ind() == ind)
      return annotation;
  }

  return nullptr;
}

void
CQChartsPlot::
removeAnnotation(CQChartsAnnotation *annotation)
{
  int pos = 0;

  for (auto &annotation1 : annotations_) {
    if (annotation1 == annotation)
      break;

    ++pos;
  }

  int n = annotations_.size();

  assert(pos >= 0 && pos < n);

  propertyModel()->removeProperties("annotations/" + annotation->propertyId());

  delete annotation;

  for (int i = pos + 1; i < n; ++i)
    annotations_[i - 1] = annotations_[i];

  annotations_.pop_back();

  emit annotationsChanged();
}

void
CQChartsPlot::
removeAllAnnotations()
{
  for (auto &annotation : annotations_)
    delete annotation;

  annotations_.clear();

  propertyModel()->removeProperties("annotations");

  emit annotationsChanged();
}

void
CQChartsPlot::
updateAnnotationSlot()
{
  if (editing_) {
    invalidateLayer(CQChartsBuffer::Type::FOREGROUND);

    invalidateOverlay();
  }
  else
    drawForeground();

  emit annotationsChanged();
}

//------

CQChartsPlotObj *
CQChartsPlot::
getPlotObject(const QString &objectId) const
{
  for (auto &plotObj : plotObjects()) {
    if (plotObj->id() == objectId)
      return plotObj;
  }

  return nullptr;
}

CQChartsObj *
CQChartsPlot::
getObject(const QString &objectId) const
{
  CQChartsPlotObj *plotObj = getPlotObject(objectId);
  if (plotObj) return plotObj;

  auto checkObjs = [&](std::initializer_list<CQChartsObj *> objs) {
    for (const auto &obj : objs)
      if (obj && obj->id() == objectId)
        return obj;

    return static_cast<CQChartsObj *>(nullptr);
  };

  return checkObjs({xAxis(), yAxis(), key(), title()});
}

QList<QModelIndex>
CQChartsPlot::
getObjectInds(const QString &objectId) const
{
  QList<QModelIndex> inds;

  CQChartsPlotObj *plotObj = getPlotObject(objectId);

  if (plotObj) {
    CQChartsPlotObj::Indices inds1;

    plotObj->getSelectIndices(inds1);

    for (auto &ind1 : inds1)
      inds.push_back(ind1);
  }

  return inds;
}

//------

CQChartsLayer *
CQChartsPlot::
initLayer(const CQChartsLayer::Type &type, const CQChartsBuffer::Type &buffer, bool active)
{
  auto pb = buffers_.find(buffer);

  if (pb == buffers_.end()) {
    auto *layerBuffer = new CQChartsBuffer(buffer);

    pb = buffers_.insert(pb, Buffers::value_type(buffer, layerBuffer));
  }

  //CQChartsBuffer *layerBuffer = (*pb).second;

  //---

  auto pl = layers_.find(type);

  if (pl == layers_.end()) {
    auto *layer = new CQChartsLayer(type, buffer);

    pl = layers_.insert(pl, Layers::value_type(type, layer));
  }

  CQChartsLayer *layer = (*pl).second;

  layer->setActive(active);

  return layer;
}

void
CQChartsPlot::
setLayerActive(const CQChartsLayer::Type &type, bool b)
{
  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->setLayerActive1(type, b);
    });
  }
  else
    setLayerActive1(type, b);
}

void
CQChartsPlot::
setLayerActive1(const CQChartsLayer::Type &type, bool b)
{
  CQChartsLayer *layer = getLayer(type);

  layer->setActive(b);

  setLayersChanged(true);
}

bool
CQChartsPlot::
isLayerActive(const CQChartsLayer::Type &type) const
{
  CQChartsLayer *layer = getLayer(type);

  return layer->isActive();
}

void
CQChartsPlot::
invalidateLayers()
{
  if (! isUpdatesEnabled()) {
    updatesData_.invalidateLayers = true;
    return;
  }

  //---

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      for (auto &buffer : plot->buffers_)
        buffer.second->setValid(false);
    });
  }
  else {
    for (auto &buffer : buffers_)
      buffer.second->setValid(false);
  }

  setLayersChanged(true);

  fromInvalidate_ = true;

  update();
}

void
CQChartsPlot::
invalidateLayer(const CQChartsBuffer::Type &type)
{
  if (! isUpdatesEnabled()) {
    updatesData_.invalidateLayers = true;
    return;
  }

  //assert(type != CQChartsBuffer::Type::MIDDLE);

  if (isOverlay()) {
    processOverlayPlots([&](CQChartsPlot *plot) {
      plot->invalidateLayer1(type);
    });
  }
  else {
    invalidateLayer1(type);
  }
}

void
CQChartsPlot::
invalidateLayer1(const CQChartsBuffer::Type &type)
{
//std::cerr << "invalidateLayer1: " << CQChartsBuffer::typeName(type) << "\n";
  CQChartsBuffer *layer = getBuffer(type);

  layer->setValid(false);

  setLayersChanged(false);

  fromInvalidate_ = true;

  update();
}

void
CQChartsPlot::
setLayersChanged(bool update)
{
  if (updateData_.rangeThread.busy.load())
    return;

  //---

  {
    LockMutex lock(this, "setLayersChanged");

    interruptDraw();
  }

  if (update) {
    if (isUpdatesEnabled())
      updateDraw();
  }
  else {
    //drawNonMiddleParts(view()->ipainter());
    drawParts(view()->ipainter());

    fromInvalidate_ = true;

    this->update();
  }

  emit layersChanged();
}

CQChartsBuffer *
CQChartsPlot::
getBuffer(const CQChartsBuffer::Type &type) const
{
  auto p = buffers_.find(type);
  assert(p != buffers_.end());

  return (*p).second;
}

CQChartsLayer *
CQChartsPlot::
getLayer(const CQChartsLayer::Type &type) const
{
  auto p = layers_.find(type);
  assert(p != layers_.end());

  return (*p).second;
}

//---

void
CQChartsPlot::
setClipRect(CQChartsPaintDevice *device) const
{
  const CQChartsPlot *plot1 = firstPlot();

  if      (plot1->isDataClip()) {
    auto bbox  = displayRangeBBox();
    auto abbox = annotationBBox();

    if      (dataScaleX() <= 1.0 && dataScaleY() <= 1.0)
      bbox.add(abbox);
    else if (dataScaleX() <= 1.0)
      bbox.addX(abbox);
    else if (dataScaleY() <= 1.0)
      bbox.addY(abbox);

    device->setClipRect(bbox);
  }
  else if (plot1->isPlotClip()) {
    auto plotRect = calcPlotRect();

    device->setClipRect(plotRect);
  }
}

QPainter *
CQChartsPlot::
beginPaint(CQChartsBuffer *buffer, QPainter *painter, const QRectF &rect) const
{
  drawBuffer_ = buffer->type();

  if (! view()->isBufferLayers())
    return painter;

  // resize and clear
  QRectF prect = (! rect.isValid() ? calcPlotPixelRect().qrect() : rect);

  QPainter *painter1 = buffer->beginPaint(painter, prect, view()->isAntiAlias());

  // don't paint if not active
  if (! buffer->isActive())
    return nullptr;

  return painter1;
}

void
CQChartsPlot::
endPaint(CQChartsBuffer *buffer) const
{
  if (! view()->isBufferLayers())
    return;

  buffer->endPaint(false);
}

CQChartsPlotKey *
CQChartsPlot::
getFirstPlotKey() const
{
  const CQChartsPlot *plot = firstPlot();

  while (plot) {
    CQChartsPlotKey *key = plot->key();

    if (key && key->isVisibleAndNonEmpty())
      return key;

    plot = plot->nextPlot();
  }

  return nullptr;
}

//------

void
CQChartsPlot::
drawSymbol(CQChartsPaintDevice *device, const CQChartsGeom::Point &p, const CQChartsSymbol &symbol,
           const CQChartsLength &size, const CQChartsPenBrush &penBrush) const
{
  CQChartsDrawUtil::setPenBrush(device, penBrush);

  CQChartsDrawUtil::drawSymbol(device, symbol, p, size);
}

void
CQChartsPlot::
drawSymbol(CQChartsPaintDevice *device, const CQChartsGeom::Point &p, const CQChartsSymbol &symbol,
           const CQChartsLength &size) const
{
  if (bufferSymbols_) {
    auto *painter = dynamic_cast<CQChartsViewPlotPainter *>(device);

    if (painter) {
      double sx, sy;

      plotSymbolSize(size, sx, sy);

      drawBufferedSymbol(painter->painter(), p, symbol, std::min(sx, sy));
    }
    else {
      CQChartsDrawUtil::drawSymbol(device, symbol, p, size);
    }
  }
  else {
    CQChartsDrawUtil::drawSymbol(device, symbol, p, size);
  }
}

void
CQChartsPlot::
drawBufferedSymbol(QPainter *painter, const CQChartsGeom::Point &p,
                   const CQChartsSymbol &symbol, double size) const
{
  auto cmpPen = [](const QPen &pen1, const QPen &pen2) {
    if (pen1.style () != pen2.style ()) return false;
    if (pen1.color () != pen2.color ()) return false;
    if (pen1.widthF() != pen2.widthF()) return false;
    return true;
  };

  auto cmpBrush = [](const QBrush &brush1, const QBrush &brush2) {
    if (brush1.style () != brush2.style ()) return false;
    if (brush1.color () != brush2.color ()) return false;
    return true;
  };

  struct ImageBuffer {
    CQChartsSymbol symbol;
    double         size { 0.0 };
    int            isize { 0 };
    QPen           pen;
    QBrush         brush;
    QImage         image;
  };

  static ImageBuffer imageBuffer;

  if (symbol != imageBuffer.symbol ||
      size   != imageBuffer.size   ||
      ! cmpPen  (painter->pen  (), imageBuffer.pen  ) ||
      ! cmpBrush(painter->brush(), imageBuffer.brush)) {
    imageBuffer.symbol = symbol;
    imageBuffer.size   = size;
    imageBuffer.isize  = CMathRound::RoundUp(2*(size + std::max(painter->pen().widthF(), 1.0)));
    imageBuffer.pen    = painter->pen  ();
    imageBuffer.brush  = painter->brush();
    imageBuffer.image  = CQChartsUtil::initImage(QSize(imageBuffer.isize, imageBuffer.isize));

    imageBuffer.image.fill(QColor(0,0,0,0));

    QPainter ipainter(&imageBuffer.image);

    ipainter.setRenderHints(QPainter::Antialiasing);

    ipainter.setPen  (imageBuffer.pen  );
    ipainter.setBrush(imageBuffer.brush);

    CQChartsPixelPainter device(&ipainter);

    CQChartsGeom::Point spos (size, size);
    CQChartsLength      ssize(size, CQChartsUnits::PIXEL);

    CQChartsDrawUtil::drawSymbol(&device, symbol, spos, ssize);
  }

  double is = imageBuffer.isize/2.0;

  painter->drawImage(int(p.x - is), int(p.y - is), imageBuffer.image);
}

CQChartsTextOptions
CQChartsPlot::
adjustTextOptions(const CQChartsTextOptions &options) const
{
  CQChartsTextOptions options1 = options;

  options1.minScaleFontSize = minScaleFontSize();
  options1.maxScaleFontSize = maxScaleFontSize();

  return options1;
}

//------

void
CQChartsPlot::
drawWindowColorBox(CQChartsPaintDevice *device, const CQChartsGeom::BBox &bbox,
                   const QColor &c) const
{
  auto *painter = dynamic_cast<CQChartsViewPlotPainter *>(device);
  if (! painter) return;

  if (! bbox.isSet())
    return;

  auto prect = windowToPixel(bbox);

  drawColorBox(painter, prect, c);
}

void
CQChartsPlot::
drawColorBox(CQChartsPaintDevice *device, const CQChartsGeom::BBox &bbox, const QColor &c) const
{
  auto *painter = dynamic_cast<CQChartsViewPlotPainter *>(device);
  if (! painter) return;

  painter->setPen(c);
  painter->setBrush(Qt::NoBrush);

  painter->drawRect(bbox);
}

//------

bool
CQChartsPlot::
isSetHidden(int id) const
{
  auto p = idHidden_.find(id);

  if (p == idHidden_.end())
    return false;

  return (*p).second;
}

void
CQChartsPlot::
setSetHidden(int id, bool hidden)
{
  idHidden_[id] = hidden;
}

void
CQChartsPlot::
resetSetHidden()
{
  idHidden_.clear();
}

void
CQChartsPlot::
update()
{
  assert(fromInvalidate_);

  view()->update();

  fromInvalidate_ = false;
}

//------

void
CQChartsPlot::
setPenBrush(CQChartsPenBrush &penBrush, const CQChartsPenData &penData,
            const CQChartsBrushData &brushData) const
{
  setPenBrush(penBrush,
    penData.isVisible(), penData.color(), penData.alpha(), penData.width(), penData.dash(),
    brushData.isVisible(), brushData.color(), brushData.alpha(), brushData.pattern());
}

void
CQChartsPlot::
setPenBrush(CQChartsPenBrush &penBrush,
            bool stroked, const QColor &strokeColor, const CQChartsAlpha &strokeAlpha,
            const CQChartsLength &strokeWidth, const CQChartsLineDash &strokeDash,
            bool filled, const QColor &fillColor, const CQChartsAlpha &fillAlpha,
            const CQChartsFillPattern &pattern) const
{
  setPen(penBrush.pen, stroked, strokeColor, strokeAlpha, strokeWidth, strokeDash);

  setBrush(penBrush.brush, filled, fillColor, fillAlpha, pattern);
}

void
CQChartsPlot::
setPen(CQChartsPenBrush &penBrush, const CQChartsPenData &penData) const
{
  setPen(penBrush.pen,
    penData.isVisible(), penData.color(), penData.alpha(), penData.width(), penData.dash());
}

void
CQChartsPlot::
setPen(QPen &pen, bool stroked, const QColor &strokeColor, const CQChartsAlpha &strokeAlpha,
       const CQChartsLength &strokeWidth, const CQChartsLineDash &strokeDash) const
{
  double width = lengthPixelWidth(strokeWidth);

  CQChartsUtil::setPen(pen, stroked, strokeColor, strokeAlpha, width, strokeDash);
}

void
CQChartsPlot::
setBrush(CQChartsPenBrush &penBrush, const CQChartsBrushData &brushData) const
{
  setBrush(penBrush.brush,
    brushData.isVisible(), brushData.color(), brushData.alpha(), brushData.pattern());
}

void
CQChartsPlot::
setBrush(QBrush &brush, bool filled, const QColor &fillColor, const CQChartsAlpha &fillAlpha,
         const CQChartsFillPattern &pattern) const
{
  CQChartsUtil::setBrush(brush, filled, fillColor, fillAlpha, pattern);
}

//------

void
CQChartsPlot::
updateObjPenBrushState(const CQChartsObj *obj, CQChartsPenBrush &penBrush,
                       DrawType drawType) const
{
  updateObjPenBrushState(obj, ColorInd(), penBrush, drawType);
}

void
CQChartsPlot::
updateObjPenBrushState(const CQChartsObj *obj, const ColorInd &ic,
                       CQChartsPenBrush &penBrush, DrawType drawType) const
{
  view()->updateObjPenBrushState(obj, ic, penBrush, drawType);
}

void
CQChartsPlot::
updateInsideObjPenBrushState(const ColorInd &ic, CQChartsPenBrush &penBrush,
                             bool outline, DrawType drawType) const
{
  view()->updateInsideObjPenBrushState(ic, penBrush, outline, drawType);
}

void
CQChartsPlot::
updateSelectedObjPenBrushState(const ColorInd &ic, CQChartsPenBrush &penBrush,
                               DrawType drawType) const
{
  view()->updateSelectedObjPenBrushState(ic, penBrush, drawType);
}

QColor
CQChartsPlot::
insideColor(const QColor &c) const
{
  return view()->insideColor(c);
}

QColor
CQChartsPlot::
selectedColor(const QColor &c) const
{
  return view()->selectedColor(c);
}

//------

QColor
CQChartsPlot::
interpPaletteColor(const ColorInd &ind, bool scale) const
{
  return (ind.isInt ?
    interpPaletteColorI(ind.i, ind.n, scale) : interpPaletteColorI(ind.r, scale));
}

QColor
CQChartsPlot::
interpPaletteColorI(int i, int n, bool scale) const
{
  return view()->interpPaletteColor(i, n, scale);
}

QColor
CQChartsPlot::
interpPaletteColorI(double r, bool scale) const
{
  return view()->interpPaletteColor(r, scale);
}

//---

QColor
CQChartsPlot::
interpGroupPaletteColor(const ColorInd &ig, const ColorInd &iv, bool scale) const
{
  return view()->interpGroupPaletteColor(ig, iv, scale);
}

QColor
CQChartsPlot::
interpGroupPaletteColorI(int ig, int ng, int i, int n, bool scale) const
{
  return view()->interpGroupPaletteColor(ig, ng, i, n, scale);
}

QColor
CQChartsPlot::
blendGroupPaletteColor(double r1, double r2, double dr) const
{
  CQColorsTheme *theme = view()->theme();

  // r1 is parent color and r2 is child color
  QColor c1 = theme->palette()->getColor(r1 - dr/2.0);
  QColor c2 = theme->palette()->getColor(r1 + dr/2.0);

  return CQChartsUtil::blendColors(c1, c2, r2);
}

QColor
CQChartsPlot::
interpThemeColor(const ColorInd &ind) const
{
  return view()->interpThemeColor(ind);
}

QColor
CQChartsPlot::
interpColor(const CQChartsColor &c, const ColorInd &ind) const
{
  if (! defaultPalette_.isValid())
    return view()->interpColor(c, ind);

  CQChartsColor c1 = charts()->adjustDefaultPalette(c, defaultPalette_.name());

  return view()->interpColor(c1, ind);
}

//---

QColor
CQChartsPlot::
calcTextColor(const QColor &bg) const
{
  return CQChartsUtil::bwColor(bg);
}

CQChartsPlot::ColorInd
CQChartsPlot::
calcColorInd(const CQChartsPlotObj *obj, const CQChartsKeyColorBox *keyBox,
             const ColorInd &is, const ColorInd &ig, const ColorInd &iv) const
{
  ColorInd colorInd;

  CQChartsPlot::ColorType colorType = this->colorType();

  if (obj && colorType == ColorType::AUTO)
    colorType = (CQChartsPlot::ColorType) obj->colorType();

  if      (colorType == ColorType::AUTO)
    colorInd = (is.n <= 1 ? (ig.n <= 1 ? iv : ig) : is);
  else if (colorType == ColorType::SET)
    colorInd = is;
  else if (colorType == ColorType::GROUP)
    colorInd = ig;
  else if (colorType == ColorType::INDEX)
    colorInd = iv;
  else if (colorType == ColorType::X_VALUE) {
    const CQChartsColorStops &stops = colorXStops();

    bool hasStops = stops.isValid();
    bool relative = (hasStops ? stops.isPercent() : true);

    double x = 0.0;

    if      (obj)
      x = obj->xColorValue(relative);
    else if (keyBox)
      x = keyBox->xColorValue(relative);

    if (hasStops) {
      int ind = stops.ind(x);

      colorInd = ColorInd(ind, stops.size() + 1);
    }
    else
      colorInd = ColorInd(x);
  }
  else if (colorType == ColorType::Y_VALUE) {
    const CQChartsColorStops &stops = colorYStops();

    bool hasStops = stops.isValid();
    bool relative = (hasStops ? stops.isPercent() : true);

    double y = 0.0;

    if      (obj)
      y = obj->yColorValue(relative);
    else if (keyBox)
      y = keyBox->yColorValue(relative);

    if (hasStops) {
      int ind = stops.ind(y);

      colorInd = ColorInd(ind, stops.size() + 1);
    }
    else
      colorInd = ColorInd(y);
  }

  return colorInd;
}

//------

bool
CQChartsPlot::
checkColumns(const CQChartsColumns &columns, const QString &name, bool required) const
{
  if (required) {
    if (! columns.isValid())
      return const_cast<CQChartsPlot *>(this)->
        addError(QString("Missing required %1 columns").arg(name));
  }

  bool valid = true;

  int iv = 0;

  for (const auto &column : columns) {
    if (columnValueType(column) == ColumnType::NONE)
      valid = const_cast<CQChartsPlot *>(this)->
        addColumnError(column, QString("Invalid %1 column (#%2)").arg(name).arg(iv));

    ++iv;
  }

  return valid;
}

bool
CQChartsPlot::
checkColumn(const CQChartsColumn &column, const QString &name, bool required) const
{
  ColumnType type;

  return checkColumn(column, name, type, required);
}

bool
CQChartsPlot::
checkColumn(const CQChartsColumn &column, const QString &name,
            ColumnType &type, bool required) const
{
  type = ColumnType::NONE;

  if (required) {
    if (! column.isValid())
      return const_cast<CQChartsPlot *>(this)->
        addColumnError(column, QString("Missing required %1 column").arg(name));
  }

  if (column.isValid()) {
    type = columnValueType(column);

    if (type == ColumnType::NONE)
      return const_cast<CQChartsPlot *>(this)->
        addColumnError(column, QString("Invalid %1 column").arg(name));
  }

  return true;
}

CQChartsPlot::ColumnType
CQChartsPlot::
columnValueType(const CQChartsColumn &column, const ColumnType &defType) const
{
  ColumnType         columnType;
  ColumnType         columnBaseType;
  CQChartsNameValues nameValues;

  if (! columnValueType(column, columnType, columnBaseType, nameValues, defType))
    return ColumnType::NONE;

  return columnType;
}

bool
CQChartsPlot::
columnValueType(const CQChartsColumn &column, ColumnType &columnType, ColumnType &columnBaseType,
                CQChartsNameValues &nameValues, const ColumnType &defType) const
{
  if (! column.isValid()) {
    columnType     = ColumnType::NONE;
    columnBaseType = ColumnType::NONE;
    return false;
  }

  CQChartsModelColumnDetails *columnDetails = this->columnDetails(column);

  if (columnDetails) {
    // if has details column is valid
    columnType     = columnDetails->type();
    columnBaseType = columnDetails->baseType();
    nameValues     = columnDetails->nameValues();

    if (columnType == ColumnType::NONE) {
      // if no column type then could not be calculated (still return true)
      columnType     = defType;
      columnBaseType = defType;
    }
  }
  else {
    QAbstractItemModel *model = this->model().data();
    assert(model);

    if (! CQChartsModelUtil::columnValueType(charts(), model, column, columnType,
                                             columnBaseType, nameValues)) {
      // if fail column is invalid
      columnType     = ColumnType::NONE;
      columnBaseType = ColumnType::NONE;
      return false;
    }
  }

  return true;
}

#if 0
bool
CQChartsPlot::
columnTypeStr(const CQChartsColumn &column, QString &typeStr) const
{
  QAbstractItemModel *model = this->model().data();
  assert(model);

  return CQChartsModelUtil::columnTypeStr(charts(), model, column, typeStr);
}

bool
CQChartsPlot::
setColumnTypeStr(const CQChartsColumn &column, const QString &typeStr)
{
  QAbstractItemModel *model = this->model().data();
  assert(model);

  return CQChartsModelUtil::setColumnTypeStr(charts(), model, column, typeStr);
}
#endif

bool
CQChartsPlot::
columnDetails(const CQChartsColumn &column, QString &typeName,
              QVariant &minValue, QVariant &maxValue) const
{
  if (! column.isValid())
    return false;

  CQChartsModelColumnDetails *details = this->columnDetails(column);
  if (! details) return false;

  typeName = details->typeName();
  minValue = details->minValue();
  maxValue = details->maxValue();

  return true;
}

CQChartsModelColumnDetails *
CQChartsPlot::
columnDetails(const CQChartsColumn &column) const
{
  CQChartsModelData *modelData = getModelData();
  if (! modelData) return nullptr;

  CQChartsModelDetails *details = modelData->details();
  if (! details) return nullptr;

  return details->columnDetails(column);
}

CQChartsModelData *
CQChartsPlot::
getModelData() const
{
  return charts()->getModelData(model_.data());
}

//------

bool
CQChartsPlot::
getHierColumnNames(const QModelIndex &parent, int row, const CQChartsColumns &nameColumns,
                   const QString &separator, QStringList &nameStrs, ModelIndices &nameInds) const
{
  QAbstractItemModel *model = this->model().data();
  assert(model);

  // single column (separated names)
  if (nameColumns.count() == 1) {
    const CQChartsColumn &nameColumn = nameColumns.column();

    //---

    CQChartsModelIndex nameModelInd(row, nameColumn, parent);

    bool ok;

    QString name = modelString(nameModelInd, ok);

    if (ok && ! name.simplified().length())
      ok = false;

    if (ok) {
      if (separator.length())
        nameStrs = name.split(separator, QString::SkipEmptyParts);
      else
        nameStrs << name;
    }

    QModelIndex nameInd = modelIndex(nameModelInd);

    nameInds.push_back(nameInd);
  }
  else {
    for (auto &nameColumn : nameColumns) {
      CQChartsModelIndex nameModelInd(row, nameColumn, parent);

      bool ok;

      QString name = modelString(nameModelInd, ok);

      if (ok && ! name.simplified().length())
        ok = false;

      if (ok) {
        nameStrs << name;

        QModelIndex nameInd = modelIndex(nameModelInd);

        nameInds.push_back(nameInd);
      }
    }
  }

  return nameStrs.length();
}

//------

CQChartsModelIndex
CQChartsPlot::
normalizeIndex(const CQChartsModelIndex &ind) const
{
  QModelIndex ind1 = normalizeIndex(modelIndex(ind));

  if (ind1.column() == ind.column.column())
    return CQChartsModelIndex(ind1.row(), ind.column, ind1.parent());
  else
    return CQChartsModelIndex(ind1.row(), CQChartsColumn(ind1.column()), ind1.parent());
}

QModelIndex
CQChartsPlot::
normalizeIndex(const QModelIndex &ind) const
{
  // map index in proxy model, to source model (non-proxy model)
  QAbstractItemModel *model = this->model().data();
  assert(model);

  std::vector<QSortFilterProxyModel *> proxyModels;
  QAbstractItemModel*                  sourceModel;

  this->proxyModels(proxyModels, sourceModel);

  QModelIndex ind1 = ind;

  int i = 0;

  // ind model should match first proxy model
  for ( ; i < int(proxyModels.size()); ++i)
    if (ind1.model() == proxyModels[i])
      break;

  for ( ; i < int(proxyModels.size()); ++i)
    ind1 = proxyModels[i]->mapToSource(ind1);

  return ind1;
}

QModelIndex
CQChartsPlot::
unnormalizeIndex(const QModelIndex &ind) const
{
  // map index in source model (non-proxy model), to proxy model
  QAbstractItemModel *model = this->model().data();
  assert(model);

  std::vector<QSortFilterProxyModel *> proxyModels;
  QAbstractItemModel*                  sourceModel;

  this->proxyModels(proxyModels, sourceModel);

  QModelIndex ind1 = ind;

  // ind model should match source model of last proxy model
  int i = int(proxyModels.size()) - 1;

  for ( ; i >= 0; --i)
    if (ind1.model() == proxyModels[i]->sourceModel())
      break;

  for ( ; i >= 0; --i)
    ind1 = proxyModels[i]->mapFromSource(ind1);

  return ind1;
}

void
CQChartsPlot::
proxyModels(std::vector<QSortFilterProxyModel *> &proxyModels,
            QAbstractItemModel* &sourceModel) const
{
  // map index in source model (non-proxy model), to proxy model
  QAbstractItemModel *model = this->model().data();
  assert(model);

  auto *proxyModel = qobject_cast<QSortFilterProxyModel *>(model);

  if (proxyModel) {
    while (proxyModel) {
      proxyModels.push_back(proxyModel);

      sourceModel = proxyModel->sourceModel();

      proxyModel = qobject_cast<QSortFilterProxyModel *>(sourceModel);
    }
  }
  else
    sourceModel = model;
}

bool
CQChartsPlot::
isHierarchical() const
{
  QAbstractItemModel *model = this->model().data();

  return CQChartsModelUtil::isHierarchical(model);
}

//------

void
CQChartsPlot::
addColumnValues(const CQChartsColumn &column, CQChartsValueSet &valueSet) const
{
  class ValueSetVisitor : public ModelVisitor {
   public:
    ValueSetVisitor(const CQChartsPlot *plot, const CQChartsColumn &column,
                    CQChartsValueSet &valueSet) :
     plot_(plot), column_(column), valueSet_(valueSet) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      CQChartsModelIndex columnInd(data.row, column_, data.parent);

      bool ok;

      QVariant value = plot_->modelValue(columnInd, ok);

      // TODO: skip if not ok ?

      valueSet_.addValue(value);

      return State::OK;
    }

   private:
    const CQChartsPlot* plot_   { nullptr };
    CQChartsColumn      column_;
    CQChartsValueSet&   valueSet_;
  };

  ValueSetVisitor valueSetVisitor(this, column, valueSet);

  visitModel(valueSetVisitor);
}

//------

void
CQChartsPlot::
visitModel(ModelVisitor &visitor) const
{
  CQPerfTrace trace("CQChartsPlot::visitModel");

  visitor.setPlot(this);

  visitor.init();

  //if (isPreview())
  //  visitor.setMaxRows(previewMaxRows());

  (void) CQChartsModelVisit::exec(charts(), model().data(), visitor);
}

//------

bool
CQChartsPlot::
modelMappedReal(int row, const CQChartsColumn &column, const QModelIndex &parent,
                double &r, bool log, double def) const
{
  return modelMappedReal(CQChartsModelIndex(row, column, parent), r, log, def);
}

bool
CQChartsPlot::
modelMappedReal(const CQChartsModelIndex &ind, double &r, bool log, double def) const
{
  bool ok = false;

  if (ind.isValid()) {
    r = modelReal(ind, ok);

    if (! ok)
      r = def;

    if (CMathUtil::isNaN(r) || CMathUtil::isInf(r))
      return false;
  }
  else
    r = def;

  if (log) {
    if (r <= 0)
      return false;

    r = logValue(r);
  }

  return ok;
}

//------

int
CQChartsPlot::
getRowForId(const QString &id) const
{
  if (! idColumn().isValid())
    return -1;

  // process model data
  class IdVisitor : public ModelVisitor {
   public:
    IdVisitor(const CQChartsPlot *plot, const QString &id) :
     plot_(plot), id_(id) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      bool ok;

      QString id = plot_->idColumnString(data.row, data.parent, ok);

      if (ok && id == id_)
        row_ = data.row;

      return State::OK;
    }

    int row() const { return row_; }

   private:
    const CQChartsPlot *plot_ { nullptr };
    QString             id_;
    int                 row_ { -1 };
  };

  IdVisitor idVisitor(this, id);

  visitModel(idVisitor);

  return idVisitor.row();
}

QString
CQChartsPlot::
idColumnString(int row, const QModelIndex &parent, bool &ok) const
{
  ok = false;

  if (! idColumn().isValid())
    return "";

  CQChartsModelIndex idColumnInd(row, idColumn(), parent);

  QVariant var = modelValue(idColumnInd, ok);

  if (! ok)
    return "";

  QString str;

  double r;

  if (CQChartsVariant::toReal(var, r))
    str = columnStr(idColumn(), r);
  else {
    bool ok;

    str = CQChartsVariant::toString(var, ok);
  }

  return str;
}

//------

QModelIndex
CQChartsPlot::
modelIndex(const CQChartsModelIndex &ind) const
{
  return modelIndex(ind.row, ind.column, ind.parent);
}

QModelIndex
CQChartsPlot::
modelIndex(int row, const CQChartsColumn &column, const QModelIndex &parent) const
{
  if (! column.hasColumn())
    return QModelIndex();

  QAbstractItemModel *model = this->model().data();
  if (! model) return QModelIndex();

  return model->index(row, column.column(), parent);
}

//------

#if 0
QVariant
CQChartsPlot::
modelHHeaderValue(const CQChartsColumn &column, bool &ok) const
{
  return modelHHeaderValue(model().data(), column, ok);
}

QVariant
CQChartsPlot::
modelHHeaderValue(const CQChartsColumn &column, int role, bool &ok) const
{
  return modelHHeaderValue(model().data(), column, role, ok);
}

QVariant
CQChartsPlot::
modelHHeaderValue(QAbstractItemModel *model, const CQChartsColumn &column, bool &ok) const
{
  return CQChartsModelUtil::modelHeaderValue(model, column, ok);
}

QVariant
CQChartsPlot::
modelHHeaderValue(QAbstractItemModel *model, const CQChartsColumn &column,
                  int role, bool &ok) const
{
  return CQChartsModelUtil::modelHeaderValue(model, column, role, ok);
}
#endif

//--

#if 0
QVariant
CQChartsPlot::
modelVHeaderValue(int section, Qt::Orientation orient, int role, bool &ok) const
{
  return modelVHeaderValue(model().data(), section, orient, role, ok);
}

QVariant
CQChartsPlot::
modelVHeaderValue(int section, Qt::Orientation orient, bool &ok) const
{
  return modelVHeaderValue(model().data(), section, orient, Qt::DisplayRole, ok);
}

QVariant
CQChartsPlot::
modelVHeaderValue(QAbstractItemModel *model, int section, Qt::Orientation orientation,
                  bool &ok) const
{
  return CQChartsModelUtil::modelHeaderValue(model, section, orientation, ok);
}

QVariant
CQChartsPlot::
modelVHeaderValue(QAbstractItemModel *model, int section, Qt::Orientation orientation,
                  int role, bool &ok) const
{
  return CQChartsModelUtil::modelHeaderValue(model, section, orientation, role, ok);
}
#endif

//--

QString
CQChartsPlot::
modelHHeaderString(const CQChartsColumn &column, bool &ok) const
{
  return modelHHeaderString(model().data(), column, ok);
}

QString
CQChartsPlot::
modelHHeaderString(const CQChartsColumn &column, int role, bool &ok) const
{
  return modelHHeaderString(model().data(), column, role, ok);
}

QString
CQChartsPlot::
modelHHeaderString(QAbstractItemModel *model, const CQChartsColumn &column, bool &ok) const
{
  return CQChartsModelUtil::modelHeaderString(model, column, ok);
}

QString
CQChartsPlot::
modelHHeaderString(QAbstractItemModel *model, const CQChartsColumn &column,
                   int role, bool &ok) const
{
  return CQChartsModelUtil::modelHeaderString(model, column, role, ok);
}

//--

QString
CQChartsPlot::
modelVHeaderString(int section, Qt::Orientation orient, int role, bool &ok) const
{
  return modelVHeaderString(model().data(), section, orient, role, ok);
}

QString
CQChartsPlot::
modelVHeaderString(int section, Qt::Orientation orient, bool &ok) const
{
  return modelVHeaderString(model().data(), section, orient, Qt::DisplayRole, ok);
}

QString
CQChartsPlot::
modelVHeaderString(QAbstractItemModel *model, int section, Qt::Orientation orientation,
                   int role, bool &ok) const
{
  return CQChartsModelUtil::modelHeaderString(model, section, orientation, role, ok);
}

QString
CQChartsPlot::
modelVHeaderString(QAbstractItemModel *model, int section, Qt::Orientation orientation,
                   bool &ok) const
{
  return CQChartsModelUtil::modelHeaderString(model, section, orientation, ok);
}

//------

QVariant
CQChartsPlot::
modelValue(int row, const CQChartsColumn &column, const QModelIndex &parent,
           int role, bool &ok) const
{
  return modelValue(CQChartsModelIndex(row, column, parent), role, ok);
}

QVariant
CQChartsPlot::
modelValue(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  return modelValue(CQChartsModelIndex(row, column, parent), ok);
}

QVariant
CQChartsPlot::
modelValue(const CQChartsModelIndex &ind, int role, bool &ok) const
{
  return modelValue(model().data(), ind.row, ind.column, ind.parent, role, ok);
}

QVariant
CQChartsPlot::
modelValue(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelValue(model().data(), ind.row, ind.column, ind.parent, ok);
}

QVariant
CQChartsPlot::
modelValue(QAbstractItemModel *model, int row, const CQChartsColumn &column,
           const QModelIndex &parent, int role, bool &ok) const
{
  return CQChartsModelUtil::modelValue(charts(), model, row, column, parent, role, ok);
}

QVariant
CQChartsPlot::
modelValue(QAbstractItemModel *model, int row, const CQChartsColumn &column,
           const QModelIndex &parent, bool &ok) const
{
  return CQChartsModelUtil::modelValue(charts(), model, row, column, parent, ok);
}

//---

QString
CQChartsPlot::
modelString(int row, const CQChartsColumn &column, const QModelIndex &parent,
            int role, bool &ok) const
{
  return modelString(CQChartsModelIndex(row, column, parent), role, ok);
}

QString
CQChartsPlot::
modelString(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  return modelString(CQChartsModelIndex(row, column, parent), ok);
}

QString
CQChartsPlot::
modelString(const CQChartsModelIndex &ind, int role, bool &ok) const
{
  return modelString(model().data(), ind.row, ind.column, ind.parent, role, ok);
}

QString
CQChartsPlot::
modelString(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelString(model().data(), ind.row, ind.column, ind.parent, ok);
}

QString
CQChartsPlot::
modelString(QAbstractItemModel *model, int row, const CQChartsColumn &column,
            const QModelIndex &parent, int role, bool &ok) const
{
  return CQChartsModelUtil::modelString(charts(), model, row, column, parent, role, ok);
}

QString
CQChartsPlot::
modelString(QAbstractItemModel *model, int row, const CQChartsColumn &column,
            const QModelIndex &parent, bool &ok) const
{
  return CQChartsModelUtil::modelString(charts(), model, row, column, parent, ok);
}

//---

double
CQChartsPlot::
modelReal(int row, const CQChartsColumn &column, const QModelIndex &parent,
          int role, bool &ok) const
{
  return modelReal(CQChartsModelIndex(row, column, parent), role, ok);
}

double
CQChartsPlot::
modelReal(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  return modelReal(CQChartsModelIndex(row, column, parent), ok);
}

double
CQChartsPlot::
modelReal(const CQChartsModelIndex &ind, int role, bool &ok) const
{
  return modelReal(model().data(), ind.row, ind.column, ind.parent, role, ok);
}

double
CQChartsPlot::
modelReal(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelReal(model().data(), ind.row, ind.column, ind.parent, ok);
}

double
CQChartsPlot::
modelReal(QAbstractItemModel *model, int row, const CQChartsColumn &column,
          const QModelIndex &parent, int role, bool &ok) const
{
  return CQChartsModelUtil::modelReal(charts(), model, row, column, parent, role, ok);
}

double
CQChartsPlot::
modelReal(QAbstractItemModel *model, int row, const CQChartsColumn &column,
          const QModelIndex &parent, bool &ok) const
{
  return CQChartsModelUtil::modelReal(charts(), model, row, column, parent, ok);
}

//--

long
CQChartsPlot::
modelInteger(int row, const CQChartsColumn &column, const QModelIndex &parent,
             int role, bool &ok) const
{
  return modelInteger(CQChartsModelIndex(row, column, parent), role, ok);
}

long
CQChartsPlot::
modelInteger(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  return modelInteger(CQChartsModelIndex(row, column, parent), ok);
}

long
CQChartsPlot::
modelInteger(const CQChartsModelIndex &ind, int role, bool &ok) const
{
  return modelInteger(model().data(), ind.row, ind.column, ind.parent, role, ok);
}

long
CQChartsPlot::
modelInteger(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelInteger(model().data(), ind.row, ind.column, ind.parent, ok);
}

long
CQChartsPlot::
modelInteger(QAbstractItemModel *model, int row, const CQChartsColumn &column,
             const QModelIndex &parent, int role, bool &ok) const
{
  return CQChartsModelUtil::modelInteger(charts(), model, row, column, parent, role, ok);
}

long
CQChartsPlot::
modelInteger(QAbstractItemModel *model, int row, const CQChartsColumn &column,
             const QModelIndex &parent, bool &ok) const
{
  return CQChartsModelUtil::modelInteger(charts(), model, row, column, parent, ok);
}

//--

#if 0
CQChartsColor
CQChartsPlot::
modelColor(int row, const CQChartsColumn &column, const QModelIndex &parent,
           int role, bool &ok) const
{
  return modelColor(model().data(), row, column, parent, role, ok);
}

CQChartsColor
CQChartsPlot::
modelColor(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  return modelColor(model().data(), row, column, parent, ok);
}

CQChartsColor
CQChartsPlot::
modelColor(QAbstractItemModel *model, int row, const CQChartsColumn &column,
           const QModelIndex &parent, int role, bool &ok) const
{
  return CQChartsModelUtil::modelColor(charts(), model, row, column, parent, role, ok);
}

CQChartsColor
CQChartsPlot::
modelColor(QAbstractItemModel *model, int row, const CQChartsColumn &column,
           const QModelIndex &parent, bool &ok) const
{
  return CQChartsModelUtil::modelColor(charts(), model, row, column, parent, ok);
}
#endif

//---

std::vector<double>
CQChartsPlot::
modelReals(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelReals(ind.row, ind.column, ind.parent, ok);
}

std::vector<double>
CQChartsPlot::
modelReals(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  std::vector<double> reals;

  QVariant var = modelValue(model().data(), row, column, parent, ok);

  if (! ok)
    return reals;

  return CQChartsVariant::toReals(var, ok);
}

//------

QVariant
CQChartsPlot::
modelRootValue(int row, const CQChartsColumn &column, const QModelIndex &parent,
               int role, bool &ok) const
{
  if (column.column() == 0 && parent.isValid())
    return modelRootValue(parent.row(), column, parent.parent(), role, ok);

  return modelHierValue(row, column, parent, role, ok);
}

QVariant
CQChartsPlot::
modelRootValue(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  if (column.column() == 0 && parent.isValid())
    return modelRootValue(parent.row(), column, parent.parent(), ok);

  return modelHierValue(row, column, parent, ok);
}

//------

QVariant
CQChartsPlot::
modelHierValue(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelHierValue(ind.row, ind.column, ind.parent, ok);
}

QVariant
CQChartsPlot::
modelHierValue(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  QVariant v = modelValue(row, column, parent, ok);

  if (! ok && column.column() == 0 && parent.isValid()) {
    QModelIndex parent1 = parent;
    int         row1    = row;

    while (! ok && parent1.isValid()) {
      row1    = parent1.row();
      parent1 = parent1.parent();

      v = modelValue(row1, column, parent1, ok);
    }
  }

  return v;
}

QVariant
CQChartsPlot::
modelHierValue(int row, const CQChartsColumn &column,
               const QModelIndex &parent, int role, bool &ok) const
{
  QVariant v = modelValue(row, column, parent, role, ok);

  if (! ok && column.column() == 0 && parent.isValid()) {
    QModelIndex parent1 = parent;
    int         row1    = row;

    while (! ok && parent1.isValid()) {
      row1    = parent1.row();
      parent1 = parent1.parent();

      v = modelValue(row1, column, parent1, role, ok);
    }
  }

  return v;
}

//--

QString
CQChartsPlot::
modelHierString(const CQChartsModelIndex &ind, bool &ok) const
{
  return modelHierString(ind.row, ind.column, ind.parent, ok);
}

QString
CQChartsPlot::
modelHierString(int row, const CQChartsColumn &column,
                const QModelIndex &parent, bool &ok) const
{
  QVariant var = modelHierValue(row, column, parent, ok);

  if (! ok)
    return QString();

  QString str;

  bool rc = CQChartsVariant::toString(var, str);
  assert(rc);

  return str;
}

QString
CQChartsPlot::
modelHierString(int row, const CQChartsColumn &column,
                const QModelIndex &parent, int role, bool &ok) const
{
  QVariant var = modelHierValue(row, column, parent, role, ok);

  if (! ok)
    return QString();

  QString str;

  bool rc = CQChartsVariant::toString(var, str);
  assert(rc);

  return str;
}

//--

#if 0
double
CQChartsPlot::
modelHierReal(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  QVariant var = modelHierValue(row, column, parent, ok);

  if (! ok)
    return 0.0;

  return CQChartsVariant::toReal(var, ok);
}

double
CQChartsPlot::
modelHierReal(int row, const CQChartsColumn &column,
              const QModelIndex &parent, int role, bool &ok) const
{
  QVariant var = modelHierValue(row, column, parent, role, ok);

  if (! ok)
    return 0.0;

  return CQChartsVariant::toReal(var, ok);
}
#endif

//--

#if 0
long
CQChartsPlot::
modelHierInteger(int row, const CQChartsColumn &column, const QModelIndex &parent, bool &ok) const
{
  QVariant var = modelHierValue(row, column, parent, ok);

  if (! ok)
    return 0;

  return CQChartsVariant::toInt(var, ok);
}

long
CQChartsPlot::
modelHierInteger(int row, const CQChartsColumn &column,
                 const QModelIndex &parent, int role, bool &ok) const
{
  QVariant var = modelHierValue(row, column, parent, role, ok);

  if (! ok)
    return 0;

  return CQChartsVariant::toInt(var, ok);
}
#endif

//------

bool
CQChartsPlot::
isSelectIndex(const QModelIndex &ind, int row, const CQChartsColumn &column,
              const QModelIndex &parent) const
{
  if (column.type() != CQChartsColumn::Type::DATA &&
      column.type() != CQChartsColumn::Type::DATA_INDEX)
    return false;

  return (ind == selectIndex(row, column, parent));
}

QModelIndex
CQChartsPlot::
selectIndex(int row, const CQChartsColumn &column, const QModelIndex &parent) const
{
  if (column.type() != CQChartsColumn::Type::DATA &&
      column.type() != CQChartsColumn::Type::DATA_INDEX)
    return QModelIndex();

  std::vector<QSortFilterProxyModel *> proxyModels;
  QAbstractItemModel*                  sourceModel;

  this->proxyModels(proxyModels, sourceModel);

  return sourceModel->index(row, column.column(), parent);
}

void
CQChartsPlot::
beginSelectIndex()
{
  selIndexColumnRows_.clear();
}

void
CQChartsPlot::
addSelectIndex(const CQChartsModelIndex &ind)
{
  addSelectIndex(ind.row, ind.column.column(), ind.parent);
}

void
CQChartsPlot::
addSelectIndex(int row, int column, const QModelIndex &parent)
{
  addSelectIndex(selectIndex(row, CQChartsColumn(column), parent));
}

void
CQChartsPlot::
addSelectIndex(const QModelIndex &ind)
{
  if (! ind.isValid())
    return;

  QModelIndex ind1 = unnormalizeIndex(ind);

  if (! ind1.isValid())
    return;

  // add to map ordered by parent, column, row
  selIndexColumnRows_[ind1.parent()][ind1.column()].insert(ind1.row());
}

void
CQChartsPlot::
endSelectIndex()
{
//QAbstractItemModel *model = this->model().data();
//assert(model);

  //---

  // build new selection
  QItemSelection optItemSelection;

  // build row range per index column
  for (const auto &p : selIndexColumnRows_) {
    const QModelIndex &parent     = p.first;
    const ColumnRows  &columnRows = p.second;

    // build row range per column
    for (const auto &p1 : columnRows) {
      int         ic   = p1.first;
      const Rows &rows = p1.second;

      CQChartsColumn column(ic);

      int startRow = -1;
      int endRow   = -1;

      for (const auto &row : rows) {
        if      (startRow < 0) {
          startRow = row;
          endRow   = row;
        }
        else if (row == endRow + 1) {
          endRow = row;
        }
        else {
          QModelIndex ind1 = modelIndex(startRow, column, parent);
          QModelIndex ind2 = modelIndex(endRow  , column, parent);

          optItemSelection.select(ind1, ind2);

          startRow = row;
          endRow   = row;
        }
      }

      if (startRow >= 0) {
        QModelIndex ind1 = modelIndex(startRow, column, parent);
        QModelIndex ind2 = modelIndex(endRow  , column, parent);

        optItemSelection.select(ind1, ind2);
      }
    }
  }

  //---

  if (optItemSelection.length()) {
    CQChartsModelData *modelData = getModelData();

    if (modelData)
      modelData->select(optItemSelection);
  }
}

//------

double
CQChartsPlot::
logValue(double x, int base) const
{
  if (x >= 1E-6)
    return std::log(x)/std::log(base);
  else
    return CMathUtil::getNaN();
}

double
CQChartsPlot::
expValue(double x, int base) const
{
  if (x <= 709.78271289)
    return std::exp(x*std::log(base));
  else
    return CMathUtil::getNaN();
}

//------

CQChartsGeom::Point
CQChartsPlot::
positionToPlot(const CQChartsPosition &pos) const
{
  auto p = CQChartsGeom::Point(pos.p());

  auto p1 = p;

  if      (pos.units() == CQChartsUnits::PIXEL)
    p1 = pixelToWindow(p);
  else if (pos.units() == CQChartsUnits::PLOT)
    p1 = p;
  else if (pos.units() == CQChartsUnits::VIEW)
    p1 = pixelToWindow(view()->windowToPixel(p));
  else if (pos.units() == CQChartsUnits::PERCENT) {
    auto pbbox = displayRangeBBox();

    p1.setX(p.getX()*pbbox.getWidth ()/100.0);
    p1.setY(p.getY()*pbbox.getHeight()/100.0);
  }

  return p1;
}

CQChartsGeom::Point
CQChartsPlot::
positionToPixel(const CQChartsPosition &pos) const
{
  auto p = CQChartsGeom::Point(pos.p());

  auto p1 = p;

  if      (pos.units() == CQChartsUnits::PIXEL)
    p1 = p;
  else if (pos.units() == CQChartsUnits::PLOT)
    p1 = windowToPixel(p);
  else if (pos.units() == CQChartsUnits::VIEW)
    p1 = view()->windowToPixel(p);
  else if (pos.units() == CQChartsUnits::PERCENT) {
    auto pbbox = calcPlotPixelRect();

    p1.setX(p.getX()*pbbox.getWidth ()/100.0);
    p1.setY(p.getY()*pbbox.getHeight()/100.0);
  }

  return p1;
}

//------

CQChartsGeom::BBox
CQChartsPlot::
rectToPlot(const CQChartsRect &rect) const
{
  auto r  = rect.bbox();
  auto r1 = r;

  if      (rect.units() == CQChartsUnits::PIXEL)
    r1 = pixelToWindow(r);
  else if (rect.units() == CQChartsUnits::PLOT)
    r1 = r;
  else if (rect.units() == CQChartsUnits::VIEW)
    r1 = pixelToWindow(view()->windowToPixel(r));
  else if (rect.units() == CQChartsUnits::PERCENT) {
    auto pbbox = displayRangeBBox();

    r1.setXMin(r.getXMin()*pbbox.getWidth ()/100.0);
    r1.setYMin(r.getYMin()*pbbox.getHeight()/100.0);
    r1.setXMax(r.getXMax()*pbbox.getWidth ()/100.0);
    r1.setYMax(r.getYMax()*pbbox.getHeight()/100.0);
  }

  return r1;
}

CQChartsGeom::BBox
CQChartsPlot::
rectToPixel(const CQChartsRect &rect) const
{
  auto r  = rect.bbox();
  auto r1 = r;

  if      (rect.units() == CQChartsUnits::PIXEL)
    r1 = r;
  else if (rect.units() == CQChartsUnits::PLOT)
    r1 = windowToPixel(r);
  else if (rect.units() == CQChartsUnits::VIEW)
    r1 = view()->windowToPixel(r);
  else if (rect.units() == CQChartsUnits::PERCENT) {
    auto pbbox = calcPlotPixelRect();

    r1.setXMin(r.getXMin()*pbbox.getWidth ()/100.0);
    r1.setYMin(r.getYMin()*pbbox.getHeight()/100.0);
    r1.setXMax(r.getXMax()*pbbox.getWidth ()/100.0);
    r1.setYMax(r.getYMax()*pbbox.getHeight()/100.0);
  }

  return r1;
}

//------

double
CQChartsPlot::
lengthPlotSize(const CQChartsLength &len, bool horizontal) const
{
  return (horizontal ? lengthPlotWidth(len) : lengthPlotHeight(len));
}

double
CQChartsPlot::
lengthPlotWidth(const CQChartsLength &len) const
{
  if      (len.units() == CQChartsUnits::PIXEL)
    return pixelToWindowWidth(len.value());
  else if (len.units() == CQChartsUnits::PLOT)
    return len.value();
  else if (len.units() == CQChartsUnits::VIEW)
    return pixelToWindowWidth(view()->windowToPixelWidth(len.value()));
  else if (len.units() == CQChartsUnits::PERCENT)
    return len.value()*displayRangeBBox().getWidth()/100.0;
  else
    return len.value();
}

double
CQChartsPlot::
lengthPlotHeight(const CQChartsLength &len) const
{
  if      (len.units() == CQChartsUnits::PIXEL)
    return pixelToWindowHeight(len.value());
  else if (len.units() == CQChartsUnits::PLOT)
    return len.value();
  else if (len.units() == CQChartsUnits::VIEW)
    return pixelToWindowHeight(view()->windowToPixelHeight(len.value()));
  else if (len.units() == CQChartsUnits::PERCENT)
    return len.value()*displayRangeBBox().getHeight()/100.0;
  else
    return len.value();
}

double
CQChartsPlot::
lengthPixelSize(const CQChartsLength &len, bool horizontal) const
{
  return (horizontal ? lengthPixelWidth(len) : lengthPixelHeight(len));
}

double
CQChartsPlot::
lengthPixelWidth(const CQChartsLength &len) const
{
  if      (len.units() == CQChartsUnits::PIXEL)
    return len.value();
  else if (len.units() == CQChartsUnits::PLOT)
    return windowToPixelWidth(len.value());
  else if (len.units() == CQChartsUnits::VIEW)
    return view()->windowToPixelWidth(len.value());
  else if (len.units() == CQChartsUnits::PERCENT)
    return len.value()*calcPlotPixelRect().getWidth()/100.0;
  else
    return len.value();
}

double
CQChartsPlot::
lengthPixelHeight(const CQChartsLength &len) const
{
  if      (len.units() == CQChartsUnits::PIXEL)
    return len.value();
  else if (len.units() == CQChartsUnits::PLOT)
    return windowToPixelHeight(len.value());
  else if (len.units() == CQChartsUnits::VIEW)
    return view()->windowToPixelHeight(len.value());
  else if (len.units() == CQChartsUnits::PERCENT)
    return len.value()*calcPlotPixelRect().getHeight()/100.0;
  else
    return len.value();
}

//------

void
CQChartsPlot::
windowToPixelI(double wx, double wy, double &px, double &py) const
{
  double vx, vy;

  windowToViewI(wx, wy, vx, vy);

  auto p = view()->windowToPixel(CQChartsGeom::Point(vx, vy));

  px = p.x;
  py = p.y;
}

double
CQChartsPlot::
windowToViewWidth(double wx) const
{
  double vx1, vy1, vx2, vy2;

  windowToViewI(0.0, 0.0, vx1, vy1);
  windowToViewI(wx , wx , vx2, vy2);

  return fabs(vx2 - vx1);
}

double
CQChartsPlot::
windowToViewHeight(double wy) const
{
  double vx1, vy1, vx2, vy2;

  windowToViewI(0.0, 0.0, vx1, vy1);
  windowToViewI(wy , wy , vx2, vy2);

  return fabs(vy2 - vy1);
}

void
CQChartsPlot::
windowToViewI(double wx, double wy, double &vx, double &vy) const
{
  displayRange().windowToPixel(wx, wy, &vx, &vy);

  if (isInvertX() || isInvertY()) {
    double ivx, ivy;

    displayRange().invertPixel(vx, vy, ivx, ivy);

    if (isInvertX()) vx = ivx;
    if (isInvertY()) vy = ivy;
  }
}

void
CQChartsPlot::
pixelToWindowI(double px, double py, double &wx, double &wy) const
{
  auto pv = view()->pixelToWindow(CQChartsGeom::Point(px, py));

  viewToWindowI(pv.x, pv.y, wx, wy);
}

void
CQChartsPlot::
viewToWindowI(double vx, double vy, double &wx, double &wy) const
{
  if (isInvertX() || isInvertY()) {
    double ivx, ivy;

    displayRange().invertPixel(vx, vy, ivx, ivy);

    if (isInvertX()) vx = ivx;
    if (isInvertY()) vy = ivy;
  }

  displayRange().pixelToWindow(vx, vy, &wx, &wy);
}

double
CQChartsPlot::
viewToWindowWidth(double vx) const
{
  double wx1, wy1, wx2, wy2;

  viewToWindowI(0.0, 0.0, wx1, wy1);
  viewToWindowI(vx , vx , wx2, wy2);

  return fabs(wx2 - wx1);
}

double
CQChartsPlot::
viewToWindowHeight(double vy) const
{
  double wx1, wy1, wx2, wy2;

  viewToWindowI(0.0, 0.0, wx1, wy1);
  viewToWindowI(vy , vy , wx2, wy2);

  return fabs(wy2 - wy1);
}

void
CQChartsPlot::
windowToPixelI(const CQChartsGeom::Point &w, CQChartsGeom::Point &p) const
{
  windowToPixelI(w.x, w.y, p.x, p.y);
}

void
CQChartsPlot::
pixelToWindowI(const CQChartsGeom::Point &p, CQChartsGeom::Point &w) const
{
  pixelToWindowI(p.x, p.y, w.x, w.y);
}

CQChartsGeom::Point
CQChartsPlot::
windowToPixel(const CQChartsGeom::Point &w) const
{
  CQChartsGeom::Point p;

  windowToPixelI(w.x, w.y, p.x, p.y);

  return p;
}

CQChartsGeom::Point
CQChartsPlot::
windowToView(const CQChartsGeom::Point &w) const
{
  double vx, vy;

  windowToViewI(w.x, w.y, vx, vy);

  return CQChartsGeom::Point(vx, vy);
}

CQChartsGeom::Point
CQChartsPlot::
pixelToWindow(const CQChartsGeom::Point &w) const
{
  CQChartsGeom::Point p;

  pixelToWindowI(w.x, w.y, p.x, p.y);

  return p;
}

CQChartsGeom::Point
CQChartsPlot::
viewToWindow(const CQChartsGeom::Point &v) const
{
  double wx, wy;

  viewToWindowI(v.x, v.y, wx, wy);

  return CQChartsGeom::Point(wx, wy);
}

void
CQChartsPlot::
windowToPixelI(const CQChartsGeom::BBox &wrect, CQChartsGeom::BBox &prect) const
{
  prect = windowToPixel(wrect);
}

void
CQChartsPlot::
pixelToWindowI(const CQChartsGeom::BBox &prect, CQChartsGeom::BBox &wrect) const
{
  wrect = pixelToWindow(prect);
}

CQChartsGeom::BBox
CQChartsPlot::
windowToPixel(const CQChartsGeom::BBox &wrect) const
{
  double px1, py1, px2, py2;

  windowToPixelI(wrect.getXMin(), wrect.getYMin(), px1, py2);
  windowToPixelI(wrect.getXMax(), wrect.getYMax(), px2, py1);

  return CQChartsGeom::BBox(px1, py1, px2, py2);
}

CQChartsGeom::BBox
CQChartsPlot::
pixelToWindow(const CQChartsGeom::BBox &wrect) const
{
  double wx1, wy1, wx2, wy2;

  pixelToWindowI(wrect.getXMin(), wrect.getYMin(), wx1, wy1);
  pixelToWindowI(wrect.getXMax(), wrect.getYMax(), wx2, wy2);

  return CQChartsGeom::BBox(wx1, wy1, wx2, wy2);
}

CQChartsGeom::BBox
CQChartsPlot::
windowToView(const CQChartsGeom::BBox &wrect) const
{
  double vx1, vy1, vx2, vy2;

  windowToViewI(wrect.getXMin(), wrect.getYMin(), vx1, vy1);
  windowToViewI(wrect.getXMax(), wrect.getYMax(), vx2, vy2);

  return CQChartsGeom::BBox(vx1, vy1, vx2, vy2);
}

CQChartsGeom::BBox
CQChartsPlot::
viewToWindow(const CQChartsGeom::BBox &vrect) const
{
  double wx1, wy1, wx2, wy2;

  viewToWindowI(vrect.getXMin(), vrect.getYMin(), wx1, wy1);
  viewToWindowI(vrect.getXMax(), vrect.getYMax(), wx2, wy2);

  return CQChartsGeom::BBox(wx1, wy1, wx2, wy2);
}

double
CQChartsPlot::
pixelToSignedWindowWidth(double ww) const
{
  return CMathUtil::sign(ww)*pixelToWindowWidth(ww);
}

double
CQChartsPlot::
pixelToSignedWindowHeight(double wh) const
{
  return CMathUtil::sign(wh)*pixelToWindowHeight(wh);
}

double
CQChartsPlot::
pixelToWindowSize(double ps, bool horizontal) const
{
  return (horizontal ? pixelToWindowWidth(ps) : pixelToWindowHeight(ps));
}

CQChartsGeom::Size
CQChartsPlot::
pixelToWindowSize(const CQChartsGeom::Size &ps) const
{
  double w = pixelToWindowWidth (ps.width ());
  double h = pixelToWindowHeight(ps.height());

  return CQChartsGeom::Size(w, h);
}

double
CQChartsPlot::
pixelToWindowWidth(double pw) const
{
  double wx1, wy1, wx2, wy2;

  pixelToWindowI( 0, 0, wx1, wy1);
  pixelToWindowI(pw, 0, wx2, wy2);

  return std::abs(wx2 - wx1);
}

double
CQChartsPlot::
pixelToWindowHeight(double ph) const
{
  double wx1, wy1, wx2, wy2;

  pixelToWindowI(0, 0 , wx1, wy1);
  pixelToWindowI(0, ph, wx2, wy2);

  return std::abs(wy2 - wy1);
}

double
CQChartsPlot::
windowToSignedPixelWidth(double ww) const
{
  return CMathUtil::sign(ww)*windowToPixelWidth(ww);
}

double
CQChartsPlot::
windowToSignedPixelHeight(double wh) const
{
  return -CMathUtil::sign(wh)*windowToPixelHeight(wh);
}

double
CQChartsPlot::
windowToPixelWidth(double ww) const
{
  double px1, py1, px2, py2;

  windowToPixelI( 0, 0, px1, py1);
  windowToPixelI(ww, 0, px2, py2);

  return std::abs(px2 - px1);
}

double
CQChartsPlot::
windowToPixelHeight(double wh) const
{
  double px1, py1, px2, py2;

  windowToPixelI(0, 0 , px1, py1);
  windowToPixelI(0, wh, px2, py2);

  return std::abs(py2 - py1);
}

CQChartsGeom::Polygon
CQChartsPlot::
windowToPixel(const CQChartsGeom::Polygon &poly) const
{
  CQChartsGeom::Polygon ppoly;

  int np = poly.size();

  for (int i = 0; i < np; ++i)
    ppoly.addPoint(windowToPixel(poly.point(i)));

  return ppoly;
}

QPainterPath
CQChartsPlot::
windowToPixel(const QPainterPath &path) const
{
  QPainterPath ppath;

  int n = path.elementCount();

  for (int i = 0; i < n; ++i) {
    const QPainterPath::Element &e = path.elementAt(i);

    if      (e.isMoveTo()) {
      auto p1 = windowToPixel(CQChartsGeom::Point(e.x, e.y));

      ppath.moveTo(p1.qpoint());
    }
    else if (e.isLineTo()) {
      auto p1 = windowToPixel(CQChartsGeom::Point(e.x, e.y));

      ppath.lineTo(p1.qpoint());
    }
    else if (e.isCurveTo()) {
      QPainterPath::Element     e1, e2;
      QPainterPath::ElementType e1t { QPainterPath::MoveToElement };
      QPainterPath::ElementType e2t { QPainterPath::MoveToElement };

      auto p1 = windowToPixel(CQChartsGeom::Point(e.x, e.y));

      if (i < n - 1) {
        e1  = path.elementAt(i + 1);
        e1t = e1.type;
      }

      if (i < n - 2) {
        e2  = path.elementAt(i + 2);
        e2t = e2.type;
      }

      if (e1t == QPainterPath::CurveToDataElement) {
        auto p2 = windowToPixel(CQChartsGeom::Point(e1.x, e1.y)); ++i;

        if (e2t == QPainterPath::CurveToDataElement) {
          auto p3 = windowToPixel(CQChartsGeom::Point(e2.x, e2.y)); ++i;

          ppath.cubicTo(p1.qpoint(), p2.qpoint(), p3.qpoint());
        }
        else {
          ppath.quadTo(p1.qpoint(), p2.qpoint());
        }
      }
    }
    else {
      assert(false);
    }
  }

  return ppath;
}

//------

void
CQChartsPlot::
plotSymbolSize(const CQChartsLength &s, double &sx, double &sy) const
{
  sx = lengthPlotWidth (s);
  sy = lengthPlotHeight(s);
}

void
CQChartsPlot::
pixelSymbolSize(const CQChartsLength &s, double &sx, double &sy) const
{
  sx = limitSymbolSize(lengthPixelWidth (s));
  sy = limitSymbolSize(lengthPixelHeight(s));
}

double
CQChartsPlot::
limitSymbolSize(double s) const
{
  // ensure not a crazy number : TODO: property for limits
  return CMathUtil::clamp(s, 1.0, CQChartsSymbolSize::maxPixelValue());
}

double
CQChartsPlot::
limitFontSize(double s) const
{
  // ensure not a crazy number : TODO: property for limits
  return CMathUtil::clamp(s, 1.0, CQChartsFontSize::maxPixelValue());
}

//------

bool
CQChartsPlot::
setParameter(CQChartsPlotParameter *param, const QVariant &value)
{
  return CQUtil::setProperty(this, param->propName(), value);
}

bool
CQChartsPlot::
getParameter(CQChartsPlotParameter *param, QVariant &value) const
{
  return CQUtil::getProperty(this, param->propName(), value);
}

bool
CQChartsPlot::
contains(const CQChartsGeom::Point &p) const
{
  return dataRange_.inside(p);
}

//------

void
CQChartsPlot::
write(std::ostream &os, const QString &plotVarName, const QString &modelVarName,
      const QString & /*viewVarName*/) const
{
  auto plotName = [&]() {
    return (plotVarName != "" ? plotVarName : "plot");
  };

  auto modelName = [&]() {
    return (modelVarName != "" ? modelVarName : "model");
  };

  //CQChartsModelData *modelData = getModelData();

  os << "set " << plotName().toStdString();
  os << " [create_charts_plot -model $" << modelName().toStdString() <<
        " -type " << type_->name().toStdString();

  //---

  // add columns
  QVariantList columnsStrs;
//QStringList parametersStrs;
  QStringList parameterPropPaths;

  for (const auto &param : type()->parameters()) {
    QString defStr;

    if (! CQChartsVariant::toString(param->defValue(), defStr))
      defStr = "";

    QVariant value;

    if (! getParameter(param, value))
      continue;

    QString str;

    if (! CQChartsVariant::toString(value, str))
      str = "";

    if (str == defStr)
      continue;

    QStringList strs1;

    strs1 << param->name() << str;

    if (param->type() == CQChartsPlotParameter::Type::COLUMN ||
        param->type() == CQChartsPlotParameter::Type::COLUMN_LIST) {
      columnsStrs += strs1;
    }
    else {
      //parametersStrs += "{" + CQTcl::mergeList(strs1) + "}";
      QString propPath = param->propPath();

      if (propPath == "") {
        if (! propertyModel()->nameToPath(this, param->propName(), propPath))
          continue;
      }

      if (propPath != "")
        parameterPropPaths << propPath;
    }
  }

  if (columnsStrs.length())
    os << " -columns {" << CQTclUtil::variantListToString(columnsStrs).toStdString() + "}";

#if 0
  for (int i = 0; i < parametersStrs.length(); ++i)
    os << " -parameter " << parametersStrs[i].toStdString();
#endif

  if (titleStr() != "")
    os << " -title {" << titleStr().toStdString() + "}";

  //---

  os << "]\n";

  //---

  // get changed name values
  CQPropertyViewModel::NameValues nameValues;

  propertyModel()->getChangedNameValues(this, nameValues, /*tcl*/true);

  // add changed parameter values
  for (auto &propPath : parameterPropPaths) {
    QVariant value;

    bool rc = getProperty(propPath, value);
    assert(rc);

    nameValues[propPath] = value;
  }

  //---

  if (! nameValues.empty())
    os << "\n";

  for (const auto &nv : nameValues) {
    QString str;

    if (! CQChartsVariant::toString(nv.second, str))
      str = "";

    os << "set_charts_property -plot $" << plotName().toStdString();

    os << " -name " << nv.first.toStdString() << " -value {" << str.toStdString() << "}\n";
  }
}

//------

CQChartsPlot::ModelVisitor::
ModelVisitor()
{
}

CQChartsPlot::ModelVisitor::
~ModelVisitor()
{
  delete expr_;
}

void
CQChartsPlot::ModelVisitor::
init()
{
  assert(plot_);

  if (plot_->filterStr().length()) {
    expr_ = new CQChartsModelExprMatch;

    expr_->setModel(plot_->model().data());

    expr_->initMatch(plot_->filterStr());

    expr_->initColumns();
  }
}

CQChartsPlot::ModelVisitor::State
CQChartsPlot::ModelVisitor::
preVisit(const QAbstractItemModel *model, const VisitData &data)
{
  if (plot_->isInterrupt())
    return State::TERMINATE;

  //---

  int vrow = vrow_++;

  //---

  if (expr_) {
    bool ok;

    QModelIndex ind = model->index(data.row, 0, data.parent);

    if (! expr_->match(ind, ok))
      return State::SKIP;
  }

  //---

  if (plot_->isEveryEnabled()) {
    int start = plot_->everyStart();
    int end   = plot_->everyEnd();

    if (vrow < start || vrow > end)
      return State::SKIP;

    int step = plot_->everyStep();

    if (step > 1) {
      int n = (vrow - start) % step;

      if (n != 0)
        return State::SKIP;
    }
  }

  //---

  if (plot_->visibleColumn().isValid()) {
    CQChartsModelIndex visibleColumnInd(data.row, plot_->visibleColumn(), data.parent);

    bool ok;

    QVariant value = plot_->modelValue(visibleColumnInd, ok);

    if (ok && ! CQChartsVariant::toBool(value, ok))
      return State::SKIP;
  }

  return State::OK;
}
