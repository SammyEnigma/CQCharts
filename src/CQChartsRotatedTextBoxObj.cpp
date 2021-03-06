#include <CQChartsRotatedTextBoxObj.h>
#include <CQChartsView.h>
#include <CQChartsPlot.h>
#include <CQChartsRotatedText.h>

CQChartsRotatedTextBoxObj::
CQChartsRotatedTextBoxObj(Plot *plot) :
 CQChartsTextBoxObj(plot)
{
}

void
CQChartsRotatedTextBoxObj::
draw(PaintDevice *device, const Point &center, const QString &text,
     double angle, Qt::Alignment align, bool isRotated) const
{
  draw(device, center, text, "", angle, align, isRotated);
}

void
CQChartsRotatedTextBoxObj::
draw(PaintDevice *device, const Point &center, const QString &text1, const QString &text2,
     double angle, Qt::Alignment align, bool isRotated) const
{
  device->save();

  setPainterFont(device, textFont());

  QFontMetricsF fm(device->font());

  // get text width
  double tw1 = fm.width(text1);
  double tw2 = (text2 != "" ? fm.width(text2) : 0.0);
  double th1 = fm.height();

  // add external margin
  double xlm = lengthPixelWidth (margin().left  ());
  double xrm = lengthPixelWidth (margin().right ());
  double ytm = lengthPixelHeight(margin().top   ());
  double ybm = lengthPixelHeight(margin().bottom());

  double twb = std::max(tw1, tw2) + xlm + xrm;
  double thb = th1 + ybm + ytm;

  if (text2 != "")
    thb += fm.height();

  //---

  auto pcenter = device->windowToPixel(center);

  double cx = pcenter.x;
  double cy = pcenter.y - thb/2;
  double cd = 0.0;

  if      (align & Qt::AlignHCenter) {
    cx -= twb/2;
  }
  else if (align & Qt::AlignRight) {
    cx -= twb;
    cd  = -xrm;
  }
  else {
    cd  = xlm;
  }

  BBox pbbox(cx, cy, cx + twb, cy + thb);

  bbox_ = device->pixelToWindow(pbbox);

  //---

  // draw box
  double cdx, cdy;

  if (CMathUtil::isZero(angle)) {
    CQChartsBoxObj::draw(device, bbox_);

    cdx = cd;
    cdy = 0.0;
  }
  else {
    BBox                        pbbox;
    CQChartsRotatedText::Points ppoints;

    CQChartsGeom::Margin border(xlm, ytm, xrm, ybm);

    CQChartsTextOptions options;

    options.angle = CQChartsAngle(angle);
    options.align = align;

    CQChartsRotatedText::calcBBoxData(pcenter.x, pcenter.y, text1, device->font(), options, border,
                                      pbbox, ppoints, /*alignBBox*/true, /*radial*/isRotated);

    Polygon poly;

    for (std::size_t i = 0; i < ppoints.size(); ++i)
      poly.addPoint(pixelToWindow(ppoints[i]));

    CQChartsBoxObj::draw(device, poly);

    //---

    double tangle = CMathUtil::Deg2Rad(angle);

    double c = std::cos(tangle);
    double s = std::sin(tangle);

    cdx =  c*cd;
    cdy = -s*cd;
  }

  //---

  // draw text
  auto c = interpTextColor(ColorInd());

  CQChartsPenBrush penBrush;

  setPen(penBrush, CQChartsPenData(true, c, textAlpha()));

  device->setPen(penBrush.pen);

  Point p1(pcenter.x + cdx, pcenter.y + cdy);

  CQChartsTextOptions options;

  options.angle         = CQChartsAngle(angle);
  options.align         = align;
  options.contrast      = isTextContrast();
  options.contrastAlpha = textContrastAlpha();
  options.clipLength    = lengthPixelWidth(textClipLength());
  options.clipElide     = textClipElide();

  CQChartsRotatedText::draw(device, device->pixelToWindow(p1), text1,
                            options, /*alignBBox*/true, /*radial*/isRotated);

  if (text2 != "") {
    p1 += Point(0.0, th1);

    CQChartsRotatedText::draw(device, device->pixelToWindow(p1), text2,
                              options, /*alignBBox*/true, /*radial*/isRotated);
  }

  device->restore();
}

