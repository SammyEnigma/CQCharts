#include <CQChartsCompositePlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsKey.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <CQChartsHtml.h>

#include <CQPropertyViewItem.h>

#include <QMenu>

CQChartsCompositePlotType::
CQChartsCompositePlotType()
{
}

void
CQChartsCompositePlotType::
addParameters()
{
  CQChartsPlotType::addParameters();
}

bool
CQChartsCompositePlotType::
canProbe() const
{
  return true;
}

bool
CQChartsCompositePlotType::
hasObjs() const
{
  return true;
}

QString
CQChartsCompositePlotType::
description() const
{
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Composite Plot").
    h3("Summary").
     p("Composite plot.").
    h3("Limitations").
     p("None.").
    h3("Example").
     p(IMG("images/composite_plot.png"));
}

CQChartsPlot *
CQChartsCompositePlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsCompositePlot(view, model);
}

//------

CQChartsCompositePlot::
CQChartsCompositePlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("composite"), model)
{
  addAxes();

  addKey();

  addTitle();
}

CQChartsCompositePlot::
~CQChartsCompositePlot()
{
}

//---

void
CQChartsCompositePlot::
setCompositeType(const CompositeType &t)
{
  compositeType_ = t;

  updatePlots();
}

void
CQChartsCompositePlot::
addPlot(CQChartsPlot *plot)
{
  plots_.push_back(plot);

  plot->setParentPlot(this);

  if (! currentPlot())
    setCurrentPlot(plot);
  else
    plot->setCurrent(false);

  updatePlots();
}

void
CQChartsCompositePlot::
updatePlots()
{
  // common y different x axis
  if      (compositeType_ == CompositeType::X1X2) {
    int i = 0;

    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      if (plot->xAxis())
        plot->xAxis()->setSide(i == 0 ? CQChartsAxisSide::Type::BOTTOM_LEFT :
                                        CQChartsAxisSide::Type::TOP_RIGHT);

      ++i;
    }
  }
  // common x different y axis
  else if (compositeType_ == CompositeType::Y1Y2) {
    int i = 0;

    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      if (plot->yAxis())
        plot->yAxis()->setSide(i == 0 ? CQChartsAxisSide::Type::BOTTOM_LEFT :
                                        CQChartsAxisSide::Type::TOP_RIGHT);

      ++i;
    }
  }
  else {
    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      if (plot->xAxis())
        plot->xAxis()->setSide(CQChartsAxisSide::Type::BOTTOM_LEFT);

      if (plot->yAxis())
        plot->yAxis()->setSide(CQChartsAxisSide::Type::BOTTOM_LEFT);
    }
  }
}

//---

void
CQChartsCompositePlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  addProp("", "compositeType", "compositeType", "Composite Type");

  addBaseProperties();
}

CQChartsGeom::Range
CQChartsCompositePlot::
calcRange() const
{
  CQChartsGeom::Range dataRange;

  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    dataRange += plot->calcRange();
  }

  if (! dataRange.isSet()) {
    dataRange.updateRange(0.0, 0.0);
    dataRange.updateRange(1.0, 1.0);
  }

  return dataRange;
}

bool
CQChartsCompositePlot::
createObjs(PlotObjs &) const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->createObjs();
  }

  return true;
}

CQChartsGeom::BBox
CQChartsCompositePlot::
calcAnnotationBBox() const
{
  CQChartsGeom::BBox bbox;

  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    bbox += plot->calcAnnotationBBox();
  }

  return bbox;
}

void
CQChartsCompositePlot::
updateAxisRanges(const CQChartsGeom::BBox &adjustedRange)
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->updateAxisRanges(adjustedRange);
  }

  CQChartsPlot::updateAxisRanges(adjustedRange);
}

//---

