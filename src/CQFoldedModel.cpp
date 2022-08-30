#include <CQFoldedModel.h>
#include <CQBaseModel.h>
#include <cassert>
#include <iostream>

//------

CQFoldedModel::
CQFoldedModel(QAbstractItemModel *model, const CQFoldData &foldData)
{
  setObjectName("foldedModel");

  setFoldData(foldData);

  setSourceModel(model);
}

CQFoldedModel::
~CQFoldedModel()
{
  clear();
}

void
CQFoldedModel::
setFoldData(const CQFoldData &foldData)
{
  foldData_ = foldData;

  bucketer_.setType    (foldData_.type      ());
  bucketer_.setRDelta  (foldData_.delta     ());
  bucketer_.setNumAuto (foldData_.numAuto   ());
  bucketer_.setIntegral(foldData_.isIntegral());

  doResetModel();
}

QAbstractItemModel *
CQFoldedModel::
sourceModel() const
{
  auto *sourceModel = QAbstractProxyModel::sourceModel();

  return sourceModel;
}

void
CQFoldedModel::
setSourceModel(QAbstractItemModel *sourceModel)
{
  connectSlots(false);

  sourceFoldedModel_ = qobject_cast<CQFoldedModel *>(sourceModel);

  QAbstractProxyModel::setSourceModel(sourceModel);

  connectSlots(true);

  fold();
}

void
CQFoldedModel::
setFoldColumn(int i)
{
  if (i != foldData_.column()) {
    foldData_.setColumn(i);

    doResetModel();
  }
}

void
CQFoldedModel::
doResetModel()
{
  auto *model = this->sourceModel();
  if (! model) return;

  beginResetModel();

  bucketer_.reset();

  fold();

  endResetModel();
}

void
CQFoldedModel::
setShowFoldColumnData(bool b)
{
  if (b != foldData_.showColumnData()) {
    beginResetModel();

    foldData_.setShowColumnData(b);

    fold();

    endResetModel();
  }
}

void
CQFoldedModel::
setKeepFoldColumn(bool b)
{
  if (b != foldData_.isKeepColumn()) {
    beginResetModel();

    foldData_.setKeepColumn(b);

    fold();

    endResetModel();
  }
}

void
CQFoldedModel::
setBucketCount(int i)
{
  if (i != foldData_.numAuto()) {
    foldData_.setNumAuto(i);

    setFoldData(foldData_);
  }
}

void
CQFoldedModel::
setBucketDelta(double r)
{
  if (r != foldData_.delta()) {
    foldData_.setDelta(r);

    setFoldData(foldData_);
  }
}

void
CQFoldedModel::
connectSlots(bool b)
{
  auto *model = this->sourceModel();
  if (! model) return;

  auto connectDisconnect = [&](bool b, const char *from, const char *to) {
    if (b)
      QObject::connect(model, from, this, to);
    else
      QObject::disconnect(model, from, this, to);
  };

  connectDisconnect(b, SIGNAL(columnsInserted(const QModelIndex &, int, int)), SLOT(foldSlot()));
  connectDisconnect(b, SIGNAL(columnsMoved(const QModelIndex &, int, int,
                              const QModelIndex &, int)), SLOT(foldSlot()));
  connectDisconnect(b, SIGNAL(columnsRemoved(const QModelIndex &, int, int)), SLOT(foldSlot()));

  connectDisconnect(b, SIGNAL(modelReset()), SLOT(foldSlot()));

  connectDisconnect(b, SIGNAL(rowsInserted(const QModelIndex &, int, int)), SLOT(foldSlot()));
  connectDisconnect(b, SIGNAL(rowsMoved(const QModelIndex &, int, int,
                              const QModelIndex &, int)), SLOT(foldSlot()));
  connectDisconnect(b, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), SLOT(foldSlot()));
}

void
CQFoldedModel::
foldSlot()
{
  fold();
}

