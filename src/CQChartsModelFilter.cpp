#include <CQChartsModelFilter.h>
#include <CQChartsModelExprMatch.h>
#include <CQChartsModelVisitor.h>
#include <CQChartsModelUtil.h>
#include <CQChartsVariant.h>
#include <CQCharts.h>

#include <CQDataModel.h>
#include <QItemSelectionModel>
#include <cassert>

CQChartsModelFilter::
CQChartsModelFilter(CQCharts *charts) :
 charts_(charts)
{
  setObjectName("modelFilter");

  expr_ = new CQChartsModelExprMatch;

  resetFilterData();

  setSortRole(Qt::EditRole);
}

CQChartsModelFilter::
~CQChartsModelFilter()
{
  delete expr_;
}

void
CQChartsModelFilter::
resetFilterData()
{
  filterDatas_.clear();

  pushFilterData();
}

void
CQChartsModelFilter::
pushFilterData()
{
  if (filterDatas_.empty() || ! currentFilterData().isEmpty())
    filterDatas_.emplace_back();
}

void
CQChartsModelFilter::
popFilterData()
{
  if (! filterDatas_.empty())
    filterDatas_.pop_back();

  if (filterDatas_.empty())
    pushFilterData();
}

void
CQChartsModelFilter::
setExpressionFilter(const QString &filter)
{
  auto &filterData = currentFilterData();

  filterData.setType  (CQChartsModelFilterData::Type::EXPRESSION);
  filterData.setFilter(filter);
  filterData.setInvert(false);

  initFilterData(filterData);

  initFilter();
}

void
CQChartsModelFilter::
setRegExpFilter(const QString &filter)
{
  auto &filterData = currentFilterData();

  filterData.setType  (CQChartsModelFilterData::Type::REGEXP);
  filterData.setFilter(filter);
  filterData.setInvert(false);

  initFilterData(filterData);

  initFilter();
}

void
CQChartsModelFilter::
setWildcardFilter(const QString &filter)
{
  auto &filterData = currentFilterData();

  filterData.setType  (CQChartsModelFilterData::Type::WILDCARD);
  filterData.setFilter(filter);
  filterData.setInvert(false);

  initFilterData(filterData);

  initFilter();
}

void
CQChartsModelFilter::
setSimpleFilter(const QString &filter)
{
  auto &filterData = currentFilterData();

  filterData.setType  (CQChartsModelFilterData::Type::SIMPLE);
  filterData.setFilter(filter);
  filterData.setInvert(false);

  initFilterData(filterData);

  initFilter();
}

void
CQChartsModelFilter::
setSelectionFilter(bool invert)
{
  auto &filterData = currentFilterData();

  filterData.setType  (CQChartsModelFilterData::Type::SELECTED);
  filterData.setFilter("");
  filterData.setInvert(invert);

  initFilterData(filterData);

  initFilter();
}

void
CQChartsModelFilter::
setFilterCombine(const Combine &c)
{
  filterCombine_ = c;

  initFilter();
}

QString
CQChartsModelFilter::
filterDetails() const
{
  QStringList strs;

  for (const auto &filterData : filterDatas_)
    strs += filterData.details();

  if (filterCombine_ == Combine::AND)
    return strs.join(" && ");
  else
    return strs.join(" || ");
}

bool
CQChartsModelFilter::
allFiltersEmpty() const
{
  for (const auto &filterData : filterDatas_)
    if (! filterData.isEmpty())
      return false;

  return true;
}

bool
CQChartsModelFilter::
filterAcceptsRow(int row, const QModelIndex &parent) const
{
  if (allFiltersEmpty())
    return true;

  //---

  auto *model = sourceModel();
  assert(model);

  class RowVisitor : public CQChartsModelVisitor {
   public:
    RowVisitor(const CQChartsModelFilter *filter, int column) :
     filter_(filter), column_(column) {
    }

    State visit(const QAbstractItemModel *model, const VisitData &data) override {
      auto ind = model->index(data.row, column_, data.parent);

      if (filter_->acceptsItem(ind)) {
        accepted_ = true;
        return State::TERMINATE;
      }

      return State::OK;
    }

    State hierPostVisit(const QAbstractItemModel *model, const VisitData &data) override {
      if (isAccepted()) {
        auto ind = model->index(data.row, column_, data.parent);

        filter_->addExpand(ind);
      }

      return State::OK;
    }

    bool isAccepted() const { return accepted_; }

   private:
    const CQChartsModelFilter* filter_   { nullptr };
    int                        column_   { 0 };
    bool                       accepted_ { false };
  };

  // visit single row
  int column = filterKeyColumn();

  if (column < 0)
    column = 0;

  RowVisitor visitor(this, column);

  (void) CQModelVisit::exec(model, parent, row, visitor);

  // TODO: cache result for hier traversal
  return visitor.isAccepted();
}

