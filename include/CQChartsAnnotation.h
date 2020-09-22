#ifndef CQChartsAnnotation_H
#define CQChartsAnnotation_H

#include <CQChartsTextBoxObj.h>
#include <CQChartsSymbol.h>
#include <CQChartsImage.h>
#include <CQChartsWidget.h>
#include <CQChartsData.h>
#include <CQChartsOptRect.h>
#include <CQChartsOptPosition.h>
#include <CQChartsPolygon.h>
#include <CQChartsPoints.h>
#include <CQChartsReals.h>
#include <CQChartsGeom.h>
#include <CQChartsGrahamHull.h>
#include <CQChartsGridCell.h>
#include <CQChartsAngle.h>
#include <CQChartsObjRef.h>

class CQChartsAnnotationGroup;
class CQChartsSmooth;
class CQChartsDensity;
class CQChartsKey;
class CQChartsAxis;
class CQChartsHtmlPaintDevice;

class CQPropertyViewItem;

/*!
 * \brief base class for view/plot annotation
 * \ingroup Charts
 */
class CQChartsAnnotation : public CQChartsTextBoxObj {
  Q_OBJECT

  Q_PROPERTY(int    ind              READ ind              WRITE setInd             )
  Q_PROPERTY(bool   enabled          READ isEnabled        WRITE setEnabled         )
  Q_PROPERTY(bool   checkable        READ isCheckable      WRITE setCheckable       )
  Q_PROPERTY(bool   checked          READ isChecked        WRITE setChecked         )
  Q_PROPERTY(double disabledLighter  READ disabledLighter  WRITE setDisabledLighter )
  Q_PROPERTY(double uncheckedLighter READ uncheckedLighter WRITE setUncheckedLighter)

 public:
  enum class Type {
    NONE,
    GROUP,
    RECT,
    ELLIPSE,
    POLYGON,
    POLYLINE,
    TEXT,
    IMAGE,
    ARROW,
    POINT,
    PIE_SLICE,
    AXIS,
    KEY,
    POINT_SET,
    VALUE_SET,
    BUTTON,
    WIDGET
  };

  using View       = CQChartsView;
  using Plot       = CQChartsPlot;
  using Group      = CQChartsAnnotationGroup;
  using ResizeSide = CQChartsResizeSide;
  using SelMod     = CQChartsSelMod;
  using ColorInd   = CQChartsUtil::ColorInd;

  using PaintDevice     = CQChartsPaintDevice;
  using HtmlPaintDevice = CQChartsHtmlPaintDevice;

 public:
  static const QStringList &typeNames();

 public:
  CQChartsAnnotation(View *view, Type type);
  CQChartsAnnotation(Plot *plot, Type type);

  virtual ~CQChartsAnnotation();

  //---

  //! get/set ind
  int ind() const { return ind_; }
  void setInd(int i) { ind_ = i; }

  //! calculate id
  QString calcId() const override;

  //! calculate tip
  QString calcTipId() const override;

  //! get path id
  QString pathId() const;

  //---

  const Group *group() const { return group_; }
  void setGroup(Group *g) { group_ = g; }

  //---

  //! get type and type name
  const Type &type() const { return type_; }

  virtual const char *typeName() const = 0;

  virtual const char *propertyName() const = 0;

  virtual const char *cmdName() const = 0;

  //---

  // enabled
  bool isEnabled() const { return enabled_; }
  void setEnabled(bool b);

  // checkable/checked
  bool isCheckable() const { return checkable_; }
  void setCheckable(bool b);

  bool isChecked() const { return checked_; }
  void setChecked(bool b);

  //---

  // disabled lighter
  double disabledLighter() const { return disabledLighter_; }
  void setDisabledLighter(double r) { disabledLighter_ = r; }

  // unchecked lighter
  double uncheckedLighter() const { return uncheckedLighter_; }
  void setUncheckedLighter(double r) { uncheckedLighter_ = r; }

  //---

  //! get bounding box
  const BBox &annotationBBox() const { return annotationBBox_; }
  void setAnnotationBBox(const BBox &bbox);

  //---

  void getMarginValues (double &xlm, double &xrm, double &ytm, double &ybm) const;
  void getPaddingValues(double &xlp, double &xrp, double &ytp, double &ybp) const;

  //---

  //! get property path
  virtual QString propertyId() const;

  //! add properties
  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //! add stroke and fill properties
  void addStrokeFillProperties(CQPropertyViewModel *model, const QString &path);

  //! add stroke properties
  void addStrokeProperties(CQPropertyViewModel *model, const QString &path);

  //! add fill properties
  void addFillProperties(CQPropertyViewModel *model, const QString &path);

  bool setProperties(const QString &properties);

  //! get/set property
  bool getProperty(const QString &name, QVariant &value) const;
  bool setProperty(const QString &name, const QVariant &value);

  bool getTclProperty(const QString &name, QVariant &value) const;

