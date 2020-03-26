#include "bandboxview.h"
#include <QGraphicsTextItem>
#include <QRect>
#include <QPen>
#include <math.h>

void BandBoxView::mouseMoveEvent(QMouseEvent *event) {
    QGraphicsView::mouseMoveEvent(event);

    QGraphicsScene *scene_ = scene();
    if(scene_ == nullptr)
        return;

    if(drawingflag && event->buttons().testFlag(Qt::LeftButton)) {
        QPointF mousePoint = Pixmap->mapFromScene(mapToScene(event->pos()));

        if(mousePoint.x() < 0)mousePoint.setX(0);
        if(mousePoint.y() < 0)mousePoint.setY(0);
        if(mousePoint.x() > Image.size().width() - 1)mousePoint.setX(Image.size().width() - 1);
        if(mousePoint.y() > Image.size().height() - 1)mousePoint.setY(Image.size().height() - 1);

        QPoint Point1(std::min(mousePoint.x(), PressPoint.x()),
                      std::min(mousePoint.y(), PressPoint.y()));

        QPoint Point2(std::max(mousePoint.x(), PressPoint.x()),
                      std::max(mousePoint.y(), PressPoint.y()));

        drawing = QRect(QPoint(0, 0), Point2 - Point1);

        if(drawingItem != nullptr) {
            drawingItem->setPos(Point1);
            drawingItem->setRect(drawing);
        }
        else {
            drawingItem = scene_->addRect(drawing);
            drawingItem->setParentItem(Pixmap);
            QPen Pen(QColor(0,255,0,128));
            //Pen.setWidth(3);
            drawingItem->setPen(Pen);
        }
    }
    else if(event->buttons().testFlag(Qt::MiddleButton)) {
        QPointF mousePoint = mapToScene(event->pos());
        Pixmap->setPos(mousePoint - transPoint + posPoint);
    }

}

void BandBoxView::mousePressEvent(QMouseEvent *event) {
    QGraphicsView::mousePressEvent(event);

    QGraphicsScene *scene_ = scene();
    if(scene_ == nullptr)
        return;

    QPointF mousePoint = Pixmap->mapFromScene(mapToScene(event->pos()));

    if(event->button() == Qt::LeftButton) {
        drawingflag = true;

        if(mousePoint.x() < 0)mousePoint.setX(0);
        if(mousePoint.y() < 0)mousePoint.setY(0);
        if(mousePoint.x() > Image.size().width() - 1)mousePoint.setX(Image.size().width() - 1);
        if(mousePoint.y() > Image.size().height() - 1)mousePoint.setY(Image.size().height() - 1);
        PressPoint = mousePoint;
    }
    else if(event->buttons().testFlag(Qt::MiddleButton)) {
        transPoint = mapToScene(event->pos());
        posPoint = Pixmap->pos();
    }
}

void BandBoxView::mouseReleaseEvent(QMouseEvent *event) {
    QGraphicsScene *scene_ = scene();
    if(scene_ == nullptr)
        return;

    QPointF mousePoint = Pixmap->mapFromScene(mapToScene(event->pos()));

    QGraphicsView::mouseReleaseEvent(event);

    if(mousePoint.x() < 0)mousePoint.setX(0);
    if(mousePoint.y() < 0)mousePoint.setY(0);
    if(mousePoint.x() > Image.size().width() - 1)mousePoint.setX(Image.size().width() - 1);
    if(mousePoint.y() > Image.size().height() - 1)mousePoint.setY(Image.size().height() - 1);

    if(event->button() == Qt::LeftButton) {
        if(drawingItem != nullptr) {
            QGraphicsTextItem * text = scene_->addText(Label + " ID=" + QString::number(ID));
            text->setDefaultTextColor(get_QColor(ID, 128));
            text->setParentItem(drawingItem);
            text->setPos(0,-22);

            drawingflag = false;
            QPoint Point1(std::min(mousePoint.x(), PressPoint.x()),
                          std::min(mousePoint.y(), PressPoint.y()));

            QPoint Point2(std::max(mousePoint.x(), PressPoint.x()),
                          std::max(mousePoint.y(), PressPoint.y()));


            drawing = QRect(QPoint(0, 0), Point2 - Point1);
            if(drawing.height() * drawing.width() < 100) {
                scene_->removeItem(drawingItem);
                drawingItem = nullptr;
                return;
            }
            drawingItem->setPos(Point1);
            drawingItem->setRect(drawing);
            drawingItem->setPen(QPen(get_QColor(ID, 128)));

            BandBox Box(drawingItem, ID, Label);

            Data->addBandBox(Box);
            Data->saveXml();
            emit drawBandBox(Box);

            drawingItem = nullptr;
        } else {
            drawingflag = false;
        }
    } else if(event->button() == Qt::RightButton) {
        if(QGraphicsItem* Item = itemAt(event->pos())) {
            if(Item->type() == QGraphicsRectItem::Type) {
                Data->removeBandBox(BandBox((QGraphicsRectItem* )Item));
                Data->saveXml();
                scene_->removeItem(Item);
                emit deletBox();
            }
        }
    }
}

