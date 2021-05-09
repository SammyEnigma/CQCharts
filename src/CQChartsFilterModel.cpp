#include <CQChartsFilterModel.h>
#include <CQChartsExprModel.h>
#include <CQCharts.h>

CQChartsFilterModel::
CQChartsFilterModel(CQCharts *charts, QAbstractItemModel *baseModel, bool exprModel) :
 CQChartsModelFilter(charts), baseModel_(baseModel)
{
  setObjectName("filterModel");

  beginResetModel();

  if (exprModel) {
    exprModel_ = new CQChartsExprModel(charts_, this, baseModel_);

    setSourceModel(exprModel_);
  }
  else
    setSourceModel(baseModel);

  endResetModel();
}

CQChartsFilterModel::
~CQChartsFilterModel()
{
  delete exprModel_;
  delete baseModel_;
}
