#ifndef CELLLABEL
#define CELLLABEL
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QItemDelegate>
#include <QBrush>
#include <QHeaderView>

class CellLabel : public QLabel
{
    Q_OBJECT

public:
    CellLabel(QWidget *parent = 0);

    virtual void paintEvent(QPaintEvent *);
};

class TableLabel : public QLabel
{
    Q_OBJECT

public:
    TableLabel(QWidget *parent = 0):QLabel(parent)
    {

    }

    virtual void paintEvent(QPaintEvent *)
    {
        int w = this->width();
        int h = this->height();
        int off = 4;
        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHints(QPainter::Antialiasing);
        QString t = this->fontMetrics().elidedText(text(), Qt::ElideRight, w-off);
        painter.drawText(off+1, h-2*off , t);
        this->setAlignment(Qt::AlignCenter);
        //QPainterPath path;
//        QFont drawFont = QFont(font());
//        painter.addText(off, drawFont.pointSize() + off, drawFont, text());
        //painter.strokePath(path, QPen(QColor((palette().color(QPalette::Foreground), 2))));
        //painter.fillPath(path, QBrush(QColor(palette().color(QPalette::Background))));
    }
};

class TempCell : public QLabel
{
    Q_OBJECT

  public:
    TempCell(QWidget *parent = 0):QLabel(parent)
    {
    }
    virtual void paintEvent(QPaintEvent*)
    {
        QPainter painter(this);
        painter.drawText(QPoint(rect().center().x() - 10, rect().center().y()), text());
        if(!this->isEnabled())
        {
            painter.setPen(QPen(QBrush(Qt::red), 5));
            painter.drawLine(rect().topLeft(), rect().bottomRight());
        }
    }
};

class MyDelegate : public QItemDelegate
{
  public:
    MyDelegate( QObject *parent ) : QItemDelegate( parent )
    {
    }
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
      QItemDelegate::paint(painter, option, index);
      painter->setPen(QPen(QColor("light gray"), 3));
      painter->drawRect(option.rect);
    }
};

#endif // CELLLABEL

