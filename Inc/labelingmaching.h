#ifndef LEABLINGMACHING_H
#define LEABLINGMACHING_H

#include <QString>
#include <QVector>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>

#include <Labels.h>
#include <imagedata.h>

class LabelingMaching
{    
protected:    
    QString ProjectPath;

    int ImgCount = 0;
    QString ImgPath;
    QString XmlPath;

public:
    Labels labels;
    QVector<ImageData> Images;

    LabelingMaching()= default;
    LabelingMaching(const QString& _Path): ProjectPath(_Path){}

    void setProjectPath(const QString& _Path);

    bool load();
    bool save();

    inline void setImgCount(int _ImgCount) { ImgCount = _ImgCount; }
    inline void setImgPath(const QString& _ImgPath) { ImgPath = _ImgPath; }
    inline void setXmlPath(const QString& _XmlPath) { XmlPath = _XmlPath; }
    inline const QString& getXmlPath() { return XmlPath; }
    inline const QString& getImgPath() const { return ImgPath; }
    inline const QString& getProjectPath() const { return ProjectPath; }

    inline bool isOpened() const {
        return (!ImgPath.isEmpty() && !XmlPath.isEmpty());
    }

    inline bool isRemote() const {
        return (ImgPath == "remote" && XmlPath== "remote");
    }

    void addImage(const ImageData& Image);
    void deleteImage(int Index);
    int findImage(const QString& filename);

    QVector<ImageData> has_label_Img();
    const QVector<ImageData>& all_Img() const;
    QVector<ImageData> no_label_Img();

private:
    void parseProjectMembers(const QDomElement& Node);
    void parseLabelsMembers(const QDomElement& Node);
    void parseImageMembers(const QDomElement& Node);
};

#endif // LEABLINGMACHING_H
