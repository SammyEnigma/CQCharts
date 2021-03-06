#include <CQChartsModelViewHolder.h>

#ifdef CQCHARTS_MODEL_VIEW
#include <CQChartsModelView.h>
#else
#include <CQChartsTable.h>
#include <CQChartsTree.h>
#endif

#include <CQUtil.h>

#include <QStackedWidget>
#include <QVBoxLayout>

CQChartsModelViewHolder::
CQChartsModelViewHolder(CQCharts *charts, QWidget *parent) :
 QFrame(parent), charts_(charts)
{
  setObjectName("modelViewHolder");

  init();
}

CQChartsModelViewHolder::
~CQChartsModelViewHolder()
{
#ifdef CQCHARTS_MODEL_VIEW
  delete view_;
#else
  delete table_;
  delete tree_;
#endif
}

void
CQChartsModelViewHolder::
init()
{
  assert(charts_);

  auto *layout = CQUtil::makeLayout<QVBoxLayout>(this, 2, 2);

#ifdef CQCHARTS_MODEL_VIEW
  assert(! view_);

  view_ = new CQChartsModelView(charts(), this);

  layout->addWidget(view_);
#else
  assert(! stack_);

  stack_ = CQUtil::makeWidget<QStackedWidget>("stack");

  table_ = new CQChartsTable(charts(), this);

  connect(table_, SIGNAL(filterChanged()), this, SIGNAL(filterChanged()));

  tree_ = new CQChartsTree(charts(), this);

  stack_->addWidget(table_);
  stack_->addWidget(tree_ );

  layout->addWidget(stack_);
#endif
}

void
CQChartsModelViewHolder::
setFilterAnd(bool b)
{
#ifdef CQCHARTS_MODEL_VIEW
  view_->setFilterAnd(b);
#else
  table_->setFilterAnd(b);
#endif
}

void
CQChartsModelViewHolder::
setFilter(const QString &text)
{
#ifdef CQCHARTS_MODEL_VIEW
  view_->setFilter(text);
#else
  table_->setFilter(text);
#endif
}

void
CQChartsModelViewHolder::
addFilter(const QString &text)
{
#ifdef CQCHARTS_MODEL_VIEW
  view_->addFilter(text);
#else
  table_->addFilter(text);
#endif
}

QString
CQChartsModelViewHolder::
filterDetails() const
{
#ifdef CQCHARTS_MODEL_VIEW
  return view_->filterDetails();
#else
  return table_->filterDetails();
#endif
}

void
CQChartsModelViewHolder::
setSearch(const QString &text)
{
#ifdef CQCHARTS_MODEL_VIEW
  view_->setSearch(text);
#else
  table_->setSearch(text);
#endif
}

void
CQChartsModelViewHolder::
addSearch(const QString &text)
{
#ifdef CQCHARTS_MODEL_VIEW
  view_->addSearch(text);
#else
  table_->addSearch(text);
#endif
}

CQChartsModelViewHolder::ModelP
CQChartsModelViewHolder::
model() const
{
#ifdef CQCHARTS_MODEL_VIEW
  return view_->modelP();
#else
  if (! hierarchical_)
    return table_->modelP();
  else
    return tree_->modelP();
#endif
}

void
CQChartsModelViewHolder::
setModel(ModelP model, bool hierarchical)
{
  hierarchical_ = hierarchical;

#ifdef CQCHARTS_MODEL_VIEW
  view_->setModelP(model);
#else
  if (! hierarchical_) {
    table_->setModelP(model);
    tree_ ->setModelP(CQChartsTree::ModelP());

    stack_->setCurrentIndex(0);
  }
  else {
    tree_ ->setModelP(model);
    table_->setModelP(CQChartsTable::ModelP());

    stack_->setCurrentIndex(1);
  }
#endif
}

QItemSelectionModel *
CQChartsModelViewHolder::
selectionModel()
{
#ifdef CQCHARTS_MODEL_VIEW
  return view_->selectionModel();
#else
  if (! hierarchical_)
    return table_->selectionModel();
  else
    return tree_->selectionModel();
#endif
}

void
CQChartsModelViewHolder::
showColumn(int column)
{
#ifdef CQCHARTS_MODEL_VIEW
  return view_->showColumn(column);
#else
  if (! hierarchical_)
    return table_->showColumn(column);
  else
    return tree_->showColumn(column);
#endif
}

void
CQChartsModelViewHolder::
hideColumn(int column)
{
#ifdef CQCHARTS_MODEL_VIEW
  return view_->hideColumn(column);
#else
  if (! hierarchical_)
    return table_->hideColumn(column);
  else
    return tree_->hideColumn(column);
#endif
}
