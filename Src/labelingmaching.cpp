#include "labelingmaching.h"

void LabelingMaching::addImage(const ImageData &Image) {
    Images.push_back(Image);
}

void LabelingMaching::deleteImage(int Index) {
    Images.erase(Images.begin() + Index);
}

int LabelingMaching::findImage(const QString &filename) {
    for (int i = 0; i < Images.size(); ++i) {
        if (Images[i].getImageFilename() == filename)
            return i;
    }
    return -1;
}

QVector<ImageData> LabelingMaching::has_label_Img() {
    QVector<ImageData> tmp;
    for (auto &img : Images) {
        if (img.isHasLabel())
            tmp.push_back(img);
    }
    return tmp;
}

const QVector<ImageData> &LabelingMaching::all_Img() const {
    return Images;
}

QVector<ImageData> LabelingMaching::no_label_Img() {
    QVector<ImageData> tmp;
    for (ImageData &img : Images) {
        if (!img.isHasLabel())
            tmp.push_back(img);
    }
    return tmp;
}


void LabelingMaching::setProjectPath(const QString &_Path) {
    ProjectPath = _Path;
}

bool LabelingMaching::load() {
    if (ProjectPath.isEmpty())
        return false;

    QFile File(ProjectPath);
    if (!File.open(QFile::ReadOnly | QFile::Text))
        return false;

    QDomDocument XmlReader;
    if (!XmlReader.setContent(&File))
        return false;

    QDomElement root = XmlReader.documentElement();

    if (root.nodeName() == "LablingMachineProject") {
        parseProjectMembers(root.firstChildElement("Project"));
        parseLabelsMembers(root.firstChildElement("Labels"));
        QDomElement Image = root.firstChildElement("Image");
        while (!Image.isNull()) {
            parseImageMembers(Image);
            Image = Image.nextSiblingElement();
        }
    } else {
        File.close();
        return false;
    }

    File.close();
    return true;
}

void LabelingMaching::parseProjectMembers(const QDomElement &Node) {
    ImgCount = Node.firstChildElement("ImgCount").text().toInt();
    ImgPath = Node.firstChildElement("ImgPath").text();
    XmlPath = Node.firstChildElement("XmlPath").text();
}

void LabelingMaching::parseLabelsMembers(const QDomElement &Node) {
    QDomNodeList childs = Node.childNodes();
    for (int i = 0; i < childs.length(); ++i) {
        QDomNode child = childs.at(i);
        QString nodeName = child.nodeName();
        if (nodeName == "label") {
            int ID = child.firstChildElement("ID").text().toInt();
            QString name = child.firstChildElement("Name").text();
            labels.addLabel(name, ID);
        }
    }
}

void LabelingMaching::parseImageMembers(const QDomElement &Node) {
    QString imagename = Node.firstChildElement("ImageFilename").text();
    QString xmlname = Node.firstChildElement("XmlFilename").text();

    Images.push_back(ImageData(ImgPath, imagename, XmlPath, xmlname));
}

bool LabelingMaching::save() {
    if (ProjectPath.isEmpty())
        return false;

    if (ImgPath == "remote" && XmlPath == "remote")
        return true;

    QFile File(ProjectPath);
    if (!File.open(QFile::WriteOnly | QFile::Text))
        return false;

    QTextStream textStream(&File);
    QDomDocument XmlReader;

    QDomElement root = XmlReader.createElement("LablingMachineProject");

    //ProjectElement
    QDomElement ProjectElement = XmlReader.createElement("Project");

    QDomElement ImgCountElement = XmlReader.createElement("ImgCount");
    ImgCountElement.appendChild(XmlReader.createTextNode(QString::number(ImgCount)));
    ProjectElement.appendChild(ImgCountElement);

    QDomElement ImgPathElement = XmlReader.createElement("ImgPath");
    ImgPathElement.appendChild(XmlReader.createTextNode(ImgPath));
    ProjectElement.appendChild(ImgPathElement);

    QDomElement XmlPathElement = XmlReader.createElement("XmlPath");
    XmlPathElement.appendChild(XmlReader.createTextNode(XmlPath));
    ProjectElement.appendChild(XmlPathElement);

    root.appendChild(ProjectElement);
    //ProjectElement End

    //LabelsElement
    QDomElement LabelsElement = XmlReader.createElement("Labels");

    QVector<QString> labelVec = labels.getLabels();
    for (int i = 0; i < labelVec.size(); ++i) {
        QDomElement labelElement = XmlReader.createElement("label");

        QDomElement IDElement = XmlReader.createElement("ID");
        IDElement.appendChild(XmlReader.createTextNode(QString::number(i)));
        labelElement.appendChild(IDElement);

        QDomElement NameElement = XmlReader.createElement("Name");
        NameElement.appendChild(XmlReader.createTextNode(labelVec[i]));
        labelElement.appendChild(NameElement);

        LabelsElement.appendChild(labelElement);
    }
    root.appendChild(LabelsElement);
    //LabelsElement End

    for (int i = 0; i < Images.size(); ++i) {
        QDomElement ImageElement = XmlReader.createElement("Image");

        QDomElement Img_PathElement = XmlReader.createElement("ImageFilename");
        Img_PathElement.appendChild(XmlReader.createTextNode(Images[i].getImageFilename()));
        ImageElement.appendChild(Img_PathElement);

        QDomElement Img_XmlPathElement = XmlReader.createElement("XmlFilename");
        Img_XmlPathElement.appendChild(XmlReader.createTextNode(Images[i].getXmlFilename()));
        ImageElement.appendChild(Img_XmlPathElement);

        root.appendChild(ImageElement);
    }

    root.save(textStream, 4);
    File.close();
    return true;
}