  bool getPropertyDesc    (const QString &name, QString  &desc, bool hidden=false) const;
  bool getPropertyType    (const QString &name, QString  &type, bool hidden=false) const;
  bool getPropertyUserType(const QString &name, QString  &type, bool hidden=false) const;
  bool getPropertyObject  (const QString &name, QObject* &obj , bool hidden=false) const;
  bool getPropertyIsHidden(const QString &name, bool &is_hidden) const;
  bool getPropertyIsStyle (const QString &name, bool &is_style) const;

  //! get property names
  virtual void getPropertyNames(QStringList &names, bool hidden=false) const;

  CQPropertyViewModel *propertyModel() const;

  const CQPropertyViewItem *propertyItem(const QString &name, bool hidden=false) const;

  //---

  //! is point inside (also checks visible)
  bool contains(const Point &p) const override;

  //! is point inside
  virtual bool inside(const Point &p) const;

  bool intersects(const BBox &r, bool inside) const;

  //---

  virtual void mousePress  (const Point &, SelMod) { }
  virtual void mouseMove   (const Point &) { }
  virtual void mouseRelease(const Point &) { }

  //---

  //! interp color
  QColor interpColor(const CQChartsColor &c, const ColorInd &ind) const;

  //---

  //! handle select press
  bool selectPress(const Point &p, SelMod selMod) override;

  //---

  // implement edit interface
  bool editPress (const Point &) override;
  bool editMove  (const Point &) override;
  bool editMotion(const Point &) override;

  void editMoveBy(const Point &d) override;

  //---

  virtual void flip(Qt::Orientation) { }

  //---

  bool isDisableSignals() const { return disableSignals_; }
  void setDisableSignals(bool b) { disableSignals_ = b; }

  //---

  //! handle box obj data changed
  void boxDataInvalidate() override { emitDataChanged(); }

  void invalidate();

  //---

  //! draw
  virtual void draw(PaintDevice *device);

  virtual void drawInit(PaintDevice *device);
  virtual void drawTerm(PaintDevice *device);

  //---

  //! write custom SVG html
  virtual void writeHtml(HtmlPaintDevice *) { }

  //! write tcl script
  void write(std::ostream &os, const QString &parentVarName="",
             const QString &varName="") const;

  //! write details (command to recreate)
  virtual void writeDetails(std::ostream &os, const QString &parentVarName="",
                            const QString &varName="") const = 0;

  //! write key values
  void writeKeys(std::ostream &os, const QString &cmd, const QString &parentVarName="",
                 const QString &varName="") const;

  //! write polygon points
  void writePoints(std::ostream &os, const Polygon &polygon) const;

  //! write properties
  void writeProperties(std::ostream &os, const QString &varName="") const;

  //---

  //! initialize state when first resized to explicit rectangle in edit
  virtual void initRectangle();

 signals:
  //! emitted when data changed
  void dataChanged();

  //! emitted when annotation pressed
  void pressed(const QString &id);

 protected slots:
  void emitDataChanged();

  void invalidateSlot() { invalidate(); }

 protected:
  void init(int &lastInd);

 protected:
  Type         type_               { Type::NONE }; //!< type
  int          ind_                { 0 };          //!< unique ind
  Group*       group_              { nullptr };    //!< parent group
  bool         enabled_            { true };       //!< is enabled
  bool         checkable_          { false };      //!< is checkable
  bool         checked_            { false };      //!< is checked
  double       disabledLighter_    { 0.8 };        //!< disabled lighter
  double       uncheckedLighter_   { 0.5 };        //!< unchecked lighter
  BBox         annotationBBox_;                    //!< bbox (plot coords) (remove ?)
  mutable bool disableSignals_     { false };      //!< disable signals
};

//---

/*!
 * \brief Annotation Group class
 * \ingroup Charts
 */
class CQChartsAnnotationGroup : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(Qt::Orientation orientation   READ orientation   WRITE setOrientation  )
  Q_PROPERTY(Qt::Alignment   alignment     READ alignment     WRITE setAlignment    )
  Q_PROPERTY(int             layoutMargin  READ layoutMargin  WRITE setLayoutMargin )
  Q_PROPERTY(int             layoutSpacing READ layoutSpacing WRITE setLayoutSpacing)

 public:
  CQChartsAnnotationGroup(View *view);
  CQChartsAnnotationGroup(Plot *plot);

 ~CQChartsAnnotationGroup();

  const char *typeName() const override { return "group"; }

  const char *propertyName() const override { return "annotationGroup"; }

  const char *cmdName() const override { return "create_charts_annotation_group"; }

  //---

  const Qt::Orientation &orientation() const { return orientation_; }
  void setOrientation(const Qt::Orientation &o) { orientation_ = o; invalidateLayout(); }

  const Qt::Alignment &alignment() const { return align_; }
  void setAlignment(const Qt::Alignment &a) { align_ = a; invalidateLayout(); }

  int layoutMargin() const { return layoutMargin_; }
  void setLayoutMargin(int i) { layoutMargin_ = i; invalidateLayout(); }

  int layoutSpacing() const { return layoutSpacing_; }
  void setLayoutSpacing(int i) { layoutSpacing_ = i; invalidateLayout(); }

  //---

  void addAnnotation(CQChartsAnnotation *annotation);
  void removeAnnotation(CQChartsAnnotation *annotation);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  bool inside(const Point &p) const override;

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  void layout();
  void invalidateLayout() { needsLayout_ = true; invalidate(); }

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  using Annotations = std::vector<CQChartsAnnotation *>;

  Annotations     annotations_;
  Qt::Orientation orientation_   { Qt::Horizontal };
  Qt::Alignment   align_         { Qt::AlignHCenter | Qt::AlignVCenter };
  int             layoutMargin_  { 2 }; //!< layout margin in pixel
  int             layoutSpacing_ { 2 }; //!< layout spacing in pixel
  mutable bool    needsLayout_   { true };
};

