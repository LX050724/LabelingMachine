#ifndef BANDBOX_H
#define BANDBOX_H

#include <QGraphicsItem>
#include <QString>
#include <QJsonObject>
#include <QtDebug>
#include <QJsonArray>
#include "Labels.h"

class BandBox {
public:
    QRect Rect;
    QString Label;
    int ID;

    BandBox() = default;

    BandBox(int xmin, int ymin, int xmax, int yamx, const QString &label, int id);

    BandBox(const QRect &rect, const QString &label, int _ID) :
            Rect(rect), Label(label), ID(_ID) {}

    BandBox(const QJsonObject& jsonObject) {
        Label = jsonObject.find("Lable")->toString();
        ID = jsonObject.find("ID")->toInt();
        auto array = jsonObject.find("Rect")->toArray();
        Rect.setX(array[0].toInt());
        Rect.setY(array[1].toInt());
        Rect.setHeight(array[2].toInt());
        Rect.setWidth(array[3].toInt());
    }

    explicit BandBox(QGraphicsRectItem *item, int id = 0, QString label = QString());

    inline bool operator==(const BandBox &comp) const {
        return (this->Rect == comp.Rect);
    }

    operator QJsonObject () const {
        return {
                {"ID",    ID},
                {"Lable", Label},
                {"Rect", {
                    {Rect.x(), Rect.y(), Rect.height(), Rect.width()}
                }},
        };
    }
};

#endif // BANDBOX_H
