#ifndef CQChartsColumnsEdit_H
#define CQChartsColumnsEdit_H

#include <CQChartsColumn.h>
#include <CQChartsLineEditBase.h>

class CQChartsColumnsEdit;
class CQChartsColumnCombo;
class CQChartsModelData;

/*!
 * \brief Columns line edit
 * \ingroup Charts
 */
class CQChartsColumnsLineEdit : public CQChartsLineEditBase {
  Q_OBJECT

  Q_PROPERTY(CQChartsColumns columns     READ columns       WRITE setColumns    )
  Q_PROPERTY(bool            numericOnly READ isNumericOnly WRITE setNumericOnly)

 public:
  CQChartsColumnsLineEdit(QWidget *parent=nullptr, bool isBasic=false);

  void setPlot(CQChartsPlot *plot) override;

  CQChartsModelData *modelData() const;
  void setModelData(CQChartsModelData *modelData);

  const CQChartsColumns &columns() const;
  void setColumns(const CQChartsColumns &c);

  bool isNumericOnly() const { return numericOnly_; }
  void setNumericOnly(bool b);

  void drawPreview(QPainter *painter, const QRect &rect) override;

 signals:
  void columnsChanged();

 private slots:
  void menuEditChanged();

 private:
  void updateColumns(const CQChartsColumns &columns, bool updateText);

  void textChanged() override;

  void updateMenu() override;

  void columnsToWidgets();

  void connectSlots(bool b);

  bool textToColumns(const QString &str, CQChartsColumns &columns) const;

 private:
  CQChartsColumnsEdit *dataEdit_    { nullptr };
  bool                 isBasic_     { false };
  bool                 numericOnly_ { false };
};

//------

#include <CQChartsEditBase.h>

class CQChartsColumnLineEdit;
class QLabel;

/*!
 * \brief Columns edit
 * \ingroup Charts
 */
class CQChartsColumnsEdit : public CQChartsEditBase {
  Q_OBJECT

  Q_PROPERTY(CQChartsColumns columns     READ columns       WRITE setColumns    )
  Q_PROPERTY(bool            numericOnly READ isNumericOnly WRITE setNumericOnly)

 public:
  CQChartsColumnsEdit(QWidget *parent=nullptr, bool isBasic=false);

  CQChartsModelData *modelData() const { return modelData_; }
  void setModelData(CQChartsModelData *modelData);

  const CQChartsColumns &columns() const;
  void setColumns(const CQChartsColumns &c);

  bool isNumericOnly() const { return numericOnly_; }
  void setNumericOnly(bool b);

  QSize sizeHint() const override;

 signals:
  void columnsChanged();

 private slots:
  void addSlot();
  void removeSlot();

  void widgetsToColumn();

 private:
  void columnsToWidgets();

  void updateEdits();

  void connectSlots(bool b);

 private:
  using ColumnCombos = std::vector<CQChartsColumnCombo *>;
  using ColumnEdits  = std::vector<CQChartsColumnLineEdit *>;

  CQChartsColumns    columns_;
  bool               isBasic_      { false };
  bool               numericOnly_  { false };
  CQChartsModelData* modelData_    { nullptr };
  QFrame*            controlFrame_ { nullptr };
  QFrame*            columnsFrame_ { nullptr };
  QLabel*            countLabel_   { nullptr };
  ColumnCombos       columnCombos_;
  ColumnEdits        columnEdits_;
};

//------

#include <CQPropertyViewType.h>

/*!
 * \brief type for CQChartsColumns
 * \ingroup Charts
 */
class CQChartsColumnsPropertyViewType : public CQPropertyViewType {
 public:
  CQChartsColumnsPropertyViewType();

  CQPropertyViewEditorFactory *getEditor() const override;

  bool setEditorData(CQPropertyViewItem *item, const QVariant &value) override;

  void draw(CQPropertyViewItem *item, const CQPropertyViewDelegate *delegate, QPainter *painter,
            const QStyleOptionViewItem &option, const QModelIndex &index,
            const QVariant &value, const ItemState &itemState) override;

  QString tip(const QVariant &value) const override;

  QString userName() const override { return "column_list"; }

 private:
  QString valueString(CQPropertyViewItem *item, const QVariant &value, bool &ok) const;
};

//---

#include <CQPropertyViewEditor.h>

/*!
 * \brief editor factory for CQChartsColumn
 * \ingroup Charts
 */
class CQChartsColumnsPropertyViewEditor : public CQPropertyViewEditorFactory {
 public:
  CQChartsColumnsPropertyViewEditor();

  QWidget *createEdit(QWidget *parent);

  void connect(QWidget *w, QObject *obj, const char *method);

  QVariant getValue(QWidget *w);

  void setValue(QWidget *w, const QVariant &var);
};

#endif