//---

/*!
 * \brief Annotation Polygon Shape class
 * \ingroup Charts
 */
class CQChartsPolyShapeAnnotation : public CQChartsAnnotation {
  Q_OBJECT

 public:
  using Polygon = CQChartsPolygon;

 public:
  CQChartsPolyShapeAnnotation(View *view, Type type, const Polygon &polygon=Polygon());
  CQChartsPolyShapeAnnotation(Plot *plot, Type type, const Polygon &polygon=Polygon());

  const Polygon &polygon() const { return polygon_; }
  void setPolygon(const Polygon &polygon) { polygon_ = polygon; emit dataChanged(); }

 protected:
  Polygon polygon_; //!< polygon points
};

//---

/*!
 * \brief rectangle annotation
 * \ingroup Charts
 *
 * Filled and/or Stroked rectangle with optional rounded corners
 */
class CQChartsRectangleAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsRect     rectangle READ rectangle WRITE setRectangle)
  Q_PROPERTY(CQChartsPosition start     READ start     WRITE setStart    )
  Q_PROPERTY(CQChartsPosition end       READ end       WRITE setEnd      )
  Q_PROPERTY(ShapeType        shapeType READ shapeType WRITE setShapeType)
  Q_PROPERTY(int              numSides  READ numSides  WRITE setNumSides )

  Q_ENUMS(ShapeType)

 public:
  enum class ShapeType {
    NONE          = (int) CQChartsAnnotation::ShapeType::NONE,
    TRIANGLE      = (int) CQChartsAnnotation::ShapeType::TRIANGLE,
    DIAMOND       = (int) CQChartsAnnotation::ShapeType::DIAMOND,
    BOX           = (int) CQChartsAnnotation::ShapeType::BOX,
    POLYGON       = (int) CQChartsAnnotation::ShapeType::POLYGON,
    CIRCLE        = (int) CQChartsAnnotation::ShapeType::CIRCLE,
    DOUBLE_CIRCLE = (int) CQChartsAnnotation::ShapeType::DOUBLE_CIRCLE
  };

 public:
  using Rect     = CQChartsRect;
  using Position = CQChartsPosition;

 public:
  CQChartsRectangleAnnotation(View *view, const Rect &rect=Rect());
  CQChartsRectangleAnnotation(Plot *plot, const Rect &rect=Rect());

  virtual ~CQChartsRectangleAnnotation();

  //---

  const char *typeName() const override { return "rectangle"; }

  const char *propertyName() const override { return "rectangleAnnotation"; }

  const char *cmdName() const override { return "create_charts_rectangle_annotation"; }

  //---

  //! get/set rectangle
  const Rect &rectangle() const { return rectangle_; }
  void setRectangle(const Rect &rectangle);
  void setRectangle(const Position &start, const Position &end);

  //! get/set start
  Position start() const;
  void setStart(const Position &p);

  //! get/set end
  Position end() const;
  void setEnd(const Position &p);

  //! get/set shape
  const ShapeType &shapeType() const { return shapeType_; }
  void setShapeType(const ShapeType &s);

  //! get/set number of sides
  int numSides() const { return numSides_; }
  void setNumSides(int n);

  //---

  bool intersectShape(const Point &p1, const Point &p2, Point &pi) const override;

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  Rect      rectangle_;                     //!< rectangle
  ShapeType shapeType_ { ShapeType::NONE }; //!< shape type
  int       numSides_ { -1 };               //!< number of sides
};

//---

/*!
 * \brief ellipse annotation
 * \ingroup Charts
 *
 * Filled and/or Stroked ellipse/circle
 */
class CQChartsEllipseAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPosition center  READ center  WRITE setCenter )
  Q_PROPERTY(CQChartsLength   xRadius READ xRadius WRITE setXRadius)
  Q_PROPERTY(CQChartsLength   yRadius READ yRadius WRITE setYRadius)

 public:
  using Position = CQChartsPosition;
  using Length   = CQChartsLength;
  using Units    = CQChartsUnits;

 public:
  CQChartsEllipseAnnotation(View *view, const Position &center=Position(),
                            const Length &xRadius=Length(1.0, Units::PLOT),
                            const Length &yRadius=Length(1.0, Units::PLOT));
  CQChartsEllipseAnnotation(Plot *plot, const Position &center=Position(),
                            const Length &xRadius=Length(1.0, Units::PLOT),
                            const Length &yRadius=Length(1.0, Units::PLOT));

  virtual ~CQChartsEllipseAnnotation();

  //---

  const char *typeName() const override { return "ellipse"; }

  const char *propertyName() const override { return "ellipseAnnotation"; }

  const char *cmdName() const override { return "create_charts_ellipse_annotation"; }

  //---

  const Position &center() const { return center_; }
  void setCenter(const Position &c) { center_ = c; emit dataChanged(); }

  const Length &xRadius() const { return xRadius_; }
  void setXRadius(const Length &r) { xRadius_ = r; }

  const Length &yRadius() const { return yRadius_; }
  void setYRadius(const Length &r) { yRadius_ = r; }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  Position center_;                       //!< ellipse center
  Length   xRadius_ { 1.0, Units::PLOT }; //!< ellipse x radius
  Length   yRadius_ { 1.0, Units::PLOT }; //!< ellipse y radius
};

//---

/*!
 * \brief polygon annotation
 * \ingroup Charts
 *
 * Filled and/or Stroked polygon. Lines can be rounded
 */
class CQChartsPolygonAnnotation : public CQChartsPolyShapeAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPolygon polygon      READ polygon        WRITE setPolygon     )
  Q_PROPERTY(bool            roundedLines READ isRoundedLines WRITE setRoundedLines)

 public:
  CQChartsPolygonAnnotation(View *view, const Polygon &polygon);
  CQChartsPolygonAnnotation(Plot *plot, const Polygon &polygon);

  virtual ~CQChartsPolygonAnnotation();

  //---

  const char *typeName() const override { return "polygon"; }

  const char *propertyName() const override { return "polygonAnnotation"; }

  const char *cmdName() const override { return "create_charts_polygon_annotation"; }

  //---

  bool isRoundedLines() const { return roundedLines_; }
  void setRoundedLines(bool b);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

  void initSmooth() const;

 private:
  using Smooth = CQChartsSmooth;

  bool    roundedLines_ { false };   //!< draw rounded (smooth) lines
  Smooth* smooth_       { nullptr }; //!< smooth object
};

//---

/*!
 * \brief polyline annotation
 * \ingroup Charts
 *
 * TODO: draw points and/or line
 *
 * Stroked polyline. Lines can be rounded
 */
class CQChartsPolylineAnnotation : public CQChartsPolyShapeAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPolygon polygon      READ polygon        WRITE setPolygon     )
  Q_PROPERTY(bool            roundedLines READ isRoundedLines WRITE setRoundedLines)

 public:
  CQChartsPolylineAnnotation(View *view, const Polygon &polygon);
  CQChartsPolylineAnnotation(Plot *plot, const Polygon &polygon);

  virtual ~CQChartsPolylineAnnotation();

  //---

  const char *typeName() const override { return "polyline"; }

  const char *propertyName() const override { return "polylineAnnotation"; }

  const char *cmdName() const override { return "create_charts_polyline_annotation"; }

  //---

  bool isRoundedLines() const { return roundedLines_; }
  void setRoundedLines(bool b);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

  void initSmooth() const;

 private:
  using Smooth = CQChartsSmooth;

  bool    roundedLines_ { false };   //!< draw rounded (smooth) lines
  Smooth* smooth_       { nullptr }; //!< smooth object
};

//---

/*!
 * \brief text annotation
 * \ingroup Charts
 *
 * Formatted text in optionally filled and/or stroked box.
 * Text can be drawn at any angle.
 */
class CQChartsTextAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsOptPosition position  READ position  WRITE setPosition )
  Q_PROPERTY(CQChartsOptRect     rectangle READ rectangle WRITE setRectangle)

 public:
  using Rect        = CQChartsRect;
  using OptRect     = CQChartsOptRect;
  using Position    = CQChartsPosition;
  using OptPosition = CQChartsOptPosition;

 public:
  CQChartsTextAnnotation(View *view, const Position &p=Position(), const QString &text="");
  CQChartsTextAnnotation(Plot *plot, const Position &p=Position(), const QString &text="");

  CQChartsTextAnnotation(View *view, const Rect &r=Rect(), const QString &text="");
  CQChartsTextAnnotation(Plot *plot, const Rect &r=Rect(), const QString &text="");

  virtual ~CQChartsTextAnnotation();

  //---

  const char *typeName() const override { return "text"; }

  const char *propertyName() const override { return "textAnnotation"; }

  const char *cmdName() const override { return "create_charts_text_annotation"; }

  //---

  const OptPosition &position() const { return position_; }
  void setPosition(const OptPosition &p);

  Position positionValue() const;
  void setPosition(const Position &p);

  const OptRect &rectangle() const { return rectangle_; }
  void setRectangle(const OptRect &r);

  Rect rectangleValue() const;
  void setRectangle(const Rect &r);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

  //---

  void initRectangle() override;

 private:
  void init(const QString &text);

  void calcTextSize(Size &psize, Size &wsize) const;

  void positionToLL(double w, double h, double &x, double &y) const;

  void rectToBBox();

  void positionToBBox();

 private:
  OptPosition position_;  //!< text position
  OptRect     rectangle_; //!< text bounding rect
};

