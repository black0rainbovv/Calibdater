#ifndef GRAPHBACKGROUNDCOLOR
#define GRAPHBACKGROUNDCOLOR

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QList>


class GraphBackgroundColor : public QWidget
{
    Q_OBJECT

public:
    GraphBackgroundColor(QWidget *parent = 0);
    ~GraphBackgroundColor();

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *e);    
    //void mouseReleaseEvent(QMouseEvent *e);

public slots:
    void SetCurrentColor(QString c);

private:
    QList<QColor> background;
    int currentcolor;

signals:
   void colorChanged(QColor c);

};

#endif // GRAPHBACKGROUNDCOLOR

