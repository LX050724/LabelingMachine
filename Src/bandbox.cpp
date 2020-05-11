#include "bandbox.h"

#include <utility>

BandBox::BandBox(int xmin, int ymin, int xmax, int yamx, const QString &label, int id) :
        Rect(QPoint(xmin, ymin), QPoint(xmax, yamx)), Label(label) {
    ID = id;
}

BandBox::BandBox(QGraphicsRectItem *item, int id, QString label) : Label(std::move(label)) {
    QRect tmp(item->rect().toRect());
    Rect = QRect(item->pos().toPoint(), item->pos().toPoint() + QPoint(tmp.width(), tmp.height())),
            Item = item;
    ID = id;
}
