#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <QString>
#include <QVector>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>
#include <QDomNode>

#include "bandbox.h"

class ImageData
{
protected:
    QString ImagePath;
    QString ImageFilename;
    QString XmlPath;
    QString XmlFilename;

    volatile bool remotemod = false;

    QSize size;
    volatile bool has_label = false;
protected:
    QVector<BandBox> BandBoxs;
    QImage *Img = nullptr;

public:
    ImageData()= default;
    ImageData(const QImage &img, const QVector<BandBox> &bandboxs);
    ImageData(const QString& imagepath, const QString& imagefilename, const QString& xmlpath, const QString &xmlfilename);
    bool saveXml();
    bool saveXml(const QString& xmlpath, const QString &xmlfilename);
    const QImage loadImage() const;
    void removeBandBox(const BandBox& Box);

    inline void addBandBox(const BandBox& Box) {
        BandBoxs.push_back(Box);
        has_label = true;
    }

    inline void setfilename(const QString& _filename = QString()) {
        if(_filename.isEmpty())
            ImageFilename = ImagePath.mid(ImagePath.lastIndexOf('/') + 1);
        else
            ImageFilename = _filename;
    }

    inline bool isHasLabel() {
        return has_label = !BandBoxs.isEmpty();
    }

    inline void setBandBoxs(const QVector<BandBox> &bandBoxs) {
        BandBoxs = bandBoxs;
        has_label = !BandBoxs.isEmpty();
    }

    inline void setsize(const QSize& _size) { size = _size; }
    inline const QVector<BandBox> &getBandBoxs() const { return BandBoxs; }
    inline const QString &getImagePath() const { return ImagePath; }
    inline const QString &getImageFilename() const { return ImageFilename; }
    inline const QString &getXmlPath() const { return XmlPath; }
    inline const QString &getXmlFilename() const { return XmlFilename; }
    inline void setRemotemod(bool b) { remotemod = b; }

protected:
    bool loadXml();
    void parsesizeMembers(const QDomElement& Node);
    void parseobjectMembers(const QDomElement& Node);
};

#endif // IMAGEDATA_H
