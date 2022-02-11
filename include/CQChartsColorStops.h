#ifndef CQChartsColorStops_H
#define CQChartsColorStops_H

#include <CQChartsTmpl.h>
#include <CQUtilMeta.h>
#include <QStringList>
#include <vector>

/*!
 * \brief class to contain color stop values
 * \ingroup Charts
 */
class CQChartsColorStops :
  public CQChartsComparatorBase<CQChartsColorStops> {
 public:
  static void registerMetaType();

  static int metaTypeId;

  //---

  CQUTIL_DEF_META_CONVERSIONS(CQChartsColorStops, metaTypeId)

 public:
  enum class Units {
    ABSOLUTE,
    PERCENT
  };

  using Values = std::vector<double>;

 public:
  //! default constructor
  CQChartsColorStops() = default;

  //! construct from values
  explicit CQChartsColorStops(const Values &values) :
   values_(values) {
  }

  //! color from string
  explicit CQChartsColorStops(const QString &str) {
    fromString(str);
  }

  //---

  //! get/set values
  const Values &values() const { return values_; }
  void setValues(const Values &values) { values_ = values; }

  //! get.set units
  const Units &units() const { return units_; }
  void setUnits(const Units &v) { units_ = v; }

  bool isPercent() const { return (units() == Units::PERCENT); }

  //---

  int size() const { return int(values_.size()); }

  //---

  bool isValid() const { return ! values_.empty(); }

  //---

  //! get value index
  int ind(double v) const {
    int n = size();

    if (n == 0)
      return -1;

    for (int i = 0; i < n; ++i) {
      if (v < values_[size_t(i)])
        return i;
    }

    return n;
  }

  //---

  //! color to/from string for QVariant
  QString toString() const;
  bool fromString(const QString &s);

  //---

  int cmp(const CQChartsColorStops &s) const;

  friend int cmp(const CQChartsColorStops &s1, const CQChartsColorStops &s2) {
    return s1.cmp(s2);
  }

  //---

 private:
  Units  units_  { Units::ABSOLUTE }; //!< units
  Values values_;                     //!< stop values
};

//---

CQUTIL_DCL_META_TYPE(CQChartsColorStops)

#endif