bool
CQChartsModelFilter::
acceptsItem(const QModelIndex &ind) const
{
  auto p = matches_.find(ind);

  if (p != matches_.end())
    return (*p).second;

  bool match = itemMatch(ind);

  matches_[ind] = match;

  return match;
}

bool
CQChartsModelFilter::
itemMatch(const QModelIndex &ind) const
{
  if (filterCombine_ == Combine::AND) {
    for (const auto &filterData : filterDatas_) {
      if (! filterItemMatch(filterData, ind))
        return false;
    }

    return true;
  }
  else {
    for (const auto &filterData : filterDatas_) {
      if (filterItemMatch(filterData, ind))
        return true;
    }

    return false;
  }
}

bool
CQChartsModelFilter::
filterItemMatch(const CQChartsModelFilterData &filterData, const QModelIndex &ind) const
{
  // filter in/out selected items
  if      (filterData.isSelected()) {
    assert(selectionModel_);

    if (filterData.filterRows().empty())
      return ! filterData.isInvert();

    auto *model = sourceModel();
    assert(model);

    auto ind1 = model->index(ind.row(), 0, ind.parent());

    bool rc = filterData.filterRows().contains(ind1);

    return (filterData.isInvert() ? ! rc  : rc);
  }
  // filter string matches regexp
  else if (filterData.isRegExp() || filterData.isWildcard()) {
    auto *model = sourceModel();
    assert(model);

    auto var = model->data(ind);

    bool ok;

    auto str = CQChartsVariant::toString(var, ok);

    return filterData.regexp().match(str);
  }
  // filter string matches one of list of strings
  else if (filterData.isSimple()) {
    auto *model = sourceModel();
    assert(model);

    for (const auto &columnFilter : filterData.columnFilterMap()) {
      auto ind1 = model->index(ind.row(), columnFilter.first, ind.parent());

      auto var1 = model->data(ind1);

      bool ok1;

      auto str1 = CQChartsVariant::toString(var1, ok1);

      if (! columnFilter.second.match(str1))
        return false;
    }

    return true;
  }
  // filter by expression
  else if (filterData.isExpr()) {
    bool ok;

    return exprMatch()->match(filterData.filterExpr(), ind, ok);
  }
  else {
    assert(false);

    return true;
  }
}

#if 0
void
CQChartsModelFilter::
expandMatches()
{
  std::vector<QModelIndex> inds;

  for (const auto &ind : expand_) {
    auto ind1 = filterModel->mapFromSource(ind);

    inds.push_back(ind1);
  }

  for (int i = inds.size() - 1; i >= 0; --i) {
    setExpanded(inds[i], true);
  }
}
#endif

void
CQChartsModelFilter::
initFilterData(CQChartsModelFilterData &filterData)
{
  if      (filterData.isSelected()) {
    assert(selectionModel_);

    if (selectionModel_->hasSelection()) {
      auto selectedRows = selectionModel_->selectedRows();

      QModelIndexList selectedRows1;

      for (int i = 0; i < selectedRows.size(); ++i) {
        auto ind1 = mapToSource(selectedRows[i]);

        selectedRows1.push_back(ind1);
      }

      filterData.setFilterRows(selectedRows1);
    }
    else {
      filterData.setFilterRows(QModelIndexList());
    }
  }
  else if (filterData.isRegExp() || filterData.isWildcard()) {
    auto *model = this->sourceModel();
    assert(model);

    QString filter;
    int     column = -1;

    if (CQChartsModelUtil::decodeModelFilterStr(model, filterData.filter(), filter, column))
      setFilterKeyColumn(column);

    if (filterData.isRegExp())
      filterData.setRegExp(CQChartsRegExp(filter, QRegExp::PatternSyntax::RegExp));
    else
      filterData.setRegExp(CQChartsRegExp(filter, QRegExp::PatternSyntax::WildcardUnix));
  }
  else if (filterData.isSimple()) {
    auto *model = this->sourceModel();
    assert(model);

    auto strs = filterData.filter().split(",");

    CQChartsModelFilterData::ColumnFilterMap columnFilterMap;

    for (int i = 0; i < strs.size(); ++i) {
      QString filter;
      int     column = -1;

      if (! CQChartsModelUtil::decodeModelFilterStr(model, strs[i], filter, column))
        continue;

      columnFilterMap[column] = CQChartsRegExp(filter);
    }

    filterData.setColumnFilterMap(columnFilterMap);
  }
  else if (filterData.isExpr()) {
    auto *model = this->sourceModel();
    assert(model);

    auto expr1 = replaceNamedColumns(model, filterData.filter());

    filterData.setFilterExpr(expr1);
  }
}

