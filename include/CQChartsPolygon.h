#ifndef CQChartsPolygon_H
#define CQChartsPolygon_H

#include <CQChartsGeom.h>
#include <CQChartsUtil.h>
#include <QString>
#include <iostream>

/*!
 * \brief Polygon class
 * \ingroup Charts
 */
class CQChartsPolygon :
  public CQChartsEqBase<CQChartsPolygon>,
  public CQChartsToStringBase<CQChartsPolygon> {
 public:
  using Polygon = CQChartsGeom::Polygon;
  using Point   = CQChartsGeom::Point;
  using Units   = CQChartsUnits;

 public:
  static void registerMetaType();

  static int metaTypeId;

 public:
  CQChartsPolygon(const Units &units, const Polygon &polygon) :
   units_(units), polygon_(polygon) {
  }

  CQChartsPolygon(const Polygon &polygon=Polygon(), const Units &units=Units::PLOT) :
   units_(units), polygon_(polygon) {
  }

  CQChartsPolygon(const QString &s, const Units &units=Units::PLOT) {
    setValue(s, units);
  }

  CQChartsPolygon(const CQChartsPolygon &rhs) :
    units_(rhs.units_), polygon_(rhs.polygon_) {
  }

  CQChartsPolygon &operator=(const CQChartsPolygon &rhs) {
    units_   = rhs.units_;
    polygon_ = rhs.polygon_;

    return *this;
  }

  //---

  bool isValid(bool closed=false) const {
    return (closed ? polygon_.size() >= 3 : polygon_.size() >= 2);
  }

  //---

  const Units &units() const { return units_; }
  void setUnits(const Units &units) { units_ = units; }

  bool hasUnits() const { return units_ != Units::NONE; }

  //---

  const Polygon &polygon() const { return polygon_; }

  int numPoints() const { return polygon_.size(); }

  Point point(int i) const { return polygon_.point(i); }

  void setPoint(int i, const Point &p) { polygon_.setPoint(i, p); }

  void addPoint(const Point &p) { polygon_.addPoint(p); }

  void removePoint() { polygon_.removePoint(); }

  //---

  void setValue(const Units &units, const Polygon &polygon) {
    units_   = units;
    polygon_ = polygon;
  }

  bool setValue(const QString &str, const Units &defUnits=Units::PLOT) {
    Units   units;
    Polygon polygon;

    if (! decodeString(str, units, polygon, defUnits))
      return false;

    units_   = units;
    polygon_ = polygon;

    return true;
  }

  //---

  QString toString() const {
    if (! isValid())
      return "";

    //---

    QString ustr = CQChartsUtil::unitsString(units_);

    QString str;

    for (int i = 0; i < polygon_.size(); ++i) {
      if (i > 0) str += " ";

      str += QString("%1 %2").arg(polygon_.point(i).x).arg(polygon_.point(i).y);
    }

    str += " " + ustr;

    return str;
  }

  bool fromString(const QString &s) {
    return setValue(s);
  }

  //---

  friend bool operator==(const CQChartsPolygon &lhs, const CQChartsPolygon &rhs) {
    if (lhs.units_   != rhs.units_  ) return false;
    if (lhs.polygon_ != rhs.polygon_) return false;

    return true;
  }

  //---

 private:
  static bool decodeString(const QString &str, Units &units, Polygon &polygon,
                           const Units &defUnits=Units::PLOT);

 private:
  Units   units_ { Units::PIXEL };
  Polygon polygon_;
};

//---

#include <CQUtilMeta.h>

CQUTIL_DCL_META_TYPE(CQChartsPolygon)

#endif