void
CQFoldedModel::
fold()
{
  clear();

  //---

  // check column valid
  auto *model = this->sourceModel();
  if (! model) return;

  numColumns_ = model->columnCount();

  if (foldColumn() < 0 || foldColumn() >= numColumns_)
    return;

  //---

  if (isKeepFoldColumn())
    ++numColumns_;

  //---

  if (bucketer_.type() != CQBucketer::Type::STRING) {
    bool   rset = false;
    double rmin = 0.0, rmax = 0.0;

    calcRMinMax(model, QModelIndex(), rset, rmin, rmax);

    bucketer_.setRMin(rmin);
    bucketer_.setRMax(rmax);
  }

  //---

  root_ = new Node(QModelIndex());

  foldNode(root_, 0);

  //---

  foldPos_ = 0;

  auto *sourceFoldedModel = sourceFoldedModel_;

  while (sourceFoldedModel) {
    ++foldPos_;

    sourceFoldedModel = sourceFoldedModel->sourceFoldedModel_;
  }

  sourceColumns_.clear();

  //---

  initSourceColumns();

  //---

  folded_ = true;
}

void
CQFoldedModel::
calcRMinMax(QAbstractItemModel *model, const QModelIndex &parent,
            bool &rset, double &rmin, double &rmax) const
{
  int nr = model->rowCount(parent);

  for (int r = 0; r < nr; ++r) {
    auto parent1 = model->index(r, 0, parent);

    int nr1 = model->rowCount(parent1);

    if (nr1 > 0) {
      calcRMinMax(model, parent1, rset, rmin, rmax);
    }
    else {
      auto ind = model->index(r, foldColumn(), parent);

      auto var = model->data(ind, Qt::DisplayRole);

      bool ok;

      double rval = CQBucketer::varReal(var, ok);
      if (! ok) continue;

      if (! rset) {
        rmin = rval;
        rmax = rval;

        rset = true;
      }
      else {
        rmin = std::min(rmin, rval);
        rmax = std::max(rmax, rval);
      }
    }
  }
}

void
CQFoldedModel::
foldNode(Node *parent, int depth)
{
  auto *model = this->sourceModel();
  assert(model);

  // fold rows at column
  int nr = model->rowCount(parent->sourceInd);

  for (int r = 0; r < nr; ++r) {
    // if leaf node then add to folded rows for this parent node
    auto ind = model->index(r, 0, parent->sourceInd);

    if (! model->hasChildren(ind)) {
      // get value for column
      auto ind = model->index(r, foldColumn(), parent->sourceInd);

      auto var = model->data(ind, Qt::DisplayRole);

      int bucket = bucketer_.bucket(var);

      bool isNew = false;

      Node *node = parent->getBucketNode(bucket, depth, isNew);

      if (isNew) {
        node->bucket = bucket;
        node->str    = bucketer_.bucketName(bucket);
      }

      node->addSourceRow(r);
    }
    else {
      auto str = model->data(ind, Qt::DisplayRole).toString();

      Node *child = new Node(parent, ind);

      child->str = str;

      foldNode(child, depth + 1);
    }
  }
}

void
CQFoldedModel::
clear()
{
  delete root_;

  root_ = nullptr;

  folded_ = false;
}

// get number of columns
int
CQFoldedModel::
columnCount(const QModelIndex &parent) const
{
  if (! folded_) {
    auto *model = this->sourceModel();
    if (! model) return 0;

    return model->columnCount(parent);
  }

  return numColumns_;
}

// get number of child rows for parent
int
CQFoldedModel::
rowCount(const QModelIndex &parent) const
{
  if (! folded_) {
    auto *model = this->sourceModel();
    if (! model) return 0;

    return model->rowCount(parent);
  }

  //---

  // children only at column 0
  if (parent.column() > 0)
    return 0;

  // get node data
  NodeData nodeData = indexNode(parent);

  // folded node has no child (no children)
  if (! nodeData.child)
    return 0;

  return nodeData.child->numRows();
}

// get child node for row/column of parent
QModelIndex
CQFoldedModel::
index(int row, int column, const QModelIndex &parent) const
{
  if (! folded_) {
    auto *model = this->sourceModel();
    if (! model) return QModelIndex();

    return model->index(row, column, parent);
  }

  //---

  // get node data
  NodeData nodeData = indexNode(parent);

  if (! nodeData.child)
    return QModelIndex();

  if (row < 0 || row >= int(nodeData.child->numRows()))
    return QModelIndex();

  // model index is row, column and parent node
  return createIndex(row, column, nodeData.child);
}