// center in pixels
CQChartsGeom::BBox
CQChartsRotatedTextBoxObj::
bbox(const Point &pcenter, const QString &text, double angle, Qt::Alignment align,
     bool isRotated) const
{
  auto font = calcFont(textFont());

  QFontMetricsF fm(font);

  double tw = fm.width(text);

  // external margin
  double xlm = lengthPixelWidth (margin().left  ());
  double xrm = lengthPixelWidth (margin().right ());
  double ytm = lengthPixelHeight(margin().top   ());
  double ybm = lengthPixelHeight(margin().bottom());

  double tw1 = tw + xlm + xrm;
  double th1 = fm.height() + ybm + ytm;

  double cx = pcenter.x;
  double cy = pcenter.y - th1/2;

  if      (align & Qt::AlignHCenter) {
    cx -= tw1/2;
  }
  else if (align & Qt::AlignRight) {
    cx -= tw1;
  }

  BBox pbbox;

  if (angle != 0.0) {
    pbbox = BBox(cx, cy, cx + tw1, cy + th1);
  }
  else {
    CQChartsRotatedText::Points ppoints;

    CQChartsGeom::Margin border(xlm, ytm, xrm, ybm);

    CQChartsTextOptions options;

    options.angle = CQChartsAngle(angle);
    options.align = align;

    CQChartsRotatedText::calcBBoxData(pcenter.x, pcenter.y, text, font, options, border,
                                      pbbox, ppoints, /*alignBBox*/true, /*radial*/isRotated);
  }

  return pixelToWindow(pbbox);
}

void
CQChartsRotatedTextBoxObj::
drawConnectedRadialText(PaintDevice *device, const Point &center, double ro, double lr,
                        double ta, const QString &text, const QPen &lpen, bool isRotated)
{
  BBox tbbox;

  drawCalcConnectedRadialText(device, center, ro, lr, ta, text, lpen, isRotated, tbbox);
}

void
CQChartsRotatedTextBoxObj::
calcConnectedRadialTextBBox(const Point &center, double ro, double lr, double ta,
                            const QString &text, bool isRotated, BBox &tbbox)
{
  QPen lpen;

  drawCalcConnectedRadialText((PaintDevice *) 0, center, ro, lr, ta,
                              text, lpen, isRotated, tbbox);
}

void
CQChartsRotatedTextBoxObj::
drawCalcConnectedRadialText(PaintDevice *device, const Point &center, double ro,
                            double lr, double ta, const QString &text,
                            const QPen &lpen, bool isRotated, BBox &tbbox)
{
  if (device)
    device->save();

  //---

  double tangle = CMathUtil::Deg2Rad(ta);

  double tc = std::cos(tangle);
//double ts = std::sin(tangle);

  bool labelRight = (tc >= 0.0);

  auto p = CQChartsGeom::circlePoint(center, lr, tangle);

  auto pt = windowToPixel(p);

  //---

  double        dx    = 0.0;
  Qt::Alignment align = Qt::AlignHCenter | Qt::AlignVCenter;

  // connect text to outer edge
  if (lr > ro) {
    auto pl1 = CQChartsGeom::circlePoint(center, ro, tangle);
    auto pl2 = CQChartsGeom::circlePoint(center, lr, tangle);

    auto lp1 = windowToPixel(pl1);
    auto lp2 = windowToPixel(pl2);

    int tickSize = 16; // TODO: allow customize

    if (plot() && plot()->isInvertX())
      labelRight = ! labelRight;

    if (labelRight) {
      dx    = tickSize;
      align = Qt::AlignLeft | Qt::AlignVCenter;
    }
    else {
      dx    = -tickSize;
      align = Qt::AlignRight | Qt::AlignVCenter;
    }

    if (device) {
      device->setPen(lpen);

      Point lp3(lp2.x + dx, lp2.y);

      auto l1 = device->pixelToWindow(lp1);
      auto l2 = device->pixelToWindow(lp2);
      auto l3 = device->pixelToWindow(lp3);

      device->drawLine(l1, l2);
      device->drawLine(l2, l3);
    }
  }

  //---

  // draw text
  Point pt1(pt.x + dx, pt.y);

  double angle = 0.0;

  if (isRotated)
    angle = (labelRight ? ta : 180.0 + ta);

  auto t1 = (device ? device->pixelToWindow(pt1) : pt1);

  if (device)
    draw(device, t1, text, angle, align, isRotated);
  else
    tbbox = this->bbox(t1, text, angle, align, isRotated);

  //---

  if (device)
    device->restore();
}
