#include <CQChartsColorStops.h>
#include <CQPropertyView.h>
#include <CQTclUtil.h>

CQUTIL_DEF_META_TYPE(CQChartsColorStops, toString, fromString)

int CQChartsColorStops::metaTypeId;

void
CQChartsColorStops::
registerMetaType()
{
  metaTypeId = CQUTIL_REGISTER_META(CQChartsColorStops);

  CQPropertyViewMgrInst->setUserName("CQChartsColorStops", "color_stops");
}

QString
CQChartsColorStops::
toString() const
{
  QStringList strs;

  for (const auto &v : values_)
    strs << QString::number(v);

  if (units() == Units::PERCENT)
    strs << "%";

  return CQTcl::mergeList(strs);
}

bool
CQChartsColorStops::
fromString(const QString &str)
{
  QStringList strs;

  if (! CQTcl::splitList(str, strs))
    return false;

  Units  units = Units::ABSOLUTE;
  Values values;

  bool ok = true;

  for (int i = 0; i < strs.length(); ++i) {
    const auto &s = strs[i];

    if (s == "%")
      units = Units::PERCENT;

    bool ok1;

    double v = s.toDouble(&ok1);

    if (ok1)
      values.push_back(v);
    else
      ok = false;
  }

  units_  = units;
  values_ = values;

  return ok;
}

int
CQChartsColorStops::
cmp(const CQChartsColorStops &s) const
{
  if (units_ != s.units_) {
    return (units_ == Units::ABSOLUTE ? -1 : 1);
  }

  int n1 =   values_.size();
  int n2 = s.values_.size();

  if (n1 != n2)
    return n1 - n2;

  for (int i = 0; i < n1; ++i) {
    if (values_[i] == s.values_[i])
      continue;

    return (values_[i] < s.values_[i] ? -1 : 1);
  }

  return 0;
}