// get parent for child
QModelIndex
CQFoldedModel::
parent(const QModelIndex &child) const
{
  if (! child.isValid())
    return QModelIndex();

  //---

  if (! folded_) {
    auto *model = this->sourceModel();
    if (! model) return QModelIndex();

    return model->parent(child);
  }

  //---

  // get node data
  NodeData nodeData = indexNode(child);

  Node *node = nodeData.parent;

  if (! node)
    return QModelIndex();

  Node *pnode = node->parent;

  // no parent so return root index
  if (! pnode) {
    return QModelIndex();
    //return createIndex(0, 0, nullptr);
  }

  // get row number for parent's child
  int row = -1;

  for (std::size_t i = 0; i < pnode->children.size(); ++i) {
    if (pnode->children[i] == node) {
      row = int(i);
    }
  }

  if (row < 0)
    return QModelIndex();

  // model index is row, column and parent node
  return createIndex(row, 0, pnode);
}

bool
CQFoldedModel::
hasChildren(const QModelIndex &parent) const
{
  if (! parent.isValid())
    return true;

  //---

  if (! folded_) {
    auto *model = this->sourceModel();
    if (! model) return false;

    return model->hasChildren(parent);
  }

  //---

  // children only at column 0
  if (parent.column() > 0)
    return 0;

  // get node data
  NodeData nodeData = indexNode(parent);

  // folded node has no child (no children)
  if (! nodeData.child)
    return false;

  return nodeData.child->numRows();
}

QVariant
CQFoldedModel::
data(const QModelIndex &index, int role) const
{
  auto *model = this->sourceModel();
  if (! model) return QVariant();

  //---

  if (! folded_) {
    return model->data(index, role);
  }

  //---

  // get node data
  NodeData nodeData = indexNode(index);

  int c = index.column();

  // folded node has no child
  if (! nodeData.child) {
    // fold column only has data after depth
    if (c <= nodeData.parent->depth) {
      if (! showFoldColumnData())
        return QVariant();
    }

    // get source model index for folded row number
    auto ind1 = foldedChildIndex(nodeData.parent, index.row(), index.column());

    if (! ind1.isValid())
      return QVariant();

    return model->data(ind1, role);
  }
  // non-folded node has child
  else {
    // handle fold node
    if (nodeData.child->isFolded()) {
      // return fold text in fold column for display role
      // TODO: other roles
      if      (role == Qt::DisplayRole) {
        if (c == foldPos())
          return nodeData.child->str;

        return "";
      }
      else if (role == Qt::EditRole) {
        if (c == foldPos())
          return nodeData.child->bucket;

        return "";
      }
      else if (role == Qt::ToolTipRole) {
        if (c == foldPos())
          return QString("%1 (%2)").arg(nodeData.child->str).arg(nodeData.child->bucket);

        return "";
      }
    }
    // forward non-fold node to source mode
    else {
      auto ind1 = mapToSource(index);

      return model->data(ind1, role);
    }

    return QVariant();
  }
}

bool
CQFoldedModel::
setData(const QModelIndex &index, const QVariant &value, int role)
{
  auto *model = this->sourceModel();
  if (! model) return false;

  //---

  if (! folded_) {
    return model->setData(index, value, role);
  }

  //---

  // get node data
  NodeData nodeData = indexNode(index);

  int c = index.column();

  // folded node has no child
  if (! nodeData.child) {
    // fold column only has data after depth
    if (c <= nodeData.parent->depth)
      return false;

    // get source model index for folded row number
    auto ind1 = foldedChildIndex(nodeData.parent, index.row(), index.column());

    if (! ind1.isValid())
      return false;

    return model->setData(ind1, value, role);
  }
  // non-folded node has child
  else {
    // folded text in depth column
    if (c != nodeData.child->depth)
      return false;

    // TODO: could set and re-fold (messy), does this set all child nodes
    //if (role == Qt::DisplayRole)
    //  nodeData.child->str = value.toString();

    return false;
  }
}

