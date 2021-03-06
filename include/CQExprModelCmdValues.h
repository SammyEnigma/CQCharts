#ifndef CQExprModelCmdValues_H
#define CQExprModelCmdValues_H

#include <QVariant>
#include <vector>

class CQExprModelCmdValues {
 public:
  using Values = std::vector<QVariant>;

 public:
  CQExprModelCmdValues(const Values &values) :
   values_(values) {
    eind_ = numValues() - 1;
  }

  int ind() const { return ind_; }

  int numValues() const { return values_.size(); }

  QVariant popValue() { QVariant value = values_.back(); values_.pop_back(); return value; }

  bool getInt(int &i) {
    if (ind_ > eind_) return false;

    bool ok;

    int i1 = values_[ind_].toInt(&ok);

    if (ok) {
      i = i1;

      ++ind_;
    }

    return ok;
  }

  bool getReal(double &r) {
    if (ind_ > eind_) return false;

    bool ok;

    double r1 = values_[ind_].toDouble(&ok);

    if (ok) {
      r = r1;

      ++ind_;
    }

    return ok;
  }

  bool getStr(QString &s) {
    if (ind_ > eind_) return false;

    s = values_[ind_++].toString();

    return true;
  }

  //---

 private:
  Values values_;
  int    ind_  { 0 };
  int    eind_ { 0 };
};

#endif
