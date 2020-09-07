#ifndef CQChartsModelDetails_H
#define CQChartsModelDetails_H

#include <CQChartsColumn.h>
#include <CQChartsColumnType.h>
#include <CQChartsModelTypes.h>
#include <CQChartsUtil.h>
#include <future>

class CQChartsModelColumnDetails;
class CQChartsModelData;
class CQCharts;
class CQChartsValueSet;

class QAbstractItemModel;

/*!
 * \brief Model Details
 * \ingroup Charts
 */
class CQChartsModelDetails : public QObject {
  Q_OBJECT

  Q_PROPERTY(int numColumns   READ numColumns    )
  Q_PROPERTY(int numRows      READ numRows       )
  Q_PROPERTY(int hierarchical READ isHierarchical)

 public:
  using ColumnDetails = CQChartsModelColumnDetails;

 public:
  CQChartsModelDetails(CQChartsModelData *data);

 ~CQChartsModelDetails();

  CQChartsModelData *data() const { return data_; }

  int numColumns() const;
  int numRows   () const;

  bool isHierarchical() const;

  ColumnDetails *columnDetails(const CQChartsColumn &column);
  const ColumnDetails *columnDetails(const CQChartsColumn &column) const;

  CQChartsColumns numericColumns() const;

  CQChartsColumns monotonicColumns() const;

  void reset();

  std::vector<int> duplicates() const;
  std::vector<int> duplicates(const CQChartsColumn &column) const;

  double correlation(const CQChartsColumn &column1, const CQChartsColumn &column2) const;

  CQCharts *charts() const;

  QAbstractItemModel *model() const;

 signals:
  void detailsReset();

 private slots:
  void modelTypeChangedSlot(int modelInd);

 private:
  void resetValues();

  std::vector<int> columnDuplicates(const CQChartsColumn &column, bool all) const;

  void updateSimple();
  void updateFull();

  void initSimpleData() const;
  void initFullData() const;

 private:
  enum class Initialized {
    NONE,
    SIMPLE,
    FULL
  };

  CQChartsModelDetails(const CQChartsModelDetails &) = delete;
  CQChartsModelDetails &operator=(const CQChartsModelDetails &) = delete;

 private:
  using ColumnDetailsMap = std::map<CQChartsColumn, ColumnDetails *>;

  CQChartsModelData* data_ { nullptr }; //!< model data

  // cached data
  Initialized      initialized_  { Initialized::NONE }; //!< is initialized
  int              numColumns_   { 0 };                 //!< model number of columns
  int              numRows_      { 0 };                 //!< model number of rows
  bool             hierarchical_ { false };             //!< model is hierarchical
  ColumnDetailsMap columnDetails_;                      //!< model column details

  // mutex
  mutable std::mutex mutex_; //!< mutex
};

//---

/*!
 * \brief Model Column Details
 * \ingroup Charts
 */
class CQChartsModelColumnDetails {
 public:
  using Details       = CQChartsModelDetails;
  using ColumnType    = CQBaseModelType;
  using TableDrawType = CQChartsColumnType::DrawType;
  using ValueCount    = std::pair<QVariant, int>;
  using ValueCounts   = std::vector<ValueCount>;

 public:
  CQChartsModelColumnDetails(Details *details, const CQChartsColumn &column);

  virtual ~CQChartsModelColumnDetails();

  Details *details() const { return details_; }

  const CQChartsColumn &column() const { return column_; }

  QString headerName() const;

  bool isKey() const;

  QString typeName() const;

  const CQChartsModelTypeData &typeData() const;

  ColumnType type() const;
  void setType(ColumnType type);

  ColumnType baseType() const;
  void setBaseType(ColumnType type);

  const CQChartsNameValues &nameValues() const;

  static const QStringList &getLongNamedValues();
  static const QStringList &getShortNamedValues();

  static bool isNamedValue(const QString &name);

  QVariant getNamedValue(const QString &name) const;

  QVariant minValue() const;
  QVariant maxValue() const;

  QVariant meanValue  (bool useNaN=true) const;
  QVariant stdDevValue(bool useNaN=true) const;

  QVariant dataName(const QVariant &v) const;

  int numRows() const;

  bool isNumeric() const;

  bool isMonotonic () const;
  bool isIncreasing() const;

  int numUnique() const;

  QVariantList uniqueValues() const;
  QVariantList uniqueCounts() const;

  ValueCounts uniqueValueCounts() const;

  int uniqueId(const QVariant &v) const;

  QVariant uniqueValue(int i) const;

  int numNull() const;

  int numValues() const { return valueInds_.size(); }

  int valueInd(const QVariant &value) const;

  QVariant medianValue     (bool useNaN=true) const;
  QVariant lowerMedianValue(bool useNaN=true) const;
  QVariant upperMedianValue(bool useNaN=true) const;

  QVariantList outlierValues() const;

  bool isOutlier(const QVariant &value) const;

  double map(const QVariant &var) const;

  //---

  CQChartsValueSet *valueSet() const { return valueSet_; }

  //---

  int preferredWidth() const { return preferredWidth_; }
  void setPreferredWidth(int w) { preferredWidth_ = w; }

  const CQChartsColor &tableDrawColor() const { return tableDrawColor_; }
  void setTableDrawColor(const CQChartsColor &c) { tableDrawColor_ = c; }

  const TableDrawType &tableDrawType() const { return tableDrawType_; }
  void setTableDrawType(const TableDrawType &t) { tableDrawType_ = t; }

  const CQChartsColorStops &tableDrawStops() const { return tableDrawStops_; }
  void setTableDrawType(const CQChartsColorStops &s) { tableDrawStops_ = s; }

  bool columnNameValue(const QString &name, QString &value) const;

  virtual bool checkRow(const QVariant &) { return true; }

  void resetTypeInitialized();

  void initCache() const;

  const CQChartsColumnType *columnType() const;

 private:
  bool initData();

  void initType() const;
  bool calcType();

  void addInt   (long i, bool ok);
  void addReal  (double r, bool ok);
  void addString(const QString &s);
  void addTime  (double t, bool ok);
  void addColor (const CQChartsColor &c, bool ok);

  void addValue(const QVariant &value);

  bool columnColor(const QVariant &var, CQChartsColor &color) const;

 private:
  CQChartsModelColumnDetails(const CQChartsModelColumnDetails &) = delete;
  CQChartsModelColumnDetails &operator=(const CQChartsModelColumnDetails &) = delete;

 private:
  using VariantInds = std::map<QVariant, int>;

  Details*       details_ { nullptr };
  CQChartsColumn column_;

  // cached type data
  bool                  typeInitialized_ { false }; //!< is type data set
  CQChartsModelTypeData typeData_;                  //!< column data type
  QString               typeName_;                  //!< type name

  // cached data
  bool                  initialized_     { false };   //!< is data set
  QVariant              minValue_;                    //!< min value (as variant)
  QVariant              maxValue_;                    //!< max value (as variant)
  int                   numRows_         { 0 };       //!< number of rows
  bool                  monotonic_       { true };    //!< values are monotonic
  bool                  increasing_      { true };    //!< values are increasing
  CQChartsValueSet*     valueSet_        { nullptr }; //!< values
  VariantInds           valueInds_;                   //!< unique values

  // table render data
  int                   preferredWidth_ { -1 };
  CQChartsColor         tableDrawColor_;
  TableDrawType         tableDrawType_   { TableDrawType::HEATMAP };
  CQChartsColorStops    tableDrawStops_;

  // mutex
  mutable std::mutex    mutex_; //!< mutex
};

#endif