QVariant
CQFoldedModel::
headerData(int section, Qt::Orientation orientation, int role) const
{
  auto *model = this->sourceModel();
  if (! model) return QVariant();

  //---

  if (! folded_) {
    return model->headerData(section, orientation, role);
  }

  //---

  if (section < 0 || section >= columnCount())
    return QVariant();

  //---

  if (orientation != Qt::Horizontal)
    return model->headerData(section, orientation, role);

  //---

  if (role == static_cast<int>(CQBaseModelRole::Type) ||
      role == static_cast<int>(CQBaseModelRole::BaseType) ||
      role == static_cast<int>(CQBaseModelRole::TypeValues) ||
      role == static_cast<int>(CQBaseModelRole::Min) ||
      role == static_cast<int>(CQBaseModelRole::Max) ||
      role == static_cast<int>(CQBaseModelRole::Sorted) ||
      role == static_cast<int>(CQBaseModelRole::SortOrder)) {
    // get node data
    auto ind = index(0, section, QModelIndex());

    NodeData nodeData = indexNode(ind);

    // folded node has no child (no children)
    if (! nodeData.child || ! nodeData.parent || section > nodeData.parent->depth) {
      int c = mapColumnToSource(section);
      if (c < 0) return QVariant();

      return model->headerData(c, orientation, role);
    }

    if (section == foldColumn()) {
      if (role == static_cast<int>(CQBaseModelRole::Type) ||
          role == static_cast<int>(CQBaseModelRole::BaseType)) {
        return QVariant(static_cast<int>(CQBaseModelType::STRING));
      }
    }

    int c = mapColumnToSource(section);
    if (c < 0) return QVariant();

    auto value = model->headerData(c, orientation, role);

    return value;
  }
  else if (role == static_cast<int>(CQBaseModelRole::Key)) {
    if (section == foldColumn())
      return QVariant(true);
  }

  //---

  int c = mapColumnToSource(section);
  if (c < 0) return QVariant();

  auto var = model->headerData(c, orientation, role);

  if (var.isValid())
    return var;

  return QVariant();
}

bool
CQFoldedModel::
setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  auto *model = this->sourceModel();
  if (! model) return false;

  //---

  if (! folded_) {
    return model->setHeaderData(section, orientation, value, role);
  }

  //---

  if (section < 0 || section >= columnCount())
    return false;

  //---

  int c = mapColumnToSource(section);
  if (c < 0) return false;

  if (model->setHeaderData(c, orientation, value, role))
    return true;

  return false;
}

