#ifndef CQChartsBarChartPlot_H
#define CQChartsBarChartPlot_H

#include <CQChartsBarPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsColor.h>

//---

/*!
 * \brief Bar Chart plot type
 * \ingroup Charts
 */
class CQChartsBarChartPlotType : public CQChartsGroupPlotType {
 public:
  CQChartsBarChartPlotType();

  QString name() const override { return "barchart"; }
  QString desc() const override { return "BarChart"; }

  Dimension dimension() const override { return Dimension::ONE_D; }

  void addParameters() override;

  QString yColumnName() const override { return "value"; }

  bool allowXAxisIntegral() const override { return false; }
  bool allowYAxisIntegral() const override { return false; }

  bool allowXLog() const override { return false; }

  bool canProbe() const override { return true; }

  QString description() const override;

  Plot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsBarChartPlot;

/*!
 * brief values for bar
 *
 * values for bar (normal: 1 value : range bar: multiple values)
 * \todo handle range bar in distribution plot
 */
class CQChartsBarChartValue {
 public:
  using NameValues = std::map<QString, QString>;

 public:
  //! value index
  struct ValueInd {
    double      value { 0.0 };
    QModelIndex ind;
    int         vrow  { -1 };
  };

  using ValueInds = std::vector<ValueInd>;

 public:
  CQChartsBarChartValue() { }

  void addValueInd(const ValueInd &valueInd) {
    valueInds_.push_back(valueInd);
  }

  const ValueInds &valueInds() const { return valueInds_; }

  const QString &valueName() const { return valueName_; }
  void setValueName(const QString &s) { valueName_ = s; }

  const QString &groupName() const { return groupName_; }
  void setGroupName(const QString &s) { groupName_ = s; }

  const NameValues &nameValues() const { return nameValues_; }
  void setNameValues(const NameValues &v) { nameValues_ = v; }

  void setNameValue(const QString &name, const QString &value) {
    nameValues_[name] = value;
  }

  QString getNameValue(const QString &name) const {
    auto p = nameValues_.find(name);
    if (p == nameValues_.end()) return "";

    return (*p).second;
  }

  void calcRange(ValueInd &minInd, ValueInd &maxInd, double &mean, double &sum) const {
    assert(! valueInds_.empty());

    int n = valueInds_.size();

    minInd = valueInds_[0];
    maxInd = valueInds_[0];

    sum = valueInds_[0].value;

    for (int i = 1; i < n; ++i) {
      double value = valueInds_[i].value;

      if (value < minInd.value)
        minInd = valueInds_[i];

      if (value > maxInd.value)
        maxInd = valueInds_[i];

      sum += value;
    }

    mean = sum/n;
  }

 private:
  ValueInds  valueInds_;  //!< value indices
  QString    valueName_;  //!< value name
  QString    groupName_;  //!< group name
  NameValues nameValues_; //!< name values
};

//------

/*!
 * \brief set of value bars for group
 * \ingroup Charts
 */
class CQChartsBarChartValueSet {
 public:
  using Values = std::vector<CQChartsBarChartValue>;

 public:
  CQChartsBarChartValueSet() = default;

  CQChartsBarChartValueSet(const QString &name, int ind) :
   name_(name), ind_(ind) {
  }

  const QString &name() const { return name_; }

  int ind() const { return ind_; }

  int groupInd() const { return groupInd_; }
  void setGroupInd(int i) { groupInd_ = i; }

  int numValues() const { return values_.size(); }

  const Values &values() const { return values_; }

  void addValue(const CQChartsBarChartValue &value) {
    values_.push_back(value);
  }

  const CQChartsBarChartValue &value(int i) const {
    assert(i >= 0 && i < int(values_.size()));

    return values_[i];
  }

  bool calcSums(double &posSum, double &negSum) const {
    if (values_.empty()) return false;

    posSum = 0.0;
    negSum = 0.0;

    for (auto &v : values_) {
      for (auto &vi : v.valueInds()) {
        double value = vi.value;

        if (value >= 0) posSum += value;
        else            negSum += value;
      }
    }

    return true;
  }

  bool calcStats(double &min, double &max, double &mean, double &sum) const {
    if (values_.empty()) return false;

    min  = 0.0;
    max  = 0.0;
    mean = 0.0;
    sum  = 0.0;

    int n = 0;

    for (auto &v : values_) {
      for (auto &vi : v.valueInds()) {
        double value = vi.value;

        if (n == 0) {
          min = value;
          max = value;
        }
        else {
          min = std::min(min, value);
          max = std::max(max, value);
        }

        sum += value;

        ++n;
      }
    }

    mean = (n > 0 ? sum/n : 0.0);

    return true;
  }

 private:
  QString name_;            //!< group name
  int     ind_      { 0 };  //!< index
  int     groupInd_ { -1 }; //!< group ind
  Values  values_;          //!< value bars
};

//------

/*!
 * \brief Bar Chart Bar object
 * \ingroup Charts
 */
class CQChartsBarChartObj : public CQChartsPlotObj {
  Q_OBJECT

