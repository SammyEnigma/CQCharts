#ifndef CQChartsPlot3D_H
#define CQChartsPlot3D_H

#include <CQChartsGroupPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsPlotType.h>
#include <CQChartsCamera.h>
#include <CQCharts.h>
#include <CInterval.h>

class CQChartsPlot3DObj;

class CQChartsPlot3DType : public CQChartsGroupPlotType {
 public:
  CQChartsPlot3DType();
};

class CQChartsPlot3D : public CQChartsGroupPlot {
  Q_OBJECT

  // range
  Q_PROPERTY(CQChartsOptReal zmin READ zmin WRITE setZMin)
  Q_PROPERTY(CQChartsOptReal zmax READ zmax WRITE setZMax)

  // grid
  Q_PROPERTY(bool gridLines READ isGridLines WRITE setGridLines)

  // camera
  Q_PROPERTY(double cameraRotateX READ cameraRotateX WRITE setCameraRotateX)
  Q_PROPERTY(double cameraRotateY READ cameraRotateY WRITE setCameraRotateY)
  Q_PROPERTY(double cameraRotateZ READ cameraRotateZ WRITE setCameraRotateZ)
  Q_PROPERTY(double cameraScale   READ cameraScale   WRITE setCameraScale  )
  Q_PROPERTY(double cameraXMin    READ cameraXMin    WRITE setCameraXMin   )
  Q_PROPERTY(double cameraXMax    READ cameraXMax    WRITE setCameraXMax   )
  Q_PROPERTY(double cameraYMin    READ cameraYMin    WRITE setCameraYMin   )
  Q_PROPERTY(double cameraYMax    READ cameraYMax    WRITE setCameraYMax   )
  Q_PROPERTY(double cameraNear    READ cameraNear    WRITE setCameraNear   )
  Q_PROPERTY(double cameraFar     READ cameraFar     WRITE setCameraFar    )

 public:
  using Range3D = CQChartsGeom::Range3D;
  using Camera  = CQChartsCamera;

 public:
  CQChartsPlot3D(CQChartsView *view, CQChartsPlotType *plotType, const ModelP &model);

  //---

  const Range3D &range3D() const { return range3D_; }

  const CQChartsOptReal &zmin() const { return zmin_; }
  void setZMin(const CQChartsOptReal &r);

  const CQChartsOptReal &zmax() const { return zmax_; }
  void setZMax(const CQChartsOptReal &r);

  //---

  Camera *camera() const { return camera_; }

  //---

  bool isGridLines() const { return gridLines_; }
  void setGridLines(bool b);

  //---

  double cameraRotateX() const { return camera_->rotateX(); }
  void setCameraRotateX(double x);

  double cameraRotateY() const { return camera_->rotateY(); }
  void setCameraRotateY(double y);

  double cameraRotateZ() const { return camera_->rotateZ(); }
  void setCameraRotateZ(double z);

  double cameraScale() const { return camera_->scaleX(); }
  void setCameraScale(double s);

  double cameraXMin() const { return camera_->xmin(); }
  void setCameraXMin(double x);

  double cameraXMax() const { return camera_->xmax(); }
  void setCameraXMax(double x);

  double cameraYMin() const { return camera_->ymin(); }
  void setCameraYMin(double y);

  double cameraYMax() const { return camera_->ymax(); }
  void setCameraYMax(double y);

  double cameraNear() const { return camera_->near(); }
  void setCameraNear(double z);

  double cameraFar() const { return camera_->far(); }
  void setCameraFar(double z);

  //---

  void addCameraProperties();

  //---

  void postUpdateRange() override;

  void deletePointObjs();

  //---

  bool objNearestPoint(const CQChartsGeom::Point &p, CQChartsPlotObj* &obj) const override;

  void plotObjsAtPoint(const CQChartsGeom::Point &p, PlotObjs &objs) const override;

  //---

  void drawBackgroundRects(CQChartsPaintDevice *device) const override;

  void addAxis(const CQChartsColumn &xColumn, const CQChartsColumn &yColumn,
               const CQChartsColumn &zColumn) const;

  //---

  bool objInsideBox(CQChartsPlotObj *, const CQChartsGeom::BBox &) const override { return true; }

  //---

  double boxZMin() const { return boxZMin_; }
  double boxZMax() const { return boxZMax_; }

  //---

  void addBgPointObj(const CQChartsGeom::Point3D &p, CQChartsPlot3DObj *obj);
  void addPointObj  (const CQChartsGeom::Point3D &p, CQChartsPlot3DObj *obj);
  void addFgPointObj(const CQChartsGeom::Point3D &p, CQChartsPlot3DObj *obj);

  void drawPointObjs(CQChartsPaintDevice *device) const;

  //---

  bool selectMousePress(const CQChartsGeom::Point &p, SelMod selMod) override;

