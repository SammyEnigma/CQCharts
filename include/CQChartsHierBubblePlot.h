#ifndef CQChartsHierBubblePlot_H
#define CQChartsHierBubblePlot_H

#include <CQChartsHierPlot.h>
#include <CQChartsPlotObj.h>
#include <CQChartsCirclePack.h>
#include <CQChartsDisplayRange.h>
#include <CQChartsData.h>
#include <CQChartsArea.h>
#include <QModelIndex>

//---

/*!
 * \brief Hierarchical Bubble Plot Type
 * \ingroup Charts
 */
class CQChartsHierBubblePlotType : public CQChartsHierPlotType {
 public:
  CQChartsHierBubblePlotType();

  QString name() const override { return "hierbubble"; }
  QString desc() const override { return "HierBubble"; }

  Category category() const override { return Category::ONE_D; }

  void addParameters() override;

  bool customXRange() const override { return false; }
  bool customYRange() const override { return false; }

  bool hasAxes() const override { return false; }
  bool hasKey () const override { return false; }

  bool allowXLog() const override { return false; }
  bool allowYLog() const override { return false; }

  bool canProbe() const override { return false; }

  bool canEqualScale() const override { return true; }

  bool supportsIdColumn() const override { return true; }

  QString description() const override;

  Plot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsHierBubblePlot;
class CQChartsHierBubbleHierNode;

/*!
 * \brief Hierarchical Bubble Plot Node
 * \ingroup Charts
 */
class CQChartsHierBubbleNode : public CQChartsCircleNode {
 protected:
  static uint nextId() {
    static uint lastId = 0;

    return ++lastId;
  }

 public:
  using HierBubblePlot = CQChartsHierBubblePlot;
  using Node           = CQChartsHierBubbleNode;
  using HierNode       = CQChartsHierBubbleHierNode;
  using Color          = CQChartsColor;
  using ColorInd       = CQChartsUtil::ColorInd;

 public:
  CQChartsHierBubbleNode(const HierBubblePlot *plot, HierNode *parent, const QString &name,
                         double size, const QModelIndex &ind);

  virtual ~CQChartsHierBubbleNode();

  void initRadius();

  const HierBubblePlot *hierBubblePlot() const { return hierBubblePlot_; }

  HierNode *parent() const { return parent_; }

  virtual uint id() const { return id_; }

  virtual const QString &name() const { return name_; }

  //! get/set size
  virtual double size() const { return size_; }
  virtual void setSize(double s) { size_ = s; }

  //! get/set radius
  double radius() const override { return r_; }
  virtual void setRadius(double r) { r_ = r; }

  //! get/set x
  double x() const override { return CQChartsCircleNode::x(); }
  void setX(double x) override { CQChartsCircleNode::setX(x); }

  //! get/set y
  double y() const override { return CQChartsCircleNode::y(); }
  void setY(double y) override { CQChartsCircleNode::setY(y); }

  //! get/set colorId
  virtual int colorId() const { return colorId_; }
  virtual void setColorId(int id) { colorId_ = id; }

  //! get/set color
  const Color &color() const { return color_; }
  void setColor(const Color &v) { color_ = v; }

  //! get/set single model index
  const QModelIndex &ind() const { return ind_; }
  void setInd(const QModelIndex &i) { ind_ = i; }

  //! get/set depth
  virtual int depth() const { return depth_; }
  virtual void setDepth(int i) { depth_ = i; }

  //! get/set is filler
  bool isFiller() const { return filler_; }
  void setFiller(bool b) { filler_ = b; }

  //! get hier size
  virtual double hierSize() const { return size(); }

  //! get hier name
  virtual QString hierName(const QString &sep) const;

  //! reset placement position
  virtual void resetPosition() {
    //CQChartsCircleNode::setPosition(0.0, 0.0);

    //placed_ = false;
  }

  //! set placement position
  void setPosition(double x, double y) override;

  //! get is placed
  virtual bool placed() const { return placed_; }

  //---