CQChartsGeom::BBox
CQChartsCompositePlot::
adjustedViewBBox(const CQChartsPlot *plot) const
{
  CQChartsGeom::BBox bbox = plot->viewBBox();

  if (compositeType_ == CompositeType::TABBED) {
    calcTabData(plots_);

    double h = view()->pixelToWindowHeight(tabData_.pth*plot->plotDepth());

    bbox = CQChartsGeom::BBox(bbox.getXMin(), bbox.getYMin() + h, bbox.getXMax(), bbox.getYMax());
  }

  return bbox;
}

//---

void
CQChartsCompositePlot::
waitRange()
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->execWaitRange();
  }

  CQChartsPlot::execWaitRange();
}

void
CQChartsCompositePlot::
waitObjs()
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->execWaitObjs();
  }

  CQChartsPlot::execWaitObjs();
}

void
CQChartsCompositePlot::
waitDraw()
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->execWaitDraw();
  }

  CQChartsPlot::execWaitDraw();
}

//---

bool
CQChartsCompositePlot::
hasTitle() const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->hasTitle())
      return true;
  }

  return false;
}

bool
CQChartsCompositePlot::
hasXAxis() const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->hasXAxis())
      return true;
  }

  return false;
}

bool
CQChartsCompositePlot::
hasYAxis() const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->hasYAxis())
      return true;
  }

  return false;
}

bool
CQChartsCompositePlot::
hasObjs(const CQChartsLayer::Type &layerType) const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->hasObjs(layerType))
      return true;
  }

  return false;
}

//------

void
CQChartsCompositePlot::
drawBackgroundDeviceParts(CQChartsPaintDevice *device, bool bgLayer, bool bgAxes,
                          bool bgKey) const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    plot->drawBackgroundDeviceParts(device, bgLayer, /*bgAxes*/false, /*bgKey*/false);
  }

  if (bgAxes)
    drawGroupedBgAxes(device);

  if (bgKey)
    drawBgKey(device);
}

void
CQChartsCompositePlot::
drawMiddleDeviceParts(CQChartsPaintDevice *device, bool bg, bool mid,
                      bool fg, bool annotations) const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    plot->drawMiddleDeviceParts(device, bg, mid, fg, annotations);
  }
}

void
CQChartsCompositePlot::
drawForegroundDeviceParts(CQChartsPaintDevice *device, bool fgAxes, bool fgKey,
                          bool title, bool foreground, bool tabbed) const
{
  if (fgAxes)
    drawGroupedFgAxes(device);

  if (fgKey)
    drawFgKey(device);

  if (title)
    drawTitle(device);

  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    plot->drawForegroundDeviceParts(device, /*fgAxes*/false, /*fgKey*/false,
                                    /*title*/false, foreground, /*tabbed*/false);
  }

  //---

  if (tabbed)
    drawTabs(device);

  if (compositeType_ == CompositeType::TABBED)
    drawTabs(device, plots_, currentPlot());
}

void
CQChartsCompositePlot::
drawOverlayDeviceParts(CQChartsPaintDevice *device, bool sel_objs, bool sel_annotations,
                       bool boxes, bool edit_handles, bool over_objs, bool over_annotations) const
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    plot->drawOverlayDeviceParts(device, sel_objs, sel_annotations, boxes,
                                 edit_handles, over_objs, over_annotations);
  }
}

void
CQChartsCompositePlot::
drawBgAxes(CQChartsPaintDevice *device) const
{
  // common y different x axis
  if      (compositeType_ == CompositeType::X1X2) {
    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      plot->drawBgAxes(device);
    }
  }
  // common x different y axis
  else if (compositeType_ == CompositeType::Y1Y2) {
    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      plot->drawBgAxes(device);
    }
  }
  else {
#if 0
    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
        continue;

      plot->drawBgAxes(device);
    }
#endif

    if (currentPlot() && currentPlot()->isVisible())
      currentPlot()->drawBgAxes(device);
  }
}

