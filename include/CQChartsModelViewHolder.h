#ifndef CQChartsModelViewHolder_H
#define CQChartsModelViewHolder_H

#include <QFrame>
#include <QAbstractItemModel>
#include <QSharedPointer>

class CQCharts;

#ifdef CQCHARTS_MODEL_VIEW
class CQChartsModelView;
#else
class CQChartsTable;
class CQChartsTree;
#endif

class QStackedWidget;
class QItemSelectionModel;

/*!
 * \brief Model View Holder Widget (Tree or Table)
 * \ingroup Charts
 */
class CQChartsModelViewHolder : public QFrame {
  Q_OBJECT

 public:
  using ModelP = QSharedPointer<QAbstractItemModel>;

 public:
  CQChartsModelViewHolder(CQCharts *charts=nullptr, QWidget *parent=nullptr);
 ~CQChartsModelViewHolder();

  CQCharts *charts() const { return charts_; }
  void setCharts(CQCharts *charts) { charts_ = charts; init(); }

#ifdef CQCHARTS_MODEL_VIEW
  CQChartsModelView *view() const { return view_; }
#else
  CQChartsTable *table() const { return table_; }
  CQChartsTree  *tree () const { return tree_ ; }
#endif

  void setFilterAnd(bool b);

  void setFilter(const QString &text);
  void addFilter(const QString &text);

  QString filterDetails() const;

  void setSearch(const QString &text);
  void addSearch(const QString &text);

  ModelP model() const;
  void setModel(ModelP model, bool hierarchical);

  QItemSelectionModel *selectionModel();

  void showColumn(int column);
  void hideColumn(int column);

 signals:
  void filterChanged();

 private:
  void init();

 private:
  CQCharts*          charts_       { nullptr };
#ifdef CQCHARTS_MODEL_VIEW
  CQChartsModelView* view_        { nullptr };
#else
  QStackedWidget*    stack_        { nullptr };
  CQChartsTable*     table_        { nullptr };
  CQChartsTree*      tree_         { nullptr };
#endif
  bool               hierarchical_ { false };
};

#endif