Qt::ItemFlags
CQFoldedModel::
flags(const QModelIndex &index) const
{
  auto *model = this->sourceModel();
  if (! model) return Qt::ItemFlags();

  //---

  if (! folded_) {
    return model->flags(index);
  }

  //---

  // get node data
  NodeData nodeData = indexNode(index);

  int c = index.column();

  // folded node has no child
  if (! nodeData.child) {
    // fold column only has data after depth
    if (c <= nodeData.parent->depth)
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // get source model index for folded row number
    auto ind1 = foldedChildIndex(nodeData.parent, index.row(), index.column());

    if (! ind1.isValid())
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return model->flags(ind1);
  }
  // non-folded node has child
  else {
    // folded text in depth column
    if (c != nodeData.child->depth)
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
}

// map index in source model to folded model
QModelIndex
CQFoldedModel::
mapFromSource(const QModelIndex &sourceIndex) const
{
  if (! folded_)
    return sourceIndex;

  //---

  if (! sourceIndex.isValid())
    return QModelIndex();

  //---

  // if no parent we are at top level node
  if (! sourceIndex.parent().isValid())
    return findSourceIndex(root_, sourceIndex);

  //---

  // map parent index
  auto parent = mapFromSource(sourceIndex.parent());

  if (! parent.isValid())
    return QModelIndex();

  // get node data
  NodeData nodeData = indexNode(parent);

  Node *pnode = nodeData.child;

  if (! pnode)
    return QModelIndex();

  return findSourceIndex(pnode, sourceIndex);
}

// map index in folded model to source model
QModelIndex
CQFoldedModel::
mapToSource(const QModelIndex &proxyIndex) const
{
  if (! folded_)
    return proxyIndex;

  //---

  if (! proxyIndex.isValid())
    return QModelIndex();

  int r = proxyIndex.row();
  int c = proxyIndex.column();

  // get node data
  NodeData nodeData = indexNode(proxyIndex);

  // folded node has no child
  if (! nodeData.child) {
    // fold column is empty
    //if (c == foldColumn()) return QModelIndex();

    return foldedChildIndex(nodeData.parent, r, c);
  }
  else {
    // only folded column is valid
    //if (c != foldColumn()) return QModelIndex();

    Node *node = nodeData.child;

    int r1 = node->sourceInd.row();

    int c1 = mapColumnToSource(c);
    if (c1 < 0) return QModelIndex();

    Node *pnode = nodeData.parent;

    auto *model = this->sourceModel();
    if (! model) return QModelIndex();

    if (! pnode)
      return model->index(r1, c1, QModelIndex());

    return model->index(r1, c1, pnode->sourceInd);
  }
}

// folded column number to source column number
int
CQFoldedModel::
mapColumnToSource(int column) const
{
  if (column < 0 || column >= numColumns_)
    return -1;

  return sourceColumns_[size_t(column)];
}

// source column number to folded column number
int
CQFoldedModel::
mapColumnFromSource(int column) const
{
  for (int i = 0; i < numColumns_; ++i)
    if (sourceColumns_[size_t(i)] == column)
      return i;

  return -1;
}

// init column number map
void
CQFoldedModel::
initSourceColumns()
{
  sourceColumns_.resize(size_t(numColumns_));

  for (int i = 0; i < numColumns_; ++i)
    sourceColumns_[size_t(i)] = i;

  if (! isKeepFoldColumn()) {
    if      (foldPos() < foldColumn()) {
      for (int i = foldColumn(); i >= foldPos() + 1; --i)
        sourceColumns_[size_t(i)] = sourceColumns_[size_t(i - 1)];

      sourceColumns_[size_t(foldPos())] = foldColumn();
    }
    else if (foldPos() > foldColumn()) {
      for (int i = foldColumn() + 1; i <= foldPos(); ++i)
        sourceColumns_[size_t(i - 1)] = sourceColumns_[size_t(i)];

      sourceColumns_[size_t(foldPos())] = foldColumn();
    }
  }
  else {
    for (int i = numColumns_ - 1; i >= foldPos() + 1; --i)
      sourceColumns_[size_t(i)] = sourceColumns_[size_t(i - 1)];

    sourceColumns_[size_t(foldPos())] = foldColumn();
  }
}

// get source index from folded parent node, row and column
QModelIndex
CQFoldedModel::
foldedChildIndex(Node *pnode, int row, int column) const
{
  if (row < 0 || row >= int(pnode->sourceRows.size()))
    return QModelIndex();

  // create source model index for folded row number
  auto *model = this->sourceModel();
  if (! model) return QModelIndex();

  int r = pnode->sourceRows[size_t(row)];

  int c = mapColumnToSource(column);
  if (c < 0) return QModelIndex();

  return model->index(r, c, pnode->sourceInd);
}

// get folded index for source index given folded parent node
QModelIndex
CQFoldedModel::
findSourceIndex(Node *pnode, const QModelIndex &sourceIndex) const
{
  int c = sourceIndex.column();

  int c1 = mapColumnFromSource(c);
  if (c1 < 0) return QModelIndex();

  int r1 = 0;

  for (const auto &node : pnode->children) {
    if (! node->isFolded()) {
      if (node->sourceInd == sourceIndex) {
        // model index is row, column and parent node
        return createIndex(r1, c1, pnode);
      }
    }
    else {
      if (node->hasSourceRow(sourceIndex.row())) {
        // TODO: return child folded node ?
        return createIndex(r1, c1, pnode);
      }
    }

    ++r1;
  }

  return QModelIndex();
}

CQFoldedModel::NodeData
CQFoldedModel::
indexNode(const QModelIndex &ind) const
{
  // invalid index is root
  if (! ind.isValid())
    return NodeData(nullptr, root_);

  Node *pnode = static_cast<Node *>(ind.internalPointer());

  if (! pnode)
    return NodeData(nullptr, root_);

  // return child for non-folded
  if (! pnode->isFolded()) {
    assert(ind.row() >= 0 && ind.row() < int(pnode->children.size()));

    return NodeData(pnode, pnode->children[size_t(ind.row())]);
  }

  // no child for folded
  return NodeData(pnode, nullptr);
}
