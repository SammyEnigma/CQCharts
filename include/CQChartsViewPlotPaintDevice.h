#ifndef CQChartsViewPlotPaintDevice_H
#define CQChartsViewPlotPaintDevice_H

#include <CQChartsPaintDevice.h>
#include <CQChartsPlot.h>

class CQHandDrawnPainter;

/*!
 * \brief Paint Device for drawing on Plot or View
 * \ingroup Charts
 */
class CQChartsViewPlotPaintDevice : public CQChartsPaintDevice {
 public:
  using View  = CQChartsView;
  using Plot  = CQChartsPlot;
  using Angle = CQChartsAngle;
  using Image = CQChartsImage;
  using Units = CQChartsUnits::Type;

 public:
  CQChartsViewPlotPaintDevice(View *view, QPainter *painter);
  CQChartsViewPlotPaintDevice(Plot *plot, QPainter *painter);

  explicit CQChartsViewPlotPaintDevice(QPainter *painter);

 ~CQChartsViewPlotPaintDevice();

  //---

  bool isInteractive() const override { return true; }

  //---

  // hand draw

  bool isHandDrawn() const { return handDrawn_; }
  void setHandDrawn(bool b);

  double handRoughness() const { return handRoughness_; }
  void setHandRoughness(double r);

  double handFillDelta() const { return handFillDelta_; }
  void setHandFillDelta(double r);

  //---

  QPainter *painter() const override { return painter_; }

  void save   () override;
  void restore() override;

  void setClipPath(const QPainterPath &path, Qt::ClipOperation operation=Qt::ReplaceClip) override;
  void setClipRect(const BBox &bbox, Qt::ClipOperation operation=Qt::ReplaceClip) override;

  BBox clipRect() const override;

  QPen pen() const override;
  void setPen(const QPen &pen) override;

  QBrush brush() const override;
  void setBrush(const QBrush &brush) override;

  void setAltColor(const QColor &c) override;
  void setAltAlpha(double alpha) override;

  void setFillAngle(double a) override;
  void setFillType(CQChartsFillPattern::Type t) override;
  void setFillRadius(double r) override;
  void setFillDelta(double d) override;
  void setFillWidth(double w) override;

  void fillPath  (const QPainterPath &path, const QBrush &brush) override;
  void strokePath(const QPainterPath &path, const QPen &pen) override;
  void drawPath  (const QPainterPath &path) override;

  void fillRect(const BBox &bbox) override;
  void drawRect(const BBox &bbox) override;

  void drawEllipse(const BBox &bbox, const Angle &a=Angle()) override;

//void drawArc(const BBox &rect, const Angle &a1, Angle &a2) override;

  void drawPolygon (const Polygon &poly) override;
  void drawPolyline(const Polygon &poly) override;

  void drawLine(const Point &p1, const Point &p2) override;

  void drawPoint(const Point &p) override;

  void drawText(const Point &p, const QString &text) override;
  void drawTransformedText(const Point &p, const QString &text) override;

  void drawImage(const Point &, const Image &) override;
  void drawImageInRect(const BBox &bbox, const Image &image, bool stretch=true,
                       const Angle &angle=Angle()) override;

  const QFont &font() const override;
  void setFont(const QFont &f, bool scale=true) override;

  void setTransformRotate(const Point &p, double angle) override;

  const QTransform &transform() const override;
  void setTransform(const QTransform &t, bool combine=false) override;

  void setRenderHints(QPainter::RenderHints hints, bool on) override;

  //---

  void setPainterFont(const Font &font) override;

 private:
  void updateBackground();

  bool adjustFillBrush(const QBrush &brush, const BBox &bbox, QBrush &brush1) const;

 private:
  QPainter*                 painter_       { nullptr };
  BBox                      clipRect_;
  QPainterPath              clipPath_;
  QColor                    altColor_;
  double                    altAlpha_      { 1.0 };
  QFont                     font_;
  bool                      handDrawn_     { false };
  int                       saveDepth_     { 0 };
  double                    handRoughness_ { 1.0 };
  double                    handFillDelta_ { 16.0 };
  CQHandDrawnPainter*       hdPainter_     { nullptr };
  double                    fillAngle_     { 0.0 }; // degrees
  CQChartsFillPattern::Type fillType_      { CQChartsFillPattern::Type::NONE };
  double                    fillRadius_    { -1 };
  double                    fillDelta_     { -1 };
  double                    fillWidth_     { -1 };
};

//---

/*!
 * \brief Paint Device for drawing on View
 * \ingroup Charts
 */
class CQChartsViewPaintDevice : public CQChartsViewPlotPaintDevice {
 public:
  CQChartsViewPaintDevice(View *view, QPainter *painter);

  Type type() const override { return Type::VIEW; }
};

//---

/*!
 * \brief Paint Device for drawing on Plot
 * \ingroup Charts
 */
class CQChartsPlotPaintDevice : public CQChartsViewPlotPaintDevice {
 public:
  CQChartsPlotPaintDevice(Plot *plot, QPainter *painter);

  Type type() const override { return Type::PLOT; }
};

#endif
