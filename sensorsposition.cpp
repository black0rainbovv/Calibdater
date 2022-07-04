#include "sensorsposition.h"
#include "mainwindow.h"

#include <QMessageBox>
#include "math.h"

QList<QColor> scolor;

SensorsPosition::SensorsPosition(QWidget *parent):
    QWidget(parent)
{
    n = 1;
    lefttracked = false;
    righttracked = false;
    leftpressed = false;
    rightpressed = false;
    setMouseTracking(true);  

    scolor << QColor(Qt::white);
    scolor << QColor(Qt::red);
    scolor << QColor(Qt::green);
    scolor << QColor(Qt::blue);
    scolor << QColor(Qt::cyan);
    scolor << QColor(Qt::magenta);
    scolor << QColor(Qt::yellow);
}

SensorsPosition::~SensorsPosition()
{

}


void SensorsPosition::paintEvent(QPaintEvent *)
{    
    int w = this->width();
    int h = this->height();
    double rx = w/6-2;
    double ry = h/6-2;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QLinearGradient gr(0, 0, 0, h/3);
    gr.setColorAt(0.0, Qt::white);
    gr.setColorAt(1.0, Qt::gray);

    QLinearGradient gr1(0, h/3, 0, 2*h/3);
    gr1.setColorAt(0.0, Qt::white);
    gr1.setColorAt(1.0, Qt::gray);

    QLinearGradient grtracked(0, 0, 0, 1);
    grtracked.setColorAt(0.0, Qt::white);
    grtracked.setColorAt(1.0, Qt::lightGray);

    QLinearGradient grpressed(0, 0, 0, 1);    
    grpressed.setColorAt(0.0, Qt::darkGray);
    grpressed.setColorAt(1.0, Qt::darkGray);

    QFont font = painter.font();
    font.setPointSize(7*w/105);
    painter.setFont(font);

    //painter.setBrush(QBrush(QColor(255, 0, 0)));
     painter.setBrush(gr);
    painter.drawEllipse(QPointF(w/6, h/6), rx, ry);
    painter.drawEllipse(QPointF(3*w/6, h/6), rx, ry);
    painter.drawEllipse(QPointF(5*w/6, h/6), rx, ry);
     painter.setBrush(gr1);
    painter.drawEllipse(QPointF(w/6, 3*h/6+5), rx, ry);
    painter.drawEllipse(QPointF(3*w/6, 3*h/6+5), rx, ry);
    painter.drawEllipse(QPointF(5*w/6, 3*h/6+5), rx, ry);
    if(lefttracked)
    {
        if(leftpressed)
            painter.setBrush(grpressed);
        else
            painter.setBrush(grtracked);
    }
    else
        painter.setBrush(gr);
    painter.drawChord(w/2-w/12-5, 4*h/6+10, w/6, h/5, 90*16, 180*16);
    if(righttracked)
    {
        if(rightpressed)
            painter.setBrush(grpressed);
        else
            painter.setBrush(grtracked);
    }
    else
        painter.setBrush(gr);
    painter.drawChord(w/2-w/12+5, 4*h/6+10, w/6, h/5, -90*16, 180*16);

    if(SensorsIsEnabled[(7-n)%6] == 1)
        painter.setBrush(QBrush(scolor[(7-n)%6+1]));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(QPointF(w/6, h/6), rx/2, ry/2);
    if(SensorsIsEnabled[(8-n)%6] == 1)
        painter.setBrush(QBrush(scolor[(8-n)%6+1]));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(QPointF(3*w/6, h/6), rx/2, ry/2);
    if(SensorsIsEnabled[(9-n)%6] == 1)
        painter.setBrush(QBrush(scolor[(9-n)%6+1]));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(QPointF(5*w/6, h/6), rx/2, ry/2);
    if(SensorsIsEnabled[(10-n)%6] == 1)
        painter.setBrush(QBrush(scolor[(10-n)%6+1]));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(QPointF(w/6, 3*h/6+5), rx/2, ry/2);
    if(SensorsIsEnabled[(11-n)%6] == 1)
        painter.setBrush(QBrush(scolor[(11-n)%6+1]));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(QPointF(3*w/6, 3*h/6+5), rx/2, ry/2);
    if(SensorsIsEnabled[(12-n)%6] == 1)
        painter.setBrush(QBrush(scolor[(12-n)%6+1]));
    else
        painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(QPointF(5*w/6, 3*h/6+5), rx/2, ry/2);

    QPointF pos1((w/6)-rx/(4*sqrt(2)), (h/6)+rx/(3*sqrt(2)));
    QPointF pos2((3*w/6)-rx/(4*sqrt(2)), (h/6)+rx/(3*sqrt(2)));
    QPointF pos3((5*w/6)-rx/(4*sqrt(2)), (h/6)+rx/(3*sqrt(2)));
    QPointF pos4((w/6)-rx/(4*sqrt(2)), (3*h/6)+rx/(3*sqrt(2))+5);
    QPointF pos5(3*w/6-rx/(4*sqrt(2)), 3*h/6+rx/(3*sqrt(2))+5);
    QPointF pos6((5*w/6)-rx/(4*sqrt(2)), (3*h/6)+rx/(3*sqrt(2))+5);

    QPainterPath path;
    path.addText(pos1, font, QString::number((7-n)%6+1));
    path.addText(pos2, font, QString::number((8-n)%6+1));
    path.addText(pos3, font, QString::number((9-n)%6+1));
    path.addText(pos4, font, QString::number((10-n)%6+1));
    path.addText(pos5, font, QString::number((11-n)%6+1));
    path.addText(pos6, font, QString::number((12-n)%6+1));

    //painter.translate((w/6)-rx/(4*sqrt(2)), -1);
    painter.setPen(QPen(Qt::black, 3));
    painter.drawPath(path);
    painter.fillPath(path, Qt::white);
}