void
CQChartsCompositePlot::
drawFgAxes(CQChartsPaintDevice *device) const
{
  // common y different x axis
  if      (compositeType_ == CompositeType::X1X2) {
    int i = 0;

    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      plot->drawFgAxes(device);

      ++i;

      if (i == 2)
        break;
    }
  }
  // common x different y axis
  else if (compositeType_ == CompositeType::Y1Y2) {
    int i = 0;

    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      plot->drawFgAxes(device);

      ++i;

      if (i == 2)
        break;
    }
  }
  else {
#if 0
    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
        continue;

      plot->drawFgAxes(device);
    }
#endif

    if (currentPlot() && currentPlot()->isVisible())
      currentPlot()->drawFgAxes(device);
  }
}

void
CQChartsCompositePlot::
drawTitle(CQChartsPaintDevice *device) const
{
#if 0
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    plot->drawTitle(device);
  }
#endif

  if (currentPlot() && currentPlot()->isVisible())
    currentPlot()->drawTitle(device);
}

//------

bool
CQChartsCompositePlot::
addMenuItems(QMenu *menu)
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    QMenu *plotMenu = new QMenu(plot->id(), menu);

    if (plot->addMenuItems(plotMenu))
      menu->addMenu(plotMenu);
    else
      delete plotMenu;
  }

  return true;
}

//------

void
CQChartsCompositePlot::
resetKeyItems()
{
  if (compositeType_ == CompositeType::TABBED) {
    // key per plot
    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      plot->resetKeyItems();
    }
  }
  else {
    // shared key
    if (! key())
      return;

    key()->clearItems();

    for (auto &plot : plots_) {
      if (! plot->isVisible())
        continue;

      plot->doAddKeyItems(key());
    }
  }
}

//------

bool
CQChartsCompositePlot::
selectPress(const CQChartsGeom::Point &w, SelMod selMod)
{
  if (tabbedSelectPress(w, selMod))
    return true;

  if (compositeType_ == CompositeType::TABBED) {
    CQChartsPlot *pressPlot = tabbedPressPlot(w, plots_);

    if (pressPlot) {
      setCurrentPlot(pressPlot);

      pressPlot->updateRangeAndObjs();

      return true;
    }
  }

  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->selectPress(w, selMod))
      return true;
  }

  return false;
}

bool
CQChartsCompositePlot::
selectMove(const CQChartsGeom::Point &w, bool first)
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->selectMove(w, first))
      return true;
  }

  return false;
}

bool
CQChartsCompositePlot::
selectRelease(const CQChartsGeom::Point &w)
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    if (compositeType_ == CompositeType::TABBED && plot != currentPlot())
      continue;

    if (plot->selectRelease(w))
      return true;
  }

  return false;
}

//------

void
CQChartsCompositePlot::
invalidateOverlay()
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->execInvalidateOverlay();
  }

  CQChartsPlot::execInvalidateOverlay();
}

void
CQChartsCompositePlot::
invalidateLayers()
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->execInvalidateLayers();
  }

  CQChartsPlot::execInvalidateLayers();
}

void
CQChartsCompositePlot::
invalidateLayer(const CQChartsBuffer::Type &layerType)
{
  for (auto &plot : plots_) {
    if (! plot->isVisible())
      continue;

    plot->execInvalidateLayer(layerType);
  }

  CQChartsPlot::execInvalidateLayer(layerType);
}

//---

CQChartsPlot *
CQChartsCompositePlot::
currentPlot() const
{
  if (currentPlot_ && currentPlot_->isVisible())
    return currentPlot_;

  for (auto &plot : plots_) {
    if (plot->isVisible())
      return plot;
  }

  return nullptr;
}

void
CQChartsCompositePlot::
setCurrentPlot(CQChartsPlot *currentPlot)
{
  for (auto &plot : plots_)
    plot->setCurrent(plot == currentPlot);

  currentPlot_ = currentPlot;
}

int
CQChartsCompositePlot::
childPlotIndex(const CQChartsPlot *childPlot) const
{
  int i = 0;

  for (auto &plot : plots_) {
    if (plot == childPlot)
      return i;

    ++i;
  }

  return -1;
}

int
CQChartsCompositePlot::
numChildPlots() const
{
  return plots_.size();
}