//---

/*!
 * \brief image annotation
 * \ingroup Charts
 *
 * Image in rectangle
 */
class CQChartsImageAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsOptPosition position      READ position      WRITE setPosition     )
  Q_PROPERTY(CQChartsOptRect     rectangle     READ rectangle     WRITE setRectangle    )
  Q_PROPERTY(CQChartsImage       image         READ image         WRITE setImage        )
  Q_PROPERTY(CQChartsImage       disabledImage READ disabledImage WRITE setDisabledImage)

 public:
  using Image       = CQChartsImage;
  using Rect        = CQChartsRect;
  using OptRect     = CQChartsOptRect;
  using Position    = CQChartsPosition;
  using OptPosition = CQChartsOptPosition;

 public:
  CQChartsImageAnnotation(View *view, const Position &p=Position(), const Image &image=Image());
  CQChartsImageAnnotation(Plot *plot, const Position &p=Position(), const Image &image=Image());

  CQChartsImageAnnotation(View *view, const Rect &r=Rect(), const Image &image=Image());
  CQChartsImageAnnotation(Plot *plot, const Rect &r=Rect(), const Image &image=Image());

  virtual ~CQChartsImageAnnotation();

  //---

  const char *typeName() const override { return "image"; }

  const char *propertyName() const override { return "imageAnnotation"; }

  const char *cmdName() const override { return "create_charts_image_annotation"; }

  //---

  const OptPosition &position() const { return position_; }
  void setPosition(const OptPosition &p);

  Position positionValue() const;
  void setPosition(const Position &p);

  const OptRect &rectangle() const { return rectangle_; }
  void setRectangle(const OptRect &r);

  Rect rectangleValue() const;
  void setRectangle(const Rect &r);

  const Image &image() const { return image_; }
  void setImage(const Image &image);

  const Image &disabledImage() const { return disabledImage_; }
  void setDisabledImage(const Image &image);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

  //---

  void initRectangle() override;

 private:
  enum class DisabledImageType {
    NONE,
    DISABLED,
    UNCHECKED,
    FIXED
  };

  void init();

  void calcImageSize(Size &psize, Size &wsize) const;

  void updateDisabledImage(const DisabledImageType &type);

  void positionToLL(double w, double h, double &x, double &y) const;

  void rectToBBox();

  void positionToBBox();

 private:
  OptPosition       position_;                                      //!< image position
  OptRect           rectangle_;                                     //!< image bounding rectangle
  Image             image_;                                         //!< image
  Image             disabledImage_;                                 //!< disabled image
  DisabledImageType disabledImageType_ { DisabledImageType::NONE }; //!< disabled image type
};

//---

class CQChartsArrow;

/*!
 * \brief arrow annotation
 * \ingroup Charts
 *
 * Arrow with custom end point arrows on a filled and/or stroked line
 */
class CQChartsArrowAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPosition start       READ start       WRITE setStart      )
  Q_PROPERTY(CQChartsObjRef   startObjRef READ startObjRef WRITE setStartObjRef)
  Q_PROPERTY(CQChartsPosition end         READ end         WRITE setEnd        )
  Q_PROPERTY(CQChartsObjRef   endObjRef   READ endObjRef   WRITE setEndObjRef  )

 public:
  using Position = CQChartsPosition;
  using ObjRef   = CQChartsObjRef;

 public:
  CQChartsArrowAnnotation(View *view, const Position &start=Position(),
                          const Position &end=Position(Point(1, 1)));
  CQChartsArrowAnnotation(Plot *plot, const Position &start=Position(),
                          const Position &end=Position(Point(1, 1)));

  virtual ~CQChartsArrowAnnotation();

  //---

  const char *typeName() const override { return "arrow"; }

  const char *propertyName() const override { return "arrowAnnotation"; }

  const char *cmdName() const override { return "create_charts_arrow_annotation"; }

  //---

  const Position &start() const { return start_; }
  void setStart(const Position &p) { start_ = p; emit dataChanged(); }

  const ObjRef &startObjRef() const { return startObjRef_; }
  void setStartObjRef(const ObjRef &o) { startObjRef_ = o; }

  const Position &end() const { return end_; }
  void setEnd(const Position &p) { end_ = p; emit dataChanged(); }

  const ObjRef &endObjRef() const { return endObjRef_; }
  void setEndObjRef(const ObjRef &o) { endObjRef_ = o; }

  //---

  CQChartsArrow *arrow() const { return arrow_.get(); }

  const CQChartsArrowData &arrowData() const;
  void setArrowData(const CQChartsArrowData &data);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  void getPropertyNames(QStringList &names, bool hidden) const override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  void flip(Qt::Orientation orient) override;

  //---

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  using ArrowP = std::unique_ptr<CQChartsArrow>;

  Position start_       { Point(0, 0) }; //!< arrow start
  ObjRef   startObjRef_;                 //!< arrow start reference object
  Position end_         { Point(1, 1) }; //!< arrow end
  ObjRef   endObjRef_;                   //!< arrow end reference object
  ArrowP   arrow_;                       //!< arrow data
};

