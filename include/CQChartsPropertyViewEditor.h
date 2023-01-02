#ifndef CQChartsPropertyViewEditor_H
#define CQChartsPropertyViewEditor_H

class CQChartsPlot;
class CQChartsView;
class CQPropertyViewItem;

namespace CQChartsPropertyViewUtil {

void getPropertyItemPlotView(CQPropertyViewItem *item, CQChartsPlot* &plot, CQChartsView * &view);

}

//---

#include <CQPropertyViewType.h>

class QRect;

/*!
 * \brief property view type
 * \ingroup Charts
 */
class CQChartsPropertyViewType : public CQPropertyViewType {
 public:
  using View = CQChartsView;
  using Plot = CQChartsPlot;

 public:
  CQChartsPropertyViewType() { }

  virtual ~CQChartsPropertyViewType() { }

  bool setEditorData(CQPropertyViewItem *item, const QVariant &value) override;

  void draw(CQPropertyViewItem *item, const CQPropertyViewDelegate *delegate, QPainter *painter,
            const QStyleOptionViewItem &option, const QModelIndex &ind,
            const QVariant &value, const ItemState &itemState) override;

  virtual void drawPreview(QPainter *painter, const QRect &rect, const QVariant &value,
                           Plot *plot, View *view) = 0;
};

//---

#include <CQPropertyViewEditor.h>

class CQChartsLineEditBase;

/*!
 * \brief property view editor factory
 * \ingroup Charts
 */
class CQChartsPropertyViewEditorFactory : public CQPropertyViewEditorFactory {
 public:
  CQChartsPropertyViewEditorFactory() { }

  virtual ~CQChartsPropertyViewEditorFactory() { }

  QWidget *createEdit(QWidget *parent) override;

  virtual CQChartsLineEditBase *createPropertyEdit(QWidget *parent) = 0;
};

#endif
