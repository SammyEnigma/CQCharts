#include <CQChartsPolygonEdit.h>
#include <CQChartsUnitsEdit.h>

#include <CQPropertyView.h>
#include <CQPixmapCache.h>
#include <CQPoint2DEdit.h>
#include <CQWidgetMenu.h>

#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>

#include <svg/add_svg.h>
#include <svg/remove_svg.h>

CQChartsPolygonLineEdit::
CQChartsPolygonLineEdit(QWidget *parent) :
 CQChartsLineEditBase(parent)
{
  setObjectName("polygonEdit");

  //---

  menuEdit_ = new CQChartsPolygonEdit;

  menu_->setWidget(menuEdit_);

  connect(menuEdit_, SIGNAL(polygonChanged()), this, SLOT(menuEditChanged()));
}

const CQChartsPolygon &
CQChartsPolygonLineEdit::
polygon() const
{
  return menuEdit_->polygon();
}

void
CQChartsPolygonLineEdit::
setPolygon(const CQChartsPolygon &polygon)
{
  updatePolygon(polygon, /*updateText*/ true);
}

void
CQChartsPolygonLineEdit::
updatePolygon(const CQChartsPolygon &polygon, bool updateText)
{
  connectSlots(false);

  menuEdit_->setPolygon(polygon);

  if (updateText)
    polygonToWidgets();

  connectSlots(true);

  emit polygonChanged();
}

void
CQChartsPolygonLineEdit::
textChanged()
{
  CQChartsPolygon polygon(edit_->text());

  if (! polygon.isValid())
    return;

  updatePolygon(polygon, /*updateText*/ false);
}

void
CQChartsPolygonLineEdit::
polygonToWidgets()
{
  connectSlots(false);

  if (polygon().isValid())
    edit_->setText(polygon().toString());
  else
    edit_->setText("");

  connectSlots(true);
}

void
CQChartsPolygonLineEdit::
menuEditChanged()
{
  polygonToWidgets();

  emit polygonChanged();
}

void
CQChartsPolygonLineEdit::
connectSlots(bool b)
{
  CQChartsLineEditBase::connectSlots(b);

  if (b)
    connect(menuEdit_, SIGNAL(polygonChanged()), this, SLOT(menuEditChanged()));
  else
    disconnect(menuEdit_, SIGNAL(polygonChanged()), this, SLOT(menuEditChanged()));
}

//------

#include <CQPropertyViewItem.h>
#include <CQPropertyViewDelegate.h>

CQChartsPolygonPropertyViewType::
CQChartsPolygonPropertyViewType()
{
}

CQPropertyViewEditorFactory *
CQChartsPolygonPropertyViewType::
getEditor() const
{
  return new CQChartsPolygonPropertyViewEditor;
}

bool
CQChartsPolygonPropertyViewType::
setEditorData(CQPropertyViewItem *item, const QVariant &value)
{
  return item->setData(value);
}

void
CQChartsPolygonPropertyViewType::
draw(const CQPropertyViewDelegate *delegate, QPainter *painter,
     const QStyleOptionViewItem &option, const QModelIndex &ind,
     const QVariant &value, bool inside)
{
  delegate->drawBackground(painter, option, ind, inside);

  CQChartsPolygon polygon = value.value<CQChartsPolygon>();

  QString str = polygon.toString();

  QFontMetricsF fm(option.font);

  double w = fm.width(str);

  //---

  QStyleOptionViewItem option1 = option;

  option1.rect.setRight(option1.rect.left() + w + 8);

  delegate->drawString(painter, option1, str, ind, inside);
}

QString
CQChartsPolygonPropertyViewType::
tip(const QVariant &value) const
{
  QString str = value.value<CQChartsPolygon>().toString();

  return str;
}

//------

CQChartsPolygonPropertyViewEditor::
CQChartsPolygonPropertyViewEditor()
{
}

QWidget *
CQChartsPolygonPropertyViewEditor::
createEdit(QWidget *parent)
{
  CQChartsPolygonLineEdit *edit = new CQChartsPolygonLineEdit(parent);

  return edit;
}

void
CQChartsPolygonPropertyViewEditor::
connect(QWidget *w, QObject *obj, const char *method)
{
  CQChartsPolygonLineEdit *edit = qobject_cast<CQChartsPolygonLineEdit *>(w);
  assert(edit);

  QObject::connect(edit, SIGNAL(polygonChanged()), obj, method);
}

QVariant
CQChartsPolygonPropertyViewEditor::
getValue(QWidget *w)
{
  CQChartsPolygonLineEdit *edit = qobject_cast<CQChartsPolygonLineEdit *>(w);
  assert(edit);

  return QVariant::fromValue(edit->polygon());
}

void
CQChartsPolygonPropertyViewEditor::
setValue(QWidget *w, const QVariant &var)
{
  CQChartsPolygonLineEdit *edit = qobject_cast<CQChartsPolygonLineEdit *>(w);
  assert(edit);

  CQChartsPolygon polygon = var.value<CQChartsPolygon>();

  edit->setPolygon(polygon);
}

