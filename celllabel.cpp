#include "celllabel.h"

CellLabel::CellLabel(QWidget *parent): QLabel(parent)
{

}

void CellLabel::paintEvent(QPaintEvent *)
{    
    int w = this->width();
    int h = this->height();
    int off = 4;
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHints(QPainter::Antialiasing);
    if(this->isEnabled())
        painter.setBrush(QBrush(QColor("light gray")));
    else
        painter.setBrush(QBrush(QColor(Qt::gray)));
    painter.drawRect(0,0,w,h);
    painter.setBrush(QBrush(QColor(palette().color(QPalette::Background))));
    painter.drawRect(off,off,w-2*off,h-2*off);
    //painter.setPen(QPen(QColor(Qt::black)));
    QString t = this->fontMetrics().elidedText(text(), Qt::ElideRight, w-off);
    painter.drawText(off+1, h-2*off , t);
}


