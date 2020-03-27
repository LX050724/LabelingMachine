#ifndef BANDBOX_H
#define BANDBOX_H

#include <QGraphicsItem>
#include <QString>
#include <QtDebug>
#include "Labels.h"

class BandBox {
    QGraphicsRectItem* Item = nullptr;
public:
    QRect Rect;
    QString Label;
    int ID;

    BandBox()= default;
    BandBox(int xmin, int ymin, int xmax, int yamx, const QString &label, int id);
    BandBox(const QRect& rect, const QString &label, int _ID) :
            Rect(rect), Label(label), ID(_ID) {}

    explicit BandBox(QGraphicsRectItem* item, int id = 0, QString  label = QString());

    inline bool operator ==(const BandBox &comp) const {
        return (this->Rect == comp.Rect);
    }
};

#endif // BANDBOX_H
