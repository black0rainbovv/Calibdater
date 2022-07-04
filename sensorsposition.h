#ifndef SENSORSPOSITION
#define SENSORSPOSITION

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QList>
#include "global.h"


class SensorsPosition : public QWidget
{
    Q_OBJECT

public:
    SensorsPosition(QWidget *parent = 0);
    ~SensorsPosition();

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private:
    int n;
    bool lefttracked;
    bool righttracked;
    bool leftpressed;
    bool rightpressed;
    int enables[6];


signals:
   void positionChanged(int n);

};

#endif // SENSORSPOSITION