void BandBoxView::keyPressEvent(QKeyEvent *event) {
    QGraphicsView::keyPressEvent(event);

    QGraphicsScene *scene_ = scene();
    if(scene_ == nullptr)
        return;

    QList<QGraphicsItem *> Items = scene_->items();

    switch (event->key()) {
    case Qt::Key_I:
        qDebug() << Items;
        break;
    case Qt::Key_R:
        if(Items.size() < 2)
            return;
        Items.pop_front();
        Data->removeBandBox(BandBox((QGraphicsRectItem* )Items.front()));
        Data->saveXml();
        scene_->removeItem(Items.front());
        Items.pop_front();
        emit deletBox();
        break;
    }
    emit Keypress(event->key());
}

void BandBoxView::wheelEvent(QWheelEvent *event) {
    QGraphicsScene *scene_ = scene();
    if(scene_ == nullptr)
        return;

    QPoint mousePoint = mapToScene(event->pos()).toPoint();
    double scale = Pixmap->scale();
    if(event->delta() > 0) {
        if(scale > 4.9)
            return;
        scale *= 1.1;
        Pixmap->setPos((Pixmap->pos() - mousePoint) * 1.1 + mousePoint);
        Pixmap->setScale(scale);
    }
    else {
        if(scale < 0.6)
            return;
        scale *= 0.9;
        Pixmap->setPos((Pixmap->pos() - mousePoint) * 0.9 + mousePoint);
        Pixmap->setScale(scale);
    }
}

void BandBoxView::loadimg(ImageData* _Imagedata) {
    drawingflag = false;
    Data = _Imagedata;
    Image = Data->loadImage();
    if(scene() == nullptr) {
        Pixmap = Scene.addPixmap(QPixmap::fromImage(Image));
        Scene.setItemIndexMethod(QGraphicsScene::BspTreeIndex);
        setScene(&Scene);
        show();
    } else {
        Scene.clear();
        Pixmap = Scene.addPixmap(QPixmap::fromImage(Image));
        Scene.setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    }

    QVector<BandBox> bandBoxs = Data->getBandBoxs();

    std::sort(bandBoxs.begin(), bandBoxs.end(), [&](const BandBox& a, const BandBox& b){
       return (a.Rect.height() * a.Rect.width()) >
              (b.Rect.height() * b.Rect.width());
    });

    for(const BandBox& i : bandBoxs) {
        QRect rect(i.Rect);
        rect.setHeight(rect.height() - 1);
        rect.setWidth(rect.width() - 1);
        rect.moveTo(0, 0);

        QGraphicsRectItem* item = Scene.addRect(rect);
        item->setParentItem(Pixmap);
        item->setPen(QPen(get_QColor(i.ID, 128)));
        item->setPos(QPoint(i.Rect.x(), i.Rect.y()));

        QGraphicsTextItem * text = Scene.addText(i.Label + " ID=" + QString::number(i.ID));
        text->setDefaultTextColor(get_QColor(i.ID, 128));
        text->setParentItem(item);
        text->setPos(0,-22);
    }

    Data->setsize(Image.size());
    Data->saveXml();
}

inline float BandBoxView::get_color(int c, int x, int max) {
    float colors[6][3] = { {1,0,1}, {0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };
    float ratio = ((float)x / max) * 5;
    int i = floor(ratio);
    int j = ceil(ratio);
    ratio -= i;
    float r = (1-ratio) * colors[i][c] + ratio*colors[j][c];
    return r;
}

inline QColor BandBoxView::get_QColor(int index, int a) {
    if(LabelCount == 0)
        LabelCount = 1;

    int offset = index * 123457 % LabelCount;
    int red = get_color(0, offset, LabelCount) * 255;
    int green = get_color(1, offset, LabelCount) * 255;
    int blue = get_color(2, offset, LabelCount) * 255;
    return QColor(red, green, blue, a);
}