//---

/*!
 * \brief point annotation
 * \ingroup Charts
 *
 * Symbol drawn at point
 */
class CQChartsPointAnnotation : public CQChartsAnnotation,
 public CQChartsObjPointData<CQChartsPointAnnotation> {
  Q_OBJECT

  Q_PROPERTY(CQChartsPosition position READ position WRITE setPosition)

  CQCHARTS_POINT_DATA_PROPERTIES

 public:
  using Position = CQChartsPosition;
  using Symbol   = CQChartsSymbol;

 public:
  CQChartsPointAnnotation(View *view, const Position &p=Position(),
                          const Symbol &type=Symbol::Type::CIRCLE);
  CQChartsPointAnnotation(Plot *plot, const Position &p=Position(),
                          const Symbol &type=Symbol::Type::CIRCLE);

  virtual ~CQChartsPointAnnotation();

  //---

  const char *typeName() const override { return "point"; }

  const char *propertyName() const override { return "pointAnnotation"; }

  const char *cmdName() const override { return "create_charts_point_annotation"; }

  //---

  const Position &position() const { return position_; }
  void setPosition(const Position &p) { position_ = p; emit dataChanged(); }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  Position position_;                                //!< point position
  Symbol   type_     { CQChartsSymbol::Type::NONE }; //!< symbol type
};

//---

/*!
 * \brief pie slice annotation
 * \ingroup Charts
 *
 * Arc between two angles at a radius with optional inner radius
 */
class CQChartsPieSliceAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPosition position    READ position    WRITE setPosition   )
  Q_PROPERTY(CQChartsLength   innerRadius READ innerRadius WRITE setInnerRadius)
  Q_PROPERTY(CQChartsLength   outerRadius READ outerRadius WRITE setOuterRadius)
  Q_PROPERTY(CQChartsAngle    startAngle  READ startAngle  WRITE setStartAngle )
  Q_PROPERTY(CQChartsAngle    spanAngle   READ spanAngle   WRITE setSpanAngle  )

 public:
  using Position = CQChartsPosition;
  using Length   = CQChartsLength;
  using Angle    = CQChartsAngle;

 public:
  CQChartsPieSliceAnnotation(View *view, const Position &p=Position(),
                             const Length &innerRadius=Length(),
                             const Length &outerRadius=Length(),
                             const Angle &startAngle=Angle(0.0),
                             const Angle &spanAngle=Angle(90.0));
  CQChartsPieSliceAnnotation(Plot *plot, const Position &p=Position(),
                             const Length &innerRadius=Length(),
                             const Length &outerRadius=Length(),
                             const Angle &startAngle=Angle(0.0),
                             const Angle &spanAngle=Angle(90.0));

  virtual ~CQChartsPieSliceAnnotation();

  //---

  const char *typeName() const override { return "pie_slice"; }

  const char *propertyName() const override { return "pieSliceAnnotation"; }

  const char *cmdName() const override { return "create_charts_pie_slice_annotation"; }

  //---

  const Position &position() const { return position_; }
  void setPosition(const Position &p) { position_ = p; emit dataChanged(); }

  const Length &innerRadius() const { return innerRadius_; }
  void setInnerRadius(const Length &r) { innerRadius_ = r; }

  const Length &outerRadius() const { return outerRadius_; }
  void setOuterRadius(const Length &r) { outerRadius_ = r; }

  const Angle &startAngle() const { return startAngle_; }
  void setStartAngle(const Angle &a) { startAngle_ = a; }

  const Angle &spanAngle() const { return spanAngle_; }
  void setSpanAngle(const Angle &a) { spanAngle_ = a; }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  Position position_;             //!< point position
  Length   innerRadius_;          //!< inner radius
  Length   outerRadius_;          //!< outer radius
  Angle    startAngle_  {  0.0 }; //!< start angle
  Angle    spanAngle_   { 90.0 }; //!< span angle
};

//---

/*!
 * \brief axis annotation
 * \ingroup Charts
 */
class CQChartsAxisAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(double position READ position WRITE setPosition)
  Q_PROPERTY(double start    READ start    WRITE setStart   )
  Q_PROPERTY(double end      READ end      WRITE setEnd     )

 public:
  using Axis = CQChartsAxis;

 public:
  CQChartsAxisAnnotation(Plot *plot, Qt::Orientation direction=Qt::Horizontal,
                         double start=0.0, double end=1.0);

  virtual ~CQChartsAxisAnnotation();

  //---

  const char *typeName() const override { return "axis"; }

  const char *propertyName() const override { return "axisAnnotation"; }

  const char *cmdName() const override { return "create_charts_axis_annotation"; }

  //---

  Axis *axis() const { return axis_; }

  //---

  //! get/set position
  double position() const;
  void setPosition(double r);

  //! get/set start
  double start() const;
  void setStart(double r);

  //! get/set end
  double end() const;
  void setEnd(double r);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private:
  Axis *axis_ { nullptr };
};

//---

/*!
 * \brief key annotation
 * \ingroup Charts
 */
class CQChartsKeyAnnotation : public CQChartsAnnotation {
  Q_OBJECT

 public:
  using Key = CQChartsKey;

 public:
  CQChartsKeyAnnotation(View *plot);
  CQChartsKeyAnnotation(Plot *plot, const CQChartsColumn &column=CQChartsColumn());

  virtual ~CQChartsKeyAnnotation();

  //---

  const char *typeName() const override { return "key"; }

  const char *propertyName() const override { return "keyAnnotation"; }

  const char *cmdName() const override { return "create_charts_key_annotation"; }

  //---

  Key *key() const { return key_; }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  bool selectPress(const Point &p, SelMod selMod) override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void init();

 private slots:
  void updateLocationSlot();

 private:
  Key *key_ { nullptr };
};

//---

/*!
 * \brief point set annotation
 * \ingroup Charts
 *
 * Set of points draw as symbols, convex hull, best fit line, density gradient or density grid
 *
 * TODO: support column
 */
class CQChartsPointSetAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPoints values   READ values   WRITE setValues  )
  Q_PROPERTY(DrawType       drawType READ drawType WRITE setDrawType)

  Q_ENUMS(DrawType)

 public:
  enum class DrawType {
    SYMBOLS,
    HULL,
    BEST_FIT,
    DENSITY,
    GRID
  };

 public:
  using Points = CQChartsPoints;

 public:
  CQChartsPointSetAnnotation(View *view, const Points &values=Points());
  CQChartsPointSetAnnotation(Plot *plot, const Points &values=Points());

  virtual ~CQChartsPointSetAnnotation();

  //---

  const char *typeName() const override { return "point_set"; }

  const char *propertyName() const override { return "pointSetAnnotation"; }

  const char *cmdName() const override { return "create_charts_point_set_annotation"; }

  //---

  const Points &values() const { return values_; }
  void setValues(const Points &values) { values_ = values; updateValues(); }

  const DrawType &drawType() const { return drawType_; }
  void setDrawType(const DrawType &t) { drawType_ = t; invalidate(); }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void updateValues();

 private:
  void init();

 private:
  using Hull     = CQChartsGrahamHull;
  using GridCell = CQChartsGridCell;

  Points   values_;                         //!< point values
  Hull     hull_;                           //!< hull
  RMinMax  xrange_;                         //!< x range
  RMinMax  yrange_;                         //!< y range
  GridCell gridCell_;                       //!< grid cell data
  DrawType drawType_ { DrawType::SYMBOLS }; //!< draw type
};

//---

/*!
 * \brief value set annotation
 * \ingroup Charts
 *
 * Set of values draw as statistics or error bar
 */
class CQChartsValueSetAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsRect  rectangle READ rectangle WRITE setRectangle)
  Q_PROPERTY(CQChartsReals values    READ values    WRITE setValues   )

 public:
  using Rect  = CQChartsRect;
  using Reals = CQChartsReals;

 public:
  CQChartsValueSetAnnotation(View *view, const Rect &rectangle=Rect(), const Reals &values=Reals());
  CQChartsValueSetAnnotation(Plot *plot, const Rect &rectangle=Rect(), const Reals &values=Reals());

  virtual ~CQChartsValueSetAnnotation();

  //---

  const char *typeName() const override { return "value_set"; }

  const char *propertyName() const override { return "valueSetAnnotation"; }

  const char *cmdName() const override { return "create_charts_value_set_annotation"; }

  //---

  const Rect &rectangle() const { return rectangle_; }
  void setRectangle(const Rect &rectangle) { rectangle_ = rectangle; emit dataChanged(); }

  const Reals &values() const { return values_; }
  void setValues(const Reals &values) { values_ = values; updateValues(); }

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

 private:
  void updateValues();

 private:
  void init();

 private:
  using Density = CQChartsDensity;

  Rect     rectangle_;           //!< rectangle
  Reals    values_;              //!< real values
  Density* density_ { nullptr }; //!< density object
};

//---