  Q_PROPERTY(QString       group READ groupStr)
  Q_PROPERTY(QString       name  READ nameStr )
  Q_PROPERTY(QString       value READ valueStr)
  Q_PROPERTY(CQChartsColor color READ color    WRITE setColor)

 public:
  using Plot = CQChartsBarChartPlot;

 public:
  CQChartsBarChartObj(const Plot *plot, const BBox &rect, const ColorInd &iset,
                      const ColorInd &ival, const ColorInd &isval, const QModelIndex &ind);

  QString typeName() const override { return "bar"; }

  //---

  QString calcId() const override;

  QString calcTipId() const override;

  //---

  QString groupStr() const;
  QString nameStr () const;
  QString valueStr() const;

  //---

  bool isValueSet() const { return valueSet_; }
  void setValueSet(bool b) { valueSet_ = b; }

  //---

  const CQChartsColor &color() const { return color_; }
  void setColor(const CQChartsColor &color) { color_ = color; }

  BBox dataLabelRect() const;

  //---

  void addProperties(CQPropertyViewModel *model, const QString &path) override;

  //---

  void getObjSelectIndices(Indices &inds) const override;

  //---

  void draw(PaintDevice *device) const override;

  void drawFg(PaintDevice *device) const override;

  //---

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const;

  QColor calcBarColor() const;

  //---

  void writeScriptData(CQChartsScriptPaintDevice *device) const override;

  //---

  const CQChartsBarChartValue *value() const;

  const CQChartsBarChartValueSet *valueSet() const;

 private:
  const Plot*   plot_     { nullptr }; //!< parent plot
  CQChartsColor color_;                //!< custom color
  bool          valueSet_ { false };   //!< is value set
};

//---

#include <CQChartsKey.h>

/*!
 * \brief Bar Chart Plot Key Color Box
 * \ingroup Charts
 */
class CQChartsBarKeyColor : public CQChartsKeyColorBox {
  Q_OBJECT

  Q_PROPERTY(CQChartsColor color READ color WRITE setColor)

 public:
  using Plot = CQChartsBarChartPlot;

 public:
  CQChartsBarKeyColor(Plot *plot, const QString &name, const ColorInd &is,
                      const ColorInd &ig, const ColorInd &iv);

  bool selectPress(const Point &p, CQChartsSelMod selMod) override;

  QBrush fillBrush() const override;
  QPen   strokePen() const override;

  bool tipText(const Point &p, QString &tip) const override;

  // get/set hidden
  bool isSetHidden() const;
  void setSetHidden(bool b);

 private:
  Plot*   plot_ { nullptr }; //!< plot
  QString name_;             //!< item name
};

/*!
 * \brief Bar Chart Key Text
 * \ingroup Charts
 */
class CQChartsBarKeyText : public CQChartsKeyText {
  Q_OBJECT

 public:
  using Plot = CQChartsBarChartPlot;

 public:
  CQChartsBarKeyText(Plot *plot, const QString &text, const ColorInd &ic);

  QColor interpTextColor(const ColorInd &ind) const override;

