#ifndef CQChartsAxisSideEdit_H
#define CQChartsAxisSideEdit_H

#include <CQChartsAxisSide.h>
#include <CQChartsEnumEdit.h>

class QComboBox;

/*!
 * \brief editor for CQChartsAxisSide
 * \ingroup Charts
 */
class CQChartsAxisSideEdit : public CQChartsEnumEdit {
  Q_OBJECT

  Q_PROPERTY(CQChartsAxisSide axisSide READ axisSide WRITE setAxisSide)

 public:
  using AxisSide = CQChartsAxisSide;

 public:
  CQChartsAxisSideEdit(QWidget *parent=nullptr);

  const AxisSide &axisSide() const { return axisSide_; }
  void setAxisSide(const AxisSide &pos);

  QStringList enumNames() const override { return axisSide_.enumNames(); }

  void setEnumFromString(const QString &str) override;

  QVariant getVariantFromEnum() const override;

  void setEnumFromVariant(const QVariant &var) override;

  QString variantToString(const QVariant &var) const override;

  void connect(QObject *obj, const char *method) override;

 Q_SIGNALS:
  void axisSideChanged();

 private:
  AxisSide   axisSide_;
  QComboBox* combo_ { nullptr };
};

//------

/*!
 * \brief type for CQChartsAxisSide
 * \ingroup Charts
 */
class CQChartsAxisSidePropertyViewType : public CQChartsEnumPropertyViewType {
 public:
  CQPropertyViewEditorFactory *getEditor() const override;

  QString variantToString(const QVariant &var) const override;
};

//---

/*!
 * \brief editor factory for CQChartsAxisSide
 * \ingroup Charts
 */
class CQChartsAxisSidePropertyViewEditorFactory :
  public CQChartsEnumPropertyViewEditorFactory {
 public:
  QWidget *createEdit(QWidget *parent) override;
};

#endif