/*!
 * \brief button annotation
 * \ingroup Charts
 *
 * Button with text
 */
class CQChartsButtonAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsPosition position READ position WRITE setPosition)

 public:
  using Position = CQChartsPosition;

 public:
  CQChartsButtonAnnotation(View *view, const Position &p=Position(), const QString &text="");
  CQChartsButtonAnnotation(Plot *plot, const Position &p=Position(), const QString &text="");

  virtual ~CQChartsButtonAnnotation();

  //---

  const char *typeName() const override { return "button"; }

  const char *propertyName() const override { return "buttonAnnotation"; }

  const char *cmdName() const override { return "create_charts_button_annotation"; }

  //---

  const Position &position() const { return position_; }
  void setPosition(const Position &p);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void mousePress  (const Point &w, SelMod) override;
  void mouseMove   (const Point &w) override;
  void mouseRelease(const Point &w) override;

  //---

  bool inside(const Point &p) const override;

  void draw(PaintDevice *device) override;

  //---

  void writeHtml(HtmlPaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

  //---

 private:
  void init(const QString &text);

  QRect calcPixelRect() const;

 private:
  Position position_;          //!< button position
  QRect    prect_;             //!< pixel rect
  bool     pressed_ { false }; //!< is pressed
};

//---

class CQWinWidget;

/*!
 * \brief widget annotation
 * \ingroup Charts
 *
 * Widget at position or in rectangle
 */
class CQChartsWidgetAnnotation : public CQChartsAnnotation {
  Q_OBJECT

  Q_PROPERTY(CQChartsOptPosition position    READ position      WRITE setPosition   )
  Q_PROPERTY(CQChartsOptRect     rectangle   READ rectangle     WRITE setRectangle  )
  Q_PROPERTY(CQChartsWidget      widget      READ widget        WRITE setWidget     )
  Q_PROPERTY(Qt::Alignment       align       READ align         WRITE setAlign      )
  Q_PROPERTY(QSizePolicy         sizePolicy  READ sizePolicy    WRITE setSizePolicy )
  Q_PROPERTY(bool                interactive READ isInteractive WRITE setInteractive)

 public:
  using Widget      = CQChartsWidget;
  using Rect        = CQChartsRect;
  using OptRect     = CQChartsOptRect;
  using Position    = CQChartsPosition;
  using OptPosition = CQChartsOptPosition;

 public:
  CQChartsWidgetAnnotation(View *view, const Position &p=Position(), const Widget &widget=Widget());
  CQChartsWidgetAnnotation(Plot *plot, const Position &p=Position(), const Widget &widget=Widget());

  CQChartsWidgetAnnotation(View *view, const Rect &r=Rect(), const Widget &widget=Widget());
  CQChartsWidgetAnnotation(Plot *plot, const Rect &r=Rect(), const Widget &widget=Widget());

  virtual ~CQChartsWidgetAnnotation();

  //---

  const char *typeName() const override { return "widget"; }

  const char *propertyName() const override { return "widgetAnnotation"; }

  const char *cmdName() const override { return "create_charts_widget_annotation"; }

  //---

  const OptPosition &position() const { return position_; }
  void setPosition(const OptPosition &p);

  Position positionValue() const;
  void setPosition(const Position &p);

  const OptRect &rectangle() const { return rectangle_; }
  void setRectangle(const OptRect &r);

  Rect rectangleValue() const;
  void setRectangle(const Rect &r);

  const Widget &widget() const { return widget_; }
  void setWidget(const Widget &widget);

  const Qt::Alignment &align() const { return align_; }
  void setAlign(const Qt::Alignment &a);

  const QSizePolicy &sizePolicy() const { return sizePolicy_; }
  void setSizePolicy(const QSizePolicy &p);

  bool isInteractive() const { return interactive_; }
  void setInteractive(bool b);

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path,
                     const QString &desc="") override;

  //---

  void setEditBBox(const BBox &bbox, const ResizeSide &dragSide) override;

  bool inside(const Point &p) const override;

  //---

  void draw(PaintDevice *device) override;

  void writeDetails(std::ostream &os, const QString &parentVarName="",
                    const QString &varName="") const override;

  //---

  void initRectangle() override;

 private:
  void init();

  void calcWidgetSize(Size &psize, Size &wsize) const;

  void positionToTopLeft(double w, double h, double &x, double &y) const;

  void rectToBBox();

  void positionToBBox();

 private slots:
  void updateWinGeometry();

 private:
  OptPosition   position_;                                     //!< widget position
  OptRect       rectangle_;                                    //!< widget bounding rectangle
  Widget        widget_;                                       //!< widget
  Qt::Alignment align_       { Qt::AlignLeft | Qt::AlignTop }; //!< position alignment
  QSizePolicy   sizePolicy_;                                   //!< size policy
  bool          interactive_ { false };                        //!< is interactive
  CQWinWidget*  winWidget_   { nullptr };                      //!< window frame
};

#endif