void
CQChartsModelFilter::
initFilter()
{
  matches_.clear();
  expand_ .clear();

  //---

  if (selectionModel_)
    selectionModel_->clear();

  auto *modelData = charts_->getModelData(this);

  if (modelData) {
    exprMatch()->setModelData(modelData);
  }
  else {
    auto *model = this->sourceModel();
    assert(model);

    exprMatch()->setModel(model);
  }

  exprMatch()->initColumns();

  //---

  invalidateFilter();
  //invalidate();

  //expandMatches();
}

bool
CQChartsModelFilter::
lessThan(const QModelIndex &lhs, const QModelIndex &rhs) const
{
  auto ldata = sourceModel()->data(lhs, Qt::EditRole);

  if (! ldata.isValid())
    ldata = sourceModel()->data(lhs, Qt::DisplayRole);

  auto rdata = sourceModel()->data(rhs, Qt::EditRole);

  if (! rdata.isValid())
    rdata = sourceModel()->data(rhs, Qt::DisplayRole);

  // invalid is less than everything
  if      (! ldata.isValid())
    return rdata.isValid();
  else if (! rdata.isValid())
    return ldata.isValid();

  return (CQChartsVariant::cmp(ldata, rdata) < 0);
//return QSortFilterProxyModel::lessThan(lhs, rhs);
}

QVariant
CQChartsModelFilter::
headerData(int section, Qt::Orientation orientation, int role) const
{
  auto var = QSortFilterProxyModel::headerData(section, orientation, role);

  if (! var.isValid())
    return var;

  if (role == Qt::EditRole) {
    if (orientation == Qt::Horizontal) {
      bool converted;

      var = CQChartsModelUtil::columnHeaderUserData(charts_, this, section, var, converted);
    }
  }

  return var;
}

QVariant
CQChartsModelFilter::
data(const QModelIndex &ind, int role) const
{
  if (! ind.isValid())
    return QVariant();

  QVariant var;

  //---

  // get cached display or edit value
  if (isMapping() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
    if (role == Qt::DisplayRole)
      var = QSortFilterProxyModel::data(ind, int(CQBaseModelRole::OutputValue));
    else
      var = QSortFilterProxyModel::data(ind, int(CQBaseModelRole::IntermediateValue));

    if (var.isValid())
      return var;
  }

  //---

  // get model value
  var = QSortFilterProxyModel::data(ind, role);

  if (role == Qt::EditRole && ! var.isValid())
    var = QSortFilterProxyModel::data(ind, Qt::DisplayRole);

  if (! var.isValid())
    return QVariant();

  //---

  // convert value to display or edit value using column type data
  if (isMapping() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
    assert(ind.model() == this);

    auto ind1 = mapToSource(ind);

    assert(ind.column() == ind1.column());

    if (! var.isValid())
      return var;

    //---

    auto storeColumnData = [&](const QVariant &var, int role) {
      std::unique_lock<std::mutex> lock(mutex_);

      auto *th        = const_cast<CQChartsModelFilter *>(this);
      auto *dataModel = dynamic_cast<CQDataModel *>(th->baseModel());

      if (dataModel) {
        if (dataModel->isReadOnly()) {
          dataModel->setReadOnly(false);

          th->setData(ind, var, role);

          dataModel->setReadOnly(true);
        }
        else
          th->setData(ind, var, role);
      }
    };

    // convert variant using column type data
    bool converted;

    auto var1 = var;

    if (role == Qt::DisplayRole) {
      var1 = CQChartsModelUtil::columnDisplayData(charts_, this,
               CQChartsColumn(ind1.column()), var, converted);

      if (converted)
        storeColumnData(var1, int(CQBaseModelRole::OutputValue));
    }
    else {
      var1 = CQChartsModelUtil::columnUserData(charts_, this,
               CQChartsColumn(ind1.column()), var, converted);

      if (converted)
        storeColumnData(var1, int(CQBaseModelRole::IntermediateValue));
    }

    return var1;
  }

  return var;
}

QString
CQChartsModelFilter::
replaceNamedColumns(QAbstractItemModel *model, const QString &expr) const
{
  QModelIndex ind;

  return CQChartsModelUtil::replaceModelExprVars(expr, model, ind, -1, -1);
}

void
CQChartsModelFilter::
sort(int column, Qt::SortOrder order)
{
  int nc = columnCount();

  for (int c = 0; c < nc; ++c)
    setHeaderData(c, Qt::Horizontal, (c == column), int(CQBaseModelRole::Sorted));

  setHeaderData(column, Qt::Horizontal, order, int(CQBaseModelRole::SortOrder));

  QSortFilterProxyModel::sort(column, order);
}
