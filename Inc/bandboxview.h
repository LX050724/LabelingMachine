#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QDebug>

#include <QKeyEvent>
#include <QMouseEvent>

#include <bandbox.h>
#include <imagedata.h>

class BandBoxView : public QGraphicsView
{
    Q_OBJECT
    QColor get_QColor(int index, int a);
public:
    explicit BandBoxView(QWidget *parent = nullptr) : QGraphicsView(parent)
    {   }

signals:

protected:
    friend class BandBox;

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

    ImageData* Data = nullptr;

    QImage Image;
    QGraphicsScene Scene;
    QGraphicsPixmapItem *Pixmap;

    QPointF PressPoint;
    volatile bool drawingflag = false;
    QRect drawing;
    QGraphicsRectItem* drawingItem = nullptr;

    QPointF transPoint;
    QPointF posPoint;

    int ID = 0;
    QString Label;
    QVector<BandBox> BandBoxList;

    int LabelCount = 0;

    float get_color(int c, int x, int max);
public:
    void loadimg(ImageData* Image);

    inline void setLabel(int id, QString label, int count) {
        ID = id;
        Label = label;
        LabelCount = count;
    }

    inline const ImageData getImageData() const {
        return Data == nullptr ? ImageData() : *Data;
    }

signals:
    void drawBandBox(BandBox);
    void deletBox();
    void Keypress(int);
};

#endif // GRAPHICSVIEW_H