void SensorsPosition::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        int w = this->width();
        int h = this->height();
        QPointF pos = e->pos();

        if(pos.x() > (w/2-w/12-5) && pos.x() < (w/2-w/12-5+w/6-w/12))
        {
            if(pos.y() >= 4*h/6+5 && pos.y() <= 4*h/6+10+h/6)
            {
                leftpressed = true;
                n = (-7+n)%6+6;
                emit positionChanged(n);
                this->repaint();
            }
        }

        if(pos.x() > (w/2-w/12+5+w/12) && pos.x() < (w/2-w/12+5+w/6))
        {
            if(pos.y() >= 4*h/6+5 && pos.y() <= 4*h/6+10+h/6)
            {
                rightpressed = true;
                n = (6+n)%6+1;
                emit positionChanged(n);
                this->repaint();
            }
        }
    }
}

void SensorsPosition::mouseReleaseEvent(QMouseEvent*)
{
    leftpressed = false;
    rightpressed = false;
    this->repaint();
}

void SensorsPosition::mouseMoveEvent(QMouseEvent *e)
{
    int w = this->width();
    int h = this->height();
    QPointF pos = e->pos();
    if(pos.x() > (w/2-w/12-5) && pos.x() < (w/2-w/12-5+w/6-w/12))
    {
        if(pos.y() >= 4*h/6+5 && pos.y() <= 4*h/6+10+h/6)
        {
            lefttracked = true;
            this->repaint();
        }
        else
            lefttracked = false;
    }
    else
        lefttracked = false;
    if(pos.x() > (w/2-w/12+5+w/12) && pos.x() < (w/2-w/12+5+w/6))
    {
        if(pos.y() >= 4*h/6+5 && pos.y() <= 4*h/6+10+h/6)
        {
            righttracked = true;
            this->repaint();
        }
        else
            righttracked = false;
    }
    else
        righttracked = false;
    this->repaint();
}