  bool isSetHidden() const;
};

//---

/*!
 * \brief bar chart plot
 * \ingroup Charts
 *
 * columns:
 *   + x     : name
 *   + y     : value(s)
 *   + group : group(s)
 *   + bar   : custom color, stacked, percent, range, horizontal, margin, stroke, fill
 *
 * Plot Type
 *   + \ref CQChartsBarChartPlotType
 *
 * Example
 *   + \image html barchart.png
 */
class CQChartsBarChartPlot : public CQChartsBarPlot,
 public CQChartsObjDotPointData<CQChartsBarChartPlot> {
  Q_OBJECT

  // style
  Q_PROPERTY(CQChartsColumn nameColumn  READ nameColumn  WRITE setNameColumn )
  Q_PROPERTY(CQChartsColumn labelColumn READ labelColumn WRITE setLabelColumn)

  // options
  Q_PROPERTY(PlotType  plotType  READ plotType  WRITE setPlotType )
  Q_PROPERTY(ValueType valueType READ valueType WRITE setValueType)

  Q_PROPERTY(bool percent    READ isPercent    WRITE setPercent   )
  Q_PROPERTY(bool skipEmpty  READ isSkipEmpty  WRITE setSkipEmpty )
  Q_PROPERTY(bool colorBySet READ isColorBySet WRITE setColorBySet)

  // dot line
  Q_PROPERTY(bool           dotLines     READ isDotLines   WRITE setDotLines    )
  Q_PROPERTY(CQChartsLength dotLineWidth READ dotLineWidth WRITE setDotLineWidth)

  CQCHARTS_NAMED_POINT_DATA_PROPERTIES(Dot, dot)

  Q_ENUMS(PlotType)
  Q_ENUMS(ValueType)

 public:
  enum class PlotType {
    NORMAL,
    STACKED,
  };

  enum class ValueType {
    VALUE,
    RANGE,
    MIN,
    MAX,
    MEAN,
    SUM
  };

 public:
  CQChartsBarChartPlot(View *view, const ModelP &model);
 ~CQChartsBarChartPlot();

  //---

  void init() override;
  void term() override;

  //---

  const Column &nameColumn() const { return nameColumn_; }
  void setNameColumn(const Column &c);

  const Column &labelColumn() const { return labelColumn_; }
  void setLabelColumn(const Column &c);

  //---

  PlotType plotType() const { return plotType_; }

  bool isNormal () const { return (plotType() == PlotType::NORMAL ); }
  bool isStacked() const { return (plotType() == PlotType::STACKED); }

  ValueType valueType() const { return valueType_; }

  bool isValueValue() const { return (valueType_ == ValueType::VALUE); }
  bool isValueRange() const { return (valueType_ == ValueType::RANGE); }
  bool isValueMin  () const { return (valueType_ == ValueType::MIN  ); }
  bool isValueMax  () const { return (valueType_ == ValueType::MAX  ); }
  bool isValueMean () const { return (valueType_ == ValueType::MEAN ); }
  bool isValueSum  () const { return (valueType_ == ValueType::SUM  ); }

  bool isPercent  () const { return percent_  ; }
  bool isSkipEmpty() const { return skipEmpty_; }

  //---

  // when multiple columns and grouped then color by value in value set (group)
  bool isColorBySet() const { return colorBySet_; }
  void setColorBySet(bool b);

  //---

  bool isDotLines() const { return dotLineData_.enabled; }

  const Length &dotLineWidth() const { return dotLineData_.width; }
  void setDotLineWidth(const Length &l);

  //---

  BBox calcAnnotationBBox() const override;

  //---

  void addProperties() override;

  //---

  Range calcRange() const override;

  void postCalcRange() override;

  bool createObjs(PlotObjs &objs) const override;

  //---

  QString valueName() const;
  QString valueStr(double v) const;

  Axis *mappedXAxis() const override;
  Axis *mappedYAxis() const override;

  void addKeyItems(PlotKey *key) override;

   //---

  bool isSetHidden  (int i) const;
  bool isValueHidden(int i) const;

  //---

  bool addMenuItems(QMenu *menu) override;

  //---

  double getPanX(bool is_shift) const override;
  double getPanY(bool is_shift) const override;

 public slots:
  // set plot type
  void setPlotType(PlotType plotType);

  // set normal or stacked
  void setNormal (bool b);
  void setStacked(bool b);

  // set value type
  void setValueType(ValueType valueType);

  void setValueValue(bool b);
  void setValueRange(bool b);
  void setValueMin  (bool b);
  void setValueMax  (bool b);
  void setValueMean (bool b);
  void setValueSum  (bool b);

  // set percent
  void setPercent(bool b);

  // set skip empty
  void setSkipEmpty(bool b);

  // set dot lines
  void setDotLines(bool b);

 protected:
  void addRow(const ModelVisitor::VisitData &data, Range &dataRange) const;

  void addRowColumn(const ModelVisitor::VisitData &data, const Columns &valueColumns,
                    Range &dataRange) const;

  void initRangeAxes() const;
  void initRangeAxesI();

  void initObjAxes() const;
  void initObjAxesI();

 protected:
  using ValueSet      = CQChartsBarChartValueSet;
  using ValueSets     = std::vector<ValueSet>;
  using ValueNames    = std::vector<QString>;
  using ValueGroupInd = std::map<int, int>;

  struct ValueData {
    ValueSets     valueSets;     //!< value sets
    ValueGroupInd valueGroupInd; //!< group ind to value index map

    void clear() {
      valueSets    .clear();
      valueGroupInd.clear();
    }
  };

 public:
  int numValueSets() const { return valueData_.valueSets.size(); }

  const ValueSet &valueSet(int i) const {
    assert(i >= 0 && i < int(valueData_.valueSets.size()));
    return valueData_.valueSets[i];
  }

  int numSetValues() const {
    return (! valueData_.valueSets.empty() ? valueData_.valueSets[0].numValues() : 0);
  }

 protected:
  void initGroupValueSet() const;

  const ValueSet *groupValueSet(int groupId) const;

  ValueSet *groupValueSetI(int groupId);

  using BarObj = CQChartsBarChartObj;

  virtual BarObj *createBarObj(const BBox &rect, const ColorInd &is, const ColorInd &ig,
                               const ColorInd &iv, const QModelIndex &ind) const;

 protected:
  struct DotLineData {
    bool   enabled { false }; //!< shown
    Length width   { "3px" }; //!< width
  };

 private:
  // columns
  Column nameColumn_;  //!< name column
  Column labelColumn_; //!< data label column

  // options
  PlotType  plotType_   { PlotType::NORMAL }; //!< plot type
  ValueType valueType_  { ValueType::VALUE }; //!< bar value type
  bool      percent_    { false };            //!< percent values
  bool      skipEmpty_  { false };            //!< skip empty groups
  bool      colorBySet_ { false };            //!< color bars by set or value

  DotLineData    dotLineData_;          //!< dot line data
  mutable double barWidth_     { 1.0 }; //!< minimum bar width
  ValueData      valueData_;            //!< value data
};

#endif