  //! sort by radius
  friend bool operator<(const Node &n1, const Node &n2) { return n1.r_ < n2.r_; }
  friend bool operator>(const Node &n1, const Node &n2) { return n1.r_ > n2.r_; }

  //---

  //! interp color
  virtual QColor interpColor(const HierBubblePlot *plot, const Color &c, const ColorInd &colorInd,
                             int n) const;

  //---

  virtual QString calcGroupName() const;

 protected:
  const HierBubblePlot* hierBubblePlot_ { nullptr }; //!< parent plot
  HierNode*             parent_         { nullptr }; //!< parent hier node
  uint                  id_             { 0 };       //!< node id
  QString               name_;                       //!< node name
  double                size_           { 0.0 };     //!< node size
  int                   colorId_        { -1 };      //!< node color index
  Color                 color_;                      //!< node explicit color
  QModelIndex           ind_;                        //!< node model index
  int                   depth_          { 0 };       //!< node depth
  bool                  filler_         { false };   //!< is filler
  bool                  placed_         { false };   //!< is placed

//mutable QString hierName_;    //!< hier name
//mutable QChar   hierNameSep_; //!< hier name separator
  mutable int     ig_ { -1 };   //!< group ind
  mutable int     ng_ { -1 };   //!< num groups
};

//---

/*!
 * \brief hier bubble node compare functor
 * \ingroup Charts
 */
struct CQChartsHierBubbleNodeCmp {
  using Node = CQChartsHierBubbleNode;

  CQChartsHierBubbleNodeCmp(bool reverse=false) :
   reverse(reverse) {
  }

  bool operator()(const Node *n1, const Node *n2) {
    if (! reverse)
      return (*n1) < (*n2);
    else
      return (*n1) > (*n2);
  }

  bool reverse { false };
};

//---

/*!
 * \brief Hierarchical Bubble Plot Hierarchical Node
 * \ingroup Charts
 */
class CQChartsHierBubbleHierNode : public CQChartsHierBubbleNode {
 public:
  using HierBubblePlot = CQChartsHierBubblePlot;
  using HierNode       = CQChartsHierBubbleHierNode;
  using Node           = CQChartsHierBubbleNode;
  using Nodes          = std::vector<Node*>;
  using Children       = std::vector<HierNode*>;
  using Pack           = CQChartsCirclePack<Node>;

 public:
  CQChartsHierBubbleHierNode(const HierBubblePlot *plot, HierNode *parent, const QString &name,
                             const QModelIndex &ind=QModelIndex());

 ~CQChartsHierBubbleHierNode();

  //! get/set hierarchical index
  int hierInd() const { return hierInd_; }
  void setHierInd(int i) { hierInd_ = i; }

  //---

  //! get/set group name
  const QString &groupName() const { return groupName_; }
  void setGroupName(const QString &v) { groupName_ = v; }

  //---

  //! get/set is visible
  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  bool isHierVisible() const;

  //---

  //! get/set is expanded
  bool isExpanded() const { return expanded_; }
  void setExpanded(bool b) { expanded_ = b; }

  bool isHierExpanded() const;

  //---

  //! get hierarchical size
  double hierSize() const override;

  //---

  //! get has child nodes
  bool hasNodes() const { return ! nodes_.empty(); }

  //! get child nodes
  const Nodes &getNodes() const { return nodes_; }
  Nodes &getNodes() { return nodes_; }

  //---

  //! get pack data
  const Pack &pack() const { return pack_; }
  Pack &pack() { return pack_; }

  //---

  //! has child hier nodes
  bool hasChildren() const { return ! children_.empty(); }

  //! get child hier nodes
  const Children &getChildren() const { return children_; }

  void addChild(HierNode *child);

  //---

  //! pack child nodes
  void packNodes();

  //! add child node
  void addNode(Node *node);

  //! remove child node
  void removeNode(Node *node);

  //! set node position
  void setPosition(double x, double y) override;

  //! interp color
  QColor interpColor(const HierBubblePlot *plot, const CQChartsColor &c, const ColorInd &colorInd,
                     int n) const override;

  //---

  QString calcGroupName() const override;

