#include "imagedata.h"

ImageData::ImageData(const QString& imagepath, const QString& imagefilename, const QString& xmlpath, const QString &xmlfilename):
    ImagePath(imagepath), ImageFilename(imagefilename), XmlPath(xmlpath), XmlFilename(xmlfilename)
{
    loadXml();
}

ImageData::ImageData(const QImage &img, const QVector<BandBox> &bandboxs) :
    BandBoxs(bandboxs),
    Img(new QImage(img))
{
    has_label = !BandBoxs.isEmpty();
}

bool ImageData::saveXml(const QString& xmlpath, const QString &xmlfilename) {
    if(xmlpath.isEmpty() || xmlfilename.isEmpty())
        return false;
    XmlPath = xmlpath;
    XmlFilename = xmlfilename;
    return saveXml();
}

bool ImageData::saveXml() {
    if(remotemod)
        return true;

    if(XmlPath.isEmpty())
        return false;

    QFile File(XmlPath + '/' + XmlFilename);
    if(!File.open(QFile::WriteOnly | QFile::Text))
        return false;

    QTextStream textStream(&File);
    QDomDocument XmlReader;

    QDomElement root = XmlReader.createElement("annotation");

    QDomElement filenameElement = XmlReader.createElement("filename");
    filenameElement.appendChild(XmlReader.createTextNode(ImageFilename));
    root.appendChild(filenameElement);

    //sizeElement
    QDomElement sizeElement = XmlReader.createElement("size");

    QDomElement widthElement = XmlReader.createElement("width");

    widthElement.appendChild(XmlReader.createTextNode(QString::number(size.width())));
    sizeElement.appendChild(widthElement);

    QDomElement heightElement = XmlReader.createElement("height");
    heightElement.appendChild(XmlReader.createTextNode(QString::number(size.height())));
    sizeElement.appendChild(heightElement);

    root.appendChild(sizeElement);
    //sizeElement End

    for(const BandBox& object : BandBoxs) {
        QDomElement objectElement = XmlReader.createElement("object");

        QDomElement nameElement = XmlReader.createElement("name");
        nameElement.appendChild(XmlReader.createTextNode(object.Label));
        objectElement.appendChild(nameElement);

        QDomElement IDElement = XmlReader.createElement("ID");
        IDElement.appendChild(XmlReader.createTextNode(QString::number(object.ID)));
        objectElement.appendChild(IDElement);

        QDomElement bndboxElement = XmlReader.createElement("bndbox");

        int xmin = object.Rect.x();
        int ymin = object.Rect.y();
        int xmax = object.Rect.x() + object.Rect.width();
        int ymax = object.Rect.y() + object.Rect.height();

        QDomElement xminElement = XmlReader.createElement("xmin");
        xminElement.appendChild(XmlReader.createTextNode(QString::number(xmin)));
        bndboxElement.appendChild(xminElement);

        QDomElement yminElement = XmlReader.createElement("ymin");
        yminElement.appendChild(XmlReader.createTextNode(QString::number(ymin)));
        bndboxElement.appendChild(yminElement);

        QDomElement xmaxElement = XmlReader.createElement("xmax");
        xmaxElement.appendChild(XmlReader.createTextNode(QString::number(xmax)));
        bndboxElement.appendChild(xmaxElement);

        QDomElement ymaxElement = XmlReader.createElement("ymax");
        ymaxElement.appendChild(XmlReader.createTextNode(QString::number(ymax)));
        bndboxElement.appendChild(ymaxElement);

        objectElement.appendChild(bndboxElement);

        root.appendChild(objectElement);
    }

    root.save(textStream, 4);
    File.close();
    return true;
}

bool ImageData::loadXml() {
    if(XmlPath.isEmpty() || XmlFilename.isEmpty())
        return false;

    QFile File(XmlPath + '/' + XmlFilename);
    if(!File.open(QFile::ReadOnly | QFile::Text))
        return false;

    QDomDocument XmlReader;
    if(!XmlReader.setContent(&File)) {
        File.close();
        return false;
    }

    QDomElement root = XmlReader.documentElement();
    if(root.nodeName() == "annotation") {
        ImageFilename = root.firstChildElement("filename").text();
        parsesizeMembers(root.firstChildElement("size"));
        QDomElement object = root.firstChildElement("object");
        while (!object.isNull()) {
            parseobjectMembers(object);
            object = object.nextSiblingElement();
        }
    }
    else {
        File.close();
        return false;
    }

    File.close();

    has_label = !BandBoxs.isEmpty();
    return true;
}

inline void ImageData::parsesizeMembers(const QDomElement& Node) {
    size.setWidth(Node.firstChildElement("width").text().toInt());
    size.setHeight(Node.firstChildElement("height").text().toInt());
}

void ImageData::parseobjectMembers(const QDomElement& Node) {
    int xmin,ymin,xmax,ymax;

    QString name = Node.firstChildElement("name").text();
    int ID = Node.firstChildElement("ID").text().toInt();
    QDomElement bndbox = Node.firstChildElement("bndbox");
    xmin = bndbox.firstChildElement("xmin").text().toInt();
    ymin = bndbox.firstChildElement("ymin").text().toInt();
    xmax = bndbox.firstChildElement("xmax").text().toInt();
    ymax = bndbox.firstChildElement("ymax").text().toInt();

    BandBoxs.push_back(BandBox(xmin, ymin, xmax, ymax, name, ID));
}

const QImage ImageData::loadImage() const {
    if(remotemod)
        return *Img;

    QImage tmp;
    tmp.load(ImagePath + '/' + ImageFilename);
    return tmp;
}

void ImageData::removeBandBox(const BandBox &Box) {
    int index = BandBoxs.indexOf(Box);
    if(index >= 0)
        BandBoxs.erase(BandBoxs.begin() + index);
    has_label = (BandBoxs.size() != 0);
}
