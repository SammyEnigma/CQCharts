#ifndef CQChartsObj_H
#define CQChartsObj_H

#include <CQChartsGeom.h>
#include <QObject>
#include <boost/optional.hpp>

class CQChartsObj : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString            id       READ id         WRITE setId      )
  Q_PROPERTY(CQChartsGeom::BBox rect     READ rect       WRITE setRect    )
  Q_PROPERTY(QString            tipId    READ tipId      WRITE setTipId   )
  Q_PROPERTY(bool               selected READ isSelected WRITE setSelected)
  Q_PROPERTY(bool               inside   READ isInside   WRITE setInside  )

 public:
  CQChartsObj(QObject *parent=nullptr, const CQChartsGeom::BBox &rect=CQChartsGeom::BBox());

  //---

  // unique id of object
  const QString &id() const;
  void setId(const QString &s) { id_ = s; }

  // calculate unique id of object (on demand)
  virtual QString calcId() const { return ""; }

  //---

  const CQChartsGeom::BBox &rect() const { return rect_; }
  void setRect(const CQChartsGeom::BBox &r) { rect_ = r; }

  //---

  // tip id for object (string to display in tooltip)
  const QString &tipId() const;
  void setTipId(const QString &s) { tipId_ = s; }

  // calculate tip id (on demand)
  virtual QString calcTipId() const { return calcId(); }

  //---

  // set/get selected
  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  //---

  // set/get inside
  bool isInside() const { return inside_; }
  void setInside(bool b) { inside_ = b; }

 protected:
  using OptString = boost::optional<QString>;

  OptString          id_;                 // id
  CQChartsGeom::BBox rect_;               // bbox
  OptString          tipId_;              // tip id
  bool               selected_ { false }; // is selected
  bool               inside_   { false }; // is mouse inside
};

#endif