 protected:
  QString  groupName_;         //!< group name
  Nodes    nodes_;             //!< child nodes
  Pack     pack_;              //!< circle pack
  Children children_;          //!< child hier nodes
  int      hierInd_  { -1 };   //!< hier index
  bool     visible_  { true }; //!< is visible
  bool     expanded_ { true }; //!< is expanded
};

//---

class CQChartsHierBubbleHierObj;

/*!
 * \brief Hierarchical Bubble Plot object
 * \ingroup Charts
 */
class CQChartsHierBubbleNodeObj : public CQChartsPlotObj {
  Q_OBJECT

  Q_PROPERTY(int     ind  READ ind WRITE setInd)
  Q_PROPERTY(QString name READ hierName)
  Q_PROPERTY(double  size READ hierSize)

 public:
  using HierBubblePlot = CQChartsHierBubblePlot;
  using Node           = CQChartsHierBubbleNode;
  using HierObj        = CQChartsHierBubbleHierObj;
  using NodeObj        = CQChartsHierBubbleNodeObj;
  using Angle          = CQChartsAngle;
  using OptReal        = CQChartsOptReal;
  using Units          = CQChartsUnits::Type;

 public:
  CQChartsHierBubbleNodeObj(const HierBubblePlot *plot, Node *node, HierObj *hierObj,
                            const BBox &rect, const ColorInd &is);

  Node *node() const { return node_; }

  HierObj *parent() const { return hierObj_; }

  //! get/set index
  int ind() const { return ind_; }
  void setInd(int ind) { ind_ = ind; }

  QString hierName(const QString &sep="/") const { return node()->hierName(sep); }
  double  hierSize() const { return node()->hierSize(); }

  //---

  QString typeName() const override { return "bubble"; }

  bool isCircle() const override { return true; }

  double radius() const override { return node_->radius(); }

  QString calcId() const override;

  QString calcTipId() const override;

  //---

  virtual QString calcGroupName() const;

  //---

  void addChild(NodeObj *child) { children_.push_back(child); }

  bool inside(const Point &p) const override;

  void getObjSelectIndices(Indices &inds) const override;

  //---

  void draw(PaintDevice *device) const override;

  void drawText(PaintDevice *device, const BBox &bbox, const QColor &brushColor,
                bool updateState) const;

  //---

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const override;

  //---

  bool isMinArea() const;

  bool isCirclePoint() const;

  //---

  bool isChildSelected() const;

 protected:
  using Children = std::vector<NodeObj *>;

  const HierBubblePlot* hierBubblePlot_ { nullptr }; //!< parent plot
  Node*                 node_           { nullptr }; //!< associated node
  HierObj*              hierObj_        { nullptr }; //!< parent hier obj
  Children              children_;                   //!< child objects
  int                   ind_            { 0 };       //!< ind

  mutable int numColorIds_ { -1 }; //!< associated group's number of color ids
};

//---

/*!
 * \brief Hierarchical Bubble Plot Hierarchical object
 * \ingroup Charts
 */
class CQChartsHierBubbleHierObj : public CQChartsHierBubbleNodeObj {
 public:
  using HierBubblePlot = CQChartsHierBubblePlot;
  using HierNode       = CQChartsHierBubbleHierNode;
  using HierObj        = CQChartsHierBubbleHierObj;

 public:
  CQChartsHierBubbleHierObj(const HierBubblePlot *plot, HierNode *hier, HierObj *hierObj,
                            const BBox &rect, const ColorInd &is);

  HierNode *hierNode() const { return hier_; }

  //---

  QString calcId() const override;

  QString calcTipId() const override;

  //---

  QString calcGroupName() const override;

  //---

  bool isCircle() const override { return true; }

  double radius() const override { return hierNode()->radius(); }

  //---

  bool inside(const Point &p) const override;

  bool isSelectable() const override;

  void getObjSelectIndices(Indices &inds) const override;

  //---

  void draw(PaintDevice *device) const override;

  //---

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const override;

 protected:
  HierNode* hier_ { nullptr }; //!< associated hier node
};

//---

/*!
 * \brief Hierarchical Bubble Plot
 * \ingroup Charts
 *
 * Plot Type
 *   + \ref CQChartsHierBubblePlotType
 *
 * Example
 *   + \image html hierbubbleplot.png
 */
class CQChartsHierBubblePlot : public CQChartsHierPlot,
 public CQChartsObjHierShapeData<CQChartsHierBubblePlot>,
 public CQChartsObjShapeData    <CQChartsHierBubblePlot>,
 public CQChartsObjTextData     <CQChartsHierBubblePlot> {
  Q_OBJECT

  // coloring
  Q_PROPERTY(NodeColorType nodeColorType READ nodeColorType WRITE setNodeColorType)
  Q_PROPERTY(HierColorType hierColorType READ hierColorType WRITE setHierColorType)

  // hier node shape (stroke, fill)
  CQCHARTS_NAMED_SHAPE_DATA_PROPERTIES(Hier, hier)

  // node options
  Q_PROPERTY(bool         valueLabel   READ isValueLabel  WRITE setValueLabel  )
  Q_PROPERTY(ValueCombine valueCombine READ valueCombine  WRITE setValueCombine)
  Q_PROPERTY(bool         sorted       READ isSorted      WRITE setSorted      )
  Q_PROPERTY(bool         sortReverse  READ isSortReverse WRITE setSortReverse )

  // node shape
  CQCHARTS_SHAPE_DATA_PROPERTIES

  // node text
  CQCHARTS_TEXT_DATA_PROPERTIES

  // global options
  Q_PROPERTY(CQChartsOptReal minSize     READ minSize       WRITE setMinSize)
  Q_PROPERTY(CQChartsArea    minArea     READ minArea       WRITE setMinArea)

  // grouping
  Q_PROPERTY(bool splitGroups  READ isSplitGroups  WRITE setSplitGroups )
  Q_PROPERTY(bool groupPalette READ isGroupPalette WRITE setGroupPalette)

  Q_ENUMS(ValueCombine)
  Q_ENUMS(NodeColorType)
  Q_ENUMS(HierColorType)

 public:
  enum class ValueCombine {
    SKIP,
    REPLACE,
    WARN,
    SUM
  };

  enum class NodeColorType {
    ID,
    PARENT_VALUE,
    GLOBAL_VALUE
  };

  enum class HierColorType {
    ID,
    BLEND,
    PARENT_VALUE,
    GLOBAL_VALUE
  };

  using Node      = CQChartsHierBubbleNode;
  using Pack      = CQChartsCirclePack<Node>;
  using Nodes     = std::vector<Node*>;
  using HierNode  = CQChartsHierBubbleHierNode;
  using HierObj   = CQChartsHierBubbleHierObj;
  using NodeObj   = CQChartsHierBubbleNodeObj;

  using Length    = CQChartsLength;
  using Area      = CQChartsArea;
  using Color     = CQChartsColor;
  using Alpha     = CQChartsAlpha;
  using PenData   = CQChartsPenData;
  using PenBrush  = CQChartsPenBrush;
  using BrushData = CQChartsBrushData;
  using ColorInd  = CQChartsUtil::ColorInd;

 public:
  CQChartsHierBubblePlot(View *view, const ModelP &model);
 ~CQChartsHierBubblePlot();

  //---

  void init() override;
  void term() override;

  //---

  //! get/set value combine policy
  const ValueCombine &valueCombine() const { return valueCombine_; }
  void setValueCombine(const ValueCombine &combine);

  //! get/set is value label
  bool isValueLabel() const { return valueLabel_; }
  void setValueLabel(bool b);

  //---

  //! get/set is sorted
  bool isSorted() const { return sortData_.enabled; }
  void setSorted(bool b);

  //! get/set is sort reverse
  bool isSortReverse() const { return sortData_.reverse; }
  void setSortReverse(bool b);

  //---

  //! get/set min size
  const OptReal &minSize() const { return minSize_; }
  void setMinSize(const OptReal &r);

  //! get/set min area
  const Area &minArea() const { return minArea_; }
  void setMinArea(const Area &a);

  //---

  //! get/set split groups
  bool isSplitGroups() const { return splitGroups_; }
  void setSplitGroups(bool b);

  //! get/set group palette
  bool isGroupPalette() const { return groupPalette_; }
  void setGroupPalette(bool b);

  //---

  void setTextFontSize(double s);

  //---

  HierNode *root(const QString &groupName) const;

  HierNode *firstHier(const QString &groupName) const;

  //! get/set current root
  HierNode *currentRoot(const QString &groupName) const;
  void setCurrentRoot(const QString &groupName, HierNode *r, bool update=false);

  //---

  //! get/set offset
  const Point &offset() const { return placeData_.offset; }
  void setOffset(const Point &o) { placeData_.offset = o; }

  //! get/set scale
  double scale() const { return placeData_.scale; }
  void setScale(double r) { placeData_.scale = r; }

  //---

  int colorId(const QString &groupName) const;

  int numColorIds(const QString &groupName) const;

  void initColorIds(const QString &groupName);

  int nextColorId(const QString &groupName);

  //---

  //! get/set node color type
  const NodeColorType &nodeColorType() const { return nodeColorType_; }
  void setNodeColorType(const NodeColorType &type);

  //! get/set hier color type
  const HierColorType &hierColorType() const { return hierColorType_; }
  void setHierColorType(const HierColorType &type);

  //---

  void addProperties() override;

  Range calcRange() const override;

  void clearPlotObjects() override;

  bool createObjs(PlotObjs &objs) const override;

  //---

  void postResize() override;

  //---

  bool hasForeground() const override;

  void execDrawForeground(PaintDevice *) const override;

  //---

  bool addMenuItems(QMenu *menu, const Point &p) override;

  //---

  int numGroups() const override;
  int groupNum(const QString &groupName) const;

  //---

  bool hasValueRange() const { return hasValueRange_; }

  double minValue() const { return minValue_; }
  double maxValue() const { return maxValue_; }

 protected:
  void initNodeObjs(HierNode *hier, const QString &groupName, HierObj *parentObj,
                    int depth, PlotObjs &objs) const;

  void resetNodes();

  void initNodes() const;

  void replaceNodes() const;

  void placeNodes(HierNode *hier, const BBox &bbox) const;

  //---

  void colorGroupNodes(const QString &groupName) const;

  void colorNodes(const QString &groupName, HierNode *hier) const;

  void colorNode(const QString &groupName, Node *node) const;

  //---

  void loadHier() const;

  HierNode *addHierNode(const QString &groupName, HierNode *parent, const QString &name,
                        const QModelIndex &nameInd) const;

  Node *hierAddNode(const QString &groupName, HierNode *parent, const QString &name,
                    double size, const QModelIndex &nameInd) const;

  void loadFlat() const;

  Node *flatAddNode(const QString &groupName, const QStringList &nameStrs,
                    double size, const QModelIndex &nameInd, const QString &name) const;

  void addExtraNodes(HierNode *hier) const;

  //---

  int nextHierInd(const QString &groupName) const;

  int groupInd(const QString &groupName) const;
  int nextGroupInd(const QString &groupName) const;

  int valueInd(const QString &groupName) const;
  int nextValueInd(const QString &groupName) const;

  int maxDepth(const QString &groupName) const;
  void updateMaxDepth(const QString &groupName, int depth) const;

  //---

  HierNode *childHierNode(HierNode *parent, const QString &name) const;
  Node *childNode(HierNode *parent, const QString &name) const;

  //---

  void initNodes(HierNode *hier) const;

  void transformNodes(HierNode *hier, const BBox &rect) const;

  void drawBounds(PaintDevice *device, const QString &groupName, HierNode *hier) const;

  //---

  void followViewExpandChanged() override;

  void modelViewExpansionChanged() override;
  void setNodeExpansion(HierNode *hierNode, const std::set<QModelIndex> &indSet);

  void resetNodeExpansion();
  void resetNodeExpansion(HierNode *hierNode);

  //---

  bool getValueSize(const ModelIndex &ind, double &size) const;

  //---

  virtual HierNode *createHierNode(HierNode *parent, const QString &name,
                                   const QModelIndex &nameInd) const;

  virtual Node *createNode(HierNode *parent, const QString &name,
                           double size, const QModelIndex &nameInd) const;

  virtual HierObj *createHierObj(HierNode *hier, HierObj *hierObj,
                                 const BBox &rect, const ColorInd &is) const;
  virtual NodeObj *createNodeObj(Node *node, HierObj *hierObj,
                                 const BBox &rect, const ColorInd &is) const;

 public Q_SLOTS:
  void pushSlot();
  void popSlot();
  void popTopSlot();

 private:
  void popTop(bool update);

  void updateCurrentRoot();

 protected:
  CQChartsPlotCustomControls *createCustomControls() override;

 private:
  struct PlaceData {
    Point  offset { 0, 0 }; //!< draw offset
    double scale  { 1.0 };  //!< draw scale
  };

  struct SortData {
    bool enabled { false };
    bool reverse { false };
  };

  struct ColorData {
    int colorId     { -1 }; //!< current color id
    int numColorIds { 0 };  //!< num used color ids

    void reset() {
      colorId     = -1;
      numColorIds = 0;
    }
  };

  // hier bubble data
  struct HierBubbleData {
    HierNode*   root             { nullptr }; //!< root node
    HierNode*   firstHier        { nullptr }; //!< first hier node
    QString     currentRootName;              //!< current root name
    int         hierInd          { 0 };       //!< current hier ind
    int         maxDepth         { 1 };       //!< max hier depth
    mutable int ig               { 0 };       //!< current group index
    mutable int in               { 0 };       //!< current node index
    ColorData   colorData;                    //!< color index data

    mutable BBox rect; //!< placement rect

    void reset() {
      root      = nullptr;
      firstHier = nullptr;
      hierInd   = 0;
      maxDepth  = 1;
      ig        = 0;
      in        = 0;

      colorData.reset();
    }
  };

  using GroupHierBubbleData = std::map<QString, HierBubbleData>;
  using GroupNameSet        = std::set<QString>;

 private:
  HierBubbleData &getHierBubbleData(const QString &groupName) const;

 private:
  ValueCombine valueCombine_ { ValueCombine::SKIP }; //!< value combine policy
  bool         valueLabel_   { false };              //!< draw value with name
  SortData     sortData_;                            //!< sort data

  OptReal minSize_;                //!< min size
  Area    minArea_;                //!< min area
  bool    splitGroups_  { false }; //!< split groups
  bool    groupPalette_ { false }; //!< use separate palette per group

  PlaceData placeData_; //!< place data

  NodeColorType nodeColorType_ { NodeColorType::ID };    //!< node color type
  HierColorType hierColorType_ { HierColorType::BLEND }; //!< hier node color type

  GroupHierBubbleData groupHierBubbleData_; //!< grouped hier bubble data
  HierBubbleData      hierBubbleData_;      //!< hier bubble data

  GroupNameSet groupNameSet_; //!< group name set

  mutable QString menuGroupName_; //!< group name for object at menu invocation

  mutable bool   hasValueRange_ { false };
  mutable double minValue_      { 0.0 };
  mutable double maxValue_      { 1.0 };
};

//---

/*!
 * \brief Hier Bubble Plot plot custom controls
 * \ingroup Charts
 */
class CQChartsHierBubblePlotCustomControls : public CQChartsHierPlotCustomControls {
  Q_OBJECT

 public:
  CQChartsHierBubblePlotCustomControls(CQCharts *charts);

  void init() override;

  void addWidgets() override;

  void setPlot(Plot *plot) override;

 public Q_SLOTS:
  void updateWidgets() override;

 protected:
  void connectSlots(bool b) override;

  //---

  CQChartsColor getColorValue() override;
  void setColorValue(const CQChartsColor &c) override;

 protected:
  CQChartsHierBubblePlot* hierBubblePlot_ { nullptr };
};

#endif