 protected:
  struct Axis {
    enum class Dir {
      NONE,
      X,
      Y,
      Z
    };

    Axis(const Dir &dir) : dir(dir) { }

    void init(double start, double end) {
      interval.setStart(start);
      interval.setEnd  (end  );
    }

    Dir       dir      { Dir::NONE }; //!< direction
    CInterval interval;               //!< interval data
  };

  Range3D         range3D_; //! 3D range
  CQChartsOptReal zmin_;    //!< zmin override
  CQChartsOptReal zmax_;    //!< zmax override

  Camera* camera_    { nullptr };      //! camera
  bool    gridLines_ { true };         //! show axis grid lines
  Axis    xAxis_     { Axis::Dir::X }; //! x axis
  Axis    yAxis_     { Axis::Dir::Y }; //! y axis
  Axis    zAxis_     { Axis::Dir::Z }; //! z axis

  double boxZMin_ { 0.0 };
  double boxZMax_ { 0.0 };

  using Objs      = std::vector<CQChartsPlot3DObj *>;
  using PointObjs = std::map<CQChartsGeom::Point3D,Objs>;

  mutable PointObjs bgPointObjs_;
  mutable PointObjs pointObjs_;
  mutable PointObjs fgPointObjs_;
};

//---

class CQChartsPlot3DObj : public CQChartsPlotObj {
 public:
  CQChartsPlot3DObj(const CQChartsPlot3D *plot);

  const CQChartsPlot3D *plot3D() const { return plot3D_; }

  const CQChartsGeom::Point3D &refPoint() const { return refPoint_; }
  void setRefPoint(const CQChartsGeom::Point3D &p) { refPoint_ = p; }

  const CQChartsPenBrush &penBrush() const { return penBrush_; }
  void setPenBrush(const CQChartsPenBrush &v) { penBrush_ = v; }

  const CQChartsGeom::BBox &drawBBox() const { return drawBBox_; }
  void setDrawBBox(const CQChartsGeom::BBox &b) { drawBBox_ = b; }

 private:
  const CQChartsPlot3D*      plot3D_ { nullptr }; //! parent plot
  CQChartsGeom::Point3D      refPoint_;           //! reference point
  CQChartsPenBrush           penBrush_;           //! pen/brush
  mutable CQChartsGeom::BBox drawBBox_;           //! draw bounding box
};

//---

class CQChartsLine3DObj : public CQChartsPlot3DObj {
 public:
  CQChartsLine3DObj(const CQChartsPlot3D *plot, const CQChartsGeom::Point3D &p1,
                    const CQChartsGeom::Point3D &p2, const QColor &color);

  const CQChartsGeom::Point3D &p1() const { return p1_; }
  void setP1(const CQChartsGeom::Point3D &p1) { p1_ = p1; }

  const CQChartsGeom::Point3D &p2() const { return p2_; }
  void setP2(const CQChartsGeom::Point3D &p2) { p2_ = p2; }

  const QColor &color() const { return color_; }
  void setColor(const QColor &v) { color_ = v; }

  double z() const { return z_; }
  void setZ(double r) { z_ = r; }

  QString typeName() const override { return "line"; }

  void postDraw(CQChartsPaintDevice *device) override;

 private:
  CQChartsGeom::Point3D p1_;                //!< start point
  CQChartsGeom::Point3D p2_;                //!< end point
  QColor                color_ { 0, 0, 0 }; //!< color
  double                z_    { 0.0 };      //!< z
};

//---

class CQChartsText3DObj : public CQChartsPlot3DObj {
 public:
  CQChartsText3DObj(const CQChartsPlot3D *plot, const CQChartsGeom::Point3D &p1,
                    const CQChartsGeom::Point3D &p2, const QString &text);

  const CQChartsGeom::Point3D &p1() const { return p1_; }
  void setP1(const CQChartsGeom::Point3D &p) { p1_ = p; }

  const CQChartsGeom::Point3D &p2() const { return p2_; }
  void setP2(const CQChartsGeom::Point3D &p) { p2_ = p; }

  bool isVertical() const { return vertical_; }
  void setVertical(bool b) { vertical_ = b; }

  const QString &text() const { return text_; }
  void setText(const QString &s) { text_ = s; }

  double z() const { return z_; }
  void setZ(double r) { z_ = r; }

  const CQChartsFont &font() const { return font_; }
  void setFont(const CQChartsFont &v) { font_ = v; }

  const CQChartsTextOptions &textOptions() const { return textOptions_; }
  void setTextOptions(const CQChartsTextOptions &v) { textOptions_ = v; }

  bool isAutoAlign() const { return autoAlign_; }
  void setAutoAlign(bool b) { autoAlign_ = b; }

  QString typeName() const override { return "line"; }

  void postDraw(CQChartsPaintDevice *device) override;

