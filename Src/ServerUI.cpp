#include <QtWidgets/QMessageBox>
#include "Inc/ServerUI.h"
#include "ui_ServerUI.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

ServerUI::ServerUI(QWidget *parent, MainWindow *pmainwindow) :
        QWidget(parent), ui(new Ui::ServerUI), logger(__FUNCTION__) {
    ui->setupUi(this);
    this->pMainWindow = pmainwindow;
    /*! throw runtime error when Tcp is failed */
    rcsServer = new RCS_Server(8848, false);
    rcsClient = new RCS_Client("Server", QHostAddress(QHostAddress::LocalHost), 8848, this);
    rcsClient->RegisterGetCallBack("ImageList", this, &ServerUI::Send_Image_List);
    rcsClient->RegisterGetCallBack("Labels", this, &ServerUI::Send_Labels);
    rcsClient->RegisterGetCallBack("Image", this, &ServerUI::Send_Image);
    rcsClient->RegisterPushCallBack("BandBoxs", this, &ServerUI::Receive_Image_BandBox);
    connect(rcsServer, &RCS_Server::NewClient,
            this, &ServerUI::Sever_TCPNewConnection);
}

ServerUI::~ServerUI() {
    delete rcsServer;
    delete ui;
}

void ServerUI::on_pushButton_clicked() {
    QStringList clientNameList = rcsServer->getClientNameList();

    if (clientNameList.size() == 0) {
        QMessageBox::warning(this, tr("error"), tr("No client connection"));
        return;
    }

    Log_printf("Stopped accessing,%d is accessed\nAssign tasks...\n", TaskMap.size());
    allocation(pMainWindow->Project.no_label_Img());

    ui->pushButton->setDisabled(true);

    Log_printf("Assigned to complete\nThe total number of %d\n", pMainWindow->Project.all_Img().size());

    auto it = TaskMap.begin();

    while (it != TaskMap.end()) {
        Log_printf("%s:%d\n", it.key().toLocal8Bit().data(), it.value().size());
        it++;
    }

    rcsClient->BROADCAST("Ready", {});
    ready = true;

    disconnect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
               pMainWindow, SLOT(comboBox_currentIndexChanged(int)));

    connect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(Proxy_comboBox_currentIndexChanged(int)));
}

void ServerUI::Sever_TCPNewConnection(const QHostAddress &addr, const QString &name) {
    logger.info("New Client: addr={}, name={}", addr, name);
    if (ready) {
        if(TaskMap.contains(name)) {
            rcsClient->BROADCAST("Ready", {});
        } else {
            rcsServer->disconnect(name);
        }
    } else {
        ui->listWidget->clear();
        ui->listWidget->addItems(rcsServer->getClientNameList());
        ui->listWidget->sortItems();
    }
    Log_println(name + " connect");
}

void ServerUI::Proxy_comboBox_currentIndexChanged(int type) {
    if (TaskMap.isEmpty())
        return;
    auto ServerTask = TaskMap["Server"];
    QStringList Itmes;
    switch (type) {
        case all: {
            for (const QString &i : ServerTask)
                Itmes.push_back(i);
            break;
        }
        case Marked: {
            for (const QString &i : ServerTask) {
                int Index = pMainWindow->Project.findImage(i);
                if (pMainWindow->Project.Images[Index].isHasLabel())
                    Itmes.push_back(i);
            }
            break;
        }
        case NoMarked: {
            for (const QString &i : ServerTask) {
                int Index = pMainWindow->Project.findImage(i);
                if (!pMainWindow->Project.Images[Index].isHasLabel())
                    Itmes.push_back(i);
            }
            break;
        }
    }
    pMainWindow->ui->imgs_listWidget->clear();
    pMainWindow->ui->imgs_listWidget->addItems(Itmes);
}

inline void ServerUI::Log_printf(const char *format, ...) {
    char buff[200];
    va_list valist;
            va_start(valist, format);
    vsnprintf(buff, 200, format, valist);
            va_end(valist);
    ui->textBrowser->append(buff);
    ui->textBrowser->moveCursor(QTextCursor::End);
}

