#ifndef CQChartsCmdLine_H
#define CQChartsCmdLine_H

#include <QFrame>

class CQChartsCmdLine;
class CQCharts;

namespace CQCommand {

class ScrollArea;

}

class CQTcl;

#define CQChartsCmdLineDlgMgrInst CQChartsCmdLineDlgMgr::instance()

/*!
 * \brief CmdLine Dialog Manager singleton
 * \ingroup Charts
 */
class CQChartsCmdLineDlgMgr {
 public:
  static CQChartsCmdLineDlgMgr *instance() {
    static CQChartsCmdLineDlgMgr *inst;

    if (! inst)
      inst = new CQChartsCmdLineDlgMgr;

    return inst;
  }

 ~CQChartsCmdLineDlgMgr() { }

  void showDialog(CQCharts *charts);

 private:
  CQChartsCmdLineDlgMgr();

 private:
  CQChartsCmdLine* dlg_ { nullptr };
};

//---

class CQChartsCmdWidget;

class CQChartsCmdLine : public QFrame {
  Q_OBJECT

 public:
  CQChartsCmdLine(CQCharts *charts, QWidget *parent=nullptr);
 ~CQChartsCmdLine();

  CQCharts *charts() const { return charts_; }

  bool complete(CQChartsCmdWidget *widget, const QString &text, int pos,
                QString &newText, bool interactive) const;

  QSize sizeHint() const override;

 private slots:
  void executeCommand(const QString &);

 private:
  CQCharts*              charts_  { nullptr }; //!< charts
  CQCommand::ScrollArea* command_ { nullptr }; //!< command widget
  CQTcl*                 qtcl_    { nullptr }; //!< qtcl interface
};

#endif