 private:
  CQChartsGeom::Point3D p1_;                  //!< point
  CQChartsGeom::Point3D p2_;                  //!< offset point
  bool                  vertical_  { false }; //!< is vertical
  QString               text_;                //!< text
  double                z_         { 0.0 };   //!< z
  CQChartsFont          font_;                //!< font
  CQChartsTextOptions   textOptions_;         //!< text options
  bool                  autoAlign_ { false }; //!< auto align
};

//---

class CQChartsPolyline3DObj : public CQChartsPlot3DObj {
 public:
  CQChartsPolyline3DObj(const CQChartsPlot3D *plot,
                        const CQChartsGeom::Polygon3D &poly=CQChartsGeom::Polygon3D());

  const CQChartsGeom::Polygon3D &poly() const { return poly_; }

  QString typeName() const override { return "poly"; }

  void addPoint(const CQChartsGeom::Point3D &p) { poly_.addPoint(p); }

  void postDraw(CQChartsPaintDevice *device) override;

 private:
  CQChartsGeom::Polygon3D poly_; //!< polygon
};

//---

class CQChartsPolygon3DObj : public CQChartsPlot3DObj {
 public:
  CQChartsPolygon3DObj(const CQChartsPlot3D *plot,
                       const CQChartsGeom::Polygon3D &poly=CQChartsGeom::Polygon3D());

  const CQChartsGeom::Polygon3D &poly() const { return poly_; }

  QString typeName() const override { return "poly"; }

  void addPoint(const CQChartsGeom::Point3D &p) { poly_.addPoint(p); }

  const CQChartsGeom::Point3D &normal() const { return normal_; }
  void setNormal(const CQChartsGeom::Point3D &v) { normal_ = v; }

  const QColor &color() const { return color_; }
  void setColor(const QColor &v) { color_ = v; }

  void postDraw(CQChartsPaintDevice *device) override;

  bool checkVisible() const;

 private:
  CQChartsGeom::Polygon3D poly_;   //!< polygon
  CQChartsGeom::Point3D   normal_; //!< normal
  QColor                  color_;  //!< color
};

//---

class CQChartsAxisPolygon3DObj : public CQChartsPolygon3DObj {
 public:
  struct SidePolygon {
    const CQChartsAxisPolygon3DObj *poly { nullptr };
    CQChartsGeom::Point3D           p1;
    CQChartsGeom::Point3D           p2;

    SidePolygon() = default;

    SidePolygon(const CQChartsAxisPolygon3DObj *poly, const CQChartsGeom::Point3D &p1,
                CQChartsGeom::Point3D &p2) :
     poly(poly), p1(p1), p2(p2) {
    }
  };

  using SidePolygons = std::vector<SidePolygon>;

  struct Line {
    CQChartsGeom::Point3D p1;
    CQChartsGeom::Point3D p2;

    Line() = default;

    Line(const CQChartsGeom::Point3D &p1, const CQChartsGeom::Point3D &p2) :
     p1(p1), p2(p2) {
    }
  };

  using Lines = std::vector<Line>;

  struct Text {
    CQChartsGeom::Point3D           p;
    QString                         text;
    const CQChartsAxisPolygon3DObj *poly { nullptr };

    Text() = default;

    Text(const CQChartsGeom::Point3D &p, const QString &text,
         const CQChartsAxisPolygon3DObj *poly) :
     p(p), text(text), poly(poly) {
    }
  };

  using Texts = std::vector<Text>;

 public:
  CQChartsAxisPolygon3DObj(const CQChartsPlot3D *plot,
                           const CQChartsGeom::Polygon3D &poly=CQChartsGeom::Polygon3D());

  QString typeName() const override { return "axis_poly"; }

  void addSidePolygon(const CQChartsAxisPolygon3DObj *poly, const CQChartsGeom::Point3D &p1,
                      CQChartsGeom::Point3D &p2) {
    sidePolygons_.push_back(SidePolygon(poly, p1, p2));
  }

  void addGridLine(const CQChartsGeom::Point3D &p1, const CQChartsGeom::Point3D &p2) {
    gridLines_.push_back(Line(p1, p2));
  }

  void addTickText(const CQChartsGeom::Point3D &p, const QString &text,
                  const CQChartsAxisPolygon3DObj *poly) {
    tickTexts_.push_back(Text(p, text, poly));
  }

  void addLabelText(const CQChartsGeom::Point3D &p, const QString &text,
                    const CQChartsAxisPolygon3DObj *poly) {
    labelTexts_.push_back(Text(p, text, poly));
  }

  void postDraw(CQChartsPaintDevice *device) override;

 private:
  SidePolygons sidePolygons_;
  Lines        gridLines_;
  Texts        tickTexts_;
  Texts        labelTexts_;
};

#endif