inline void ServerUI::Log_println(const QString &_Log) {
    ui->textBrowser->append(_Log + '\n');
    ui->textBrowser->moveCursor(QTextCursor::End);
}

inline void ServerUI::Log_putc(char c) {
    ui->textBrowser->append(QString(c));
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void ServerUI::allocation(const QVector<ImageData> &Img) {
    size_t clientCount = rcsServer->getClientCount();
    auto clientNameList = rcsServer->getClientNameList().toVector();
    int div = Img.size() / clientCount;
    logger.debug("clientCount {}, div {}", clientCount, div);
    QVector<QString> Task;
    for (int n = 0; n < clientCount - 1; ++n) {
        for (int i = 0; i < div; ++i) {
            logger.debug("clientNameList[{}] {} += {}", n, clientNameList[n], Img[div * n + i].getImageFilename());
            Task.push_back(Img[div * n + i].getImageFilename());
        }
        TaskMap.insert(clientNameList[n], Task);
        Task.clear();
    }
    for (int i = div * (clientCount - 1); i < Img.size(); ++i) {
        logger.debug("clientNameList[{}] {} += {}", clientCount - 1, clientNameList[clientCount - 1], Img[i].getImageFilename());
        Task.push_back(Img[i].getImageFilename());
    }
    TaskMap.insert(clientNameList[clientCount - 1], Task);
    Proxy_comboBox_currentIndexChanged(all);
}

QJsonObject ServerUI::Send_Labels(const QString &name, const QJsonObject &info) {
    QVector<QString> labels = pMainWindow->Project.labels.getLabels();
    QJsonArray array;
    for (const QString &i : labels) {
        array.push_back(i);
    }
    return {{"lables", array}};
}

QJsonObject ServerUI::Send_Image(const QString &name, const QJsonObject &info) {
    QString filename = info.find("filename")->toString();
    logger.info("send image {}", filename);
    int Index = pMainWindow->Project.findImage(filename);
    QString ImagePath = pMainWindow->Project.all_Img()[Index].getImagePath();
    QString ImageFilename = pMainWindow->Project.all_Img()[Index].getImageFilename();
    QFile file;
    file.setFileName(ImagePath + '/' + ImageFilename);
    file.open(QFile::ReadOnly);
    QByteArray imgdata = file.readAll();
    file.close();
    QVector<BandBox> BandBoxs = pMainWindow->Project.all_Img()[Index].getBandBoxs();
    QJsonArray array;
    for (const auto &i : BandBoxs) {
        array.push_back((QJsonObject) i);
    }
    return {{"img",      QString(imgdata.toBase64())},
            {"BandBoxs", array}};
}

QJsonObject ServerUI::Send_Image_List(const QString &name, const QJsonObject &info) {
    QJsonArray ImgArray;
    const auto imagelist = TaskMap[name];

    for (const QString &filename : imagelist) {
        int i = pMainWindow->Project.findImage(filename);
        bool haslabel = pMainWindow->Project.Images[i].isHasLabel();
        ImgArray.push_back(QJsonObject({{"Image",    filename},
                                        {"HasLabel", haslabel}}));
    }

    QVector<QString> labels = pMainWindow->Project.labels.getLabels();
    QJsonArray LableArray;
    for (const QString &i : labels) {
        LableArray.push_back(i);
    }
    return {{"ImageList", ImgArray},
            {"lables",    LableArray}};
}

void ServerUI::Receive_Image_BandBox(const QString &name, const QJsonObject &info) {
    QString filename = info.find("filename")->toString();
    int h = info.find("height")->toInt();
    int w = info.find("weight")->toInt();
    auto array = info.find("bandbox")->toArray();
    QVector<BandBox> bandBoxs;
    for (const auto &i : array) {
        bandBoxs.push_back(i.toObject());
    }
    int Index = pMainWindow->Project.findImage(filename);
    if (Index >= 0) {
        pMainWindow->Project.Images[Index].setsize(QSize(w, h));
        pMainWindow->Project.Images[Index].setBandBoxs(bandBoxs);
        pMainWindow->Project.Images[Index].saveXml();
    } else logger.error("Receive_Image_BandBox error {}", info);
}
