#include <CQChartsStyle.h>
#include <CQChartsUtil.h>
#include <CQStrParse.h>

namespace {

static int s_typeId = -1;

}

CQUTIL_DEF_META_TYPE(CQChartsStyle, toString, fromString)

void
CQChartsStyle::
registerMetaType()
{
  s_typeId = CQUTIL_REGISTER_META(CQChartsStyle);
}

int
CQChartsStyle::
metaType()
{
  return s_typeId;
}

QString
CQChartsStyle::
toString() const
{
  if (! pen_)
    return "";

  QString str;

  str += QString("stroke: %1;"      ).arg(pen_  ->color().name());
  str += " ";
  str += QString("stroke-width: %1;").arg(pen_  ->widthF());
  str += " ";
  str += QString("fill: %1;"        ).arg(brush_->color().name());
  str += " ";
  str += QString("fill-opacity: %1;").arg(brush_->color().alphaF());

  return str;
}

void
CQChartsStyle::
fromString(const QString &str)
{
  delete pen_;
  delete brush_;

  pen_   = new QPen  (Qt::NoPen);
  brush_ = new QBrush(Qt::NoBrush);

  //---

  CQStrParse parse(str);

  while (! parse.eof()) {
    parse.skipSpace();

    QString name;

    while (! parse.eof()) {
      if (parse.isSpace() || parse.isChar(':'))
        break;

      name += parse.getChar();
    }

    parse.skipSpace();

    if (parse.isChar(':')) {
      parse.skipChar();

      parse.skipSpace();
    }

    QString value;

    while (! parse.eof()) {
      if (parse.isSpace() || parse.isChar(';'))
        break;

      value += parse.getChar();
    }

    parse.skipSpace();

    if (parse.isChar(';')) {
      parse.skipChar();

      parse.skipSpace();
    }

    if      (name == "stroke") {
      QColor c(value);

      pen_->setColor(c);
    }
    else if (name == "stroke-width") {
      bool ok;
      double w = CQChartsUtil::toReal(value, ok);
      if (! ok) continue;

      pen_->setWidthF(w);
    }
    else if (name == "fill") {
      QColor c(value);

      brush_->setColor(c);
      brush_->setStyle(Qt::SolidPattern);
    }
    else if (name == "fill-opacity") {
      bool ok;
      double a = CQChartsUtil::toReal(value, ok);
      if (! ok) continue;

      QColor c = brush_->color();
      c.setAlphaF(a);
      brush_->setColor(c);
    }
    else {
      std::cerr << name.toStdString() << "=" << value.toStdString() << "\n";
    }
  }

  return;
}