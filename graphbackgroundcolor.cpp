#include "GraphBackgroundColor.h"

GraphBackgroundColor::GraphBackgroundColor(QWidget *parent):
    QWidget(parent)
{
    background << QColor("#ffffff") << QColor("#d3d3d3") << QColor("#a0a0a4") <<
                  QColor("#808080") << QColor("#32cd32") << QColor("#008000") <<
                  QColor("#556B2F");
    currentcolor = -2;
}

GraphBackgroundColor::~GraphBackgroundColor()
{

}

void GraphBackgroundColor::paintEvent(QPaintEvent *)
{
    int w = this->width();
    int h = this->height();
    while(w%7!=0)
        w-=1;
    QPainter painter(this);

    painter.setPen(QPen(QColor(Qt::black), 1));
    for(int i=0; i<background.size(); i++)
    {
        painter.setBrush(QBrush(background[i]));
        painter.drawRect(i*w/background.size(),0,w/background.size(), h-1);
    }

    QPainter r(this);
    int penwidth = 3;
    r.setPen(QPen(QColor(Qt::red), penwidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    r.drawRect(currentcolor*w/background.size()+penwidth-2, penwidth-2,
               w/background.size()-2*(penwidth-2), h-2*(penwidth-2)-1);
}

void GraphBackgroundColor::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        int w = this->width();
        int h = this->height();
        QPointF pos = e->pos();
        for(int i=0; i<background.size(); i++)
        {
            if((pos.x() > i*w/background.size() && pos.x() < (i+1)*w/background.size()) && (pos.y() > 0 && pos.y() < h-1))
            {
                emit colorChanged(background[i]);
                currentcolor = i;
            }
        }
        this->repaint();
    }
}

void GraphBackgroundColor::SetCurrentColor(QString c)
{
    for(int i=0; i<background.size(); i++)
    {
        if(background[i].name() == c)
        {
            currentcolor = i;
            this->repaint();
        }
    }
}