//------

CQChartsPolygonEdit::
CQChartsPolygonEdit(QWidget *parent) :
 QFrame(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(2);

  //---

  controlFrame_ = new QFrame;

  QHBoxLayout *controlFrameLayout = new QHBoxLayout(controlFrame_);
  controlFrameLayout->setMargin(0); controlFrameLayout->setSpacing(2);

  layout->addWidget(controlFrame_);

  //---

  unitsEdit_ = new CQChartsUnitsEdit;

  connect(unitsEdit_, SIGNAL(unitsChanged()), this, SLOT(unitsChanged()));

  //---

  QToolButton *addButton    = new QToolButton;
  QToolButton *removeButton = new QToolButton;

  addButton   ->setIcon(CQPixmapCacheInst->getIcon("ADD"));
  removeButton->setIcon(CQPixmapCacheInst->getIcon("REMOVE"));

  connect(addButton, SIGNAL(clicked()), this, SLOT(addSlot()));
  connect(removeButton, SIGNAL(clicked()), this, SLOT(removeSlot()));

  controlFrameLayout->addWidget(new QLabel("Units"));
  controlFrameLayout->addWidget(unitsEdit_);
  controlFrameLayout->addStretch(1);
  controlFrameLayout->addWidget(addButton);
  controlFrameLayout->addWidget(removeButton);

  //---

  scrollArea_ = new QScrollArea;

  pointsFrame_ = new QFrame;

  QVBoxLayout *pointsFrameLayout = new QVBoxLayout(pointsFrame_);
  pointsFrameLayout->setMargin(0); pointsFrameLayout->setSpacing(0);

  scrollArea_->setWidget(pointsFrame_);

  layout->addWidget(scrollArea_);

  //---

  updateState();
}

const CQChartsPolygon &
CQChartsPolygonEdit::
polygon() const
{
  return polygon_;
}

void
CQChartsPolygonEdit::
setPolygon(const CQChartsPolygon &polygon)
{
  polygon_ = polygon;

  polygonToWidgets();

  updateState();
}

void
CQChartsPolygonEdit::
polygonToWidgets()
{
//const QPolygonF     &polygon = polygon_.polygon();
  const CQChartsUnits &units   = polygon_.units();

  unitsEdit_->setUnits(units);

  int n  = polygon_.numPoints();
  int ne = pointEdits_.size();
  assert(n == ne);

  for (int i = 0; i < n; ++i)
    pointEdits_[i]->setValue(polygon_.point(i));
}

void
CQChartsPolygonEdit::
widgetsToPolygon()
{
  int n  = polygon_.numPoints();
  int ne = pointEdits_.size();
  assert(n == ne);

  for (int i = 0; i < n; ++i)
    polygon_.setPoint(i, pointEdits_[i]->getQValue());

  CQChartsUnits units = unitsEdit_->units();

  polygon_.setUnits(units);
}

void
CQChartsPolygonEdit::
unitsChanged()
{
  widgetsToPolygon();

  emit polygonChanged();
}

void
CQChartsPolygonEdit::
addSlot()
{
  polygon_.addPoint(QPointF());

  updatePointEdits();

  widgetsToPolygon();

  updateState();

  emit polygonChanged();
}

void
CQChartsPolygonEdit::
removeSlot()
{
  polygon_.removePoint();

  updatePointEdits();

  widgetsToPolygon();

  updateState();

  emit polygonChanged();
}

void
CQChartsPolygonEdit::
pointSlot()
{
  widgetsToPolygon();

  updateState();

  emit polygonChanged();
}

void
CQChartsPolygonEdit::
updateState()
{
  QFontMetrics fm(font());

  int ew = 32*fm.width("8");
  int eh = fm.height() + 4;

  int w = ew;
  int h = 2*controlFrame_->sizeHint().height();

  int n  = polygon_.numPoints();
  int ch = n*eh;

  int n1  = std::min(n, 10);
  int ch1 = n1*eh;

  int sw = 0;

  if (n1 < n) {
    QStyleOptionSlider opt;

    sw = style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt, this);
  }

  h += ch1;

  int w1 = w + sw;

  //---

  scrollArea_->setFixedSize(QSize(w1, ch1));

  pointsFrame_->setFixedSize(QSize(w, ch));
}

void
CQChartsPolygonEdit::
updatePointEdits()
{
  int n = polygon_.numPoints();

  int ne = pointEdits_.size();

  while (ne < n) {
    CQPoint2DEdit *edit = new CQPoint2DEdit;

    connect(edit, SIGNAL(valueChanged()), this, SLOT(pointSlot()));

    qobject_cast<QVBoxLayout *>(pointsFrame_->layout())->addWidget(edit);

    pointEdits_.push_back(edit);

    ++ne;
  }

  while (ne > n) {
    delete pointEdits_.back();

    pointEdits_.pop_back();

    --ne;
  }
}