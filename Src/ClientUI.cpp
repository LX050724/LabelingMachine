#include "Inc/ClientUI.h"
#include "ui_ClientUI.h"

#include <QtWidgets/QMessageBox>
#include <RCS_Server.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"

ClientUI::ClientUI(QWidget *parent, MainWindow *pmainwindow) :
        QWidget(parent),
        logger(__FUNCTION__),
        pMainWindow(pmainwindow),
        ui(new Ui::ClientUI) {
    ui->setupUi(this);
    Image = new QImage;
    pMainWindow->Project.setXmlPath("remote");
    pMainWindow->Project.setImgPath("remote");

    TimeoutTimer.setInterval(10000);
    TimeoutTimer.setTimerType(Qt::PreciseTimer);

    connect(&TimeoutTimer, &QTimer::timeout, this, &ClientUI::Timer_timeout);
}

ClientUI::~ClientUI() {
    delete Image;
    delete ui;
}

void ClientUI::Client_Disconnected(const QString &) {
    Log_println(tr("Connection is broken"));
    ui->connectButton->setText(tr("The connection"));
}

void ClientUI::Send_Image_BandBox() {
    if (filename_now.isEmpty())
        return;

    QJsonArray array;
    for (const auto &i : Data.getBandBoxs()) {
        array.push_back((QJsonObject) i);
    }
    ImageList[filename_now] = !array.isEmpty();
    QSize imgsize = (Image != nullptr ? Image->size() : QSize());

    rcsClient->PUSH(RCS_Server::__NAME__, "BandBoxs", {
            {"filename", filename_now},
            {"height",   imgsize.height()},
            {"weight",   imgsize.width()},
            {"bandbox",  array}
    });
    Log_println(tr("BandBox data has been sent"));
}

void ClientUI::Receive_Image(const QString &from, const QJsonObject &obj) {
    auto array = obj.find("BandBoxs")->toArray();
    QByteArray base64 = obj.find("img")->toVariant().toByteArray();
    ImageRAWData = QByteArray::fromBase64(base64);
    Image->loadFromData(ImageRAWData);
    bandBoxs.clear();
    for (const auto &i : array) {
        bandBoxs.push_back(i.toObject());
    }
    loadimg();
    logger.info("Image receive sucesses");
}

void ClientUI::Receive_Image_List(const QString &from, const QJsonObject &obj) {
    auto ImgArray = obj.find("ImageList")->toArray();
    auto LableArray = obj.find("lables")->toArray();
    for (const auto &i : ImgArray) {
        auto object = i.toObject();
        auto filename = object.find("Image")->toString();
        auto haslabel = object.find("HasLabel")->toBool();
        ImageList.insert(filename, haslabel);
    }
    for (const auto &i : LableArray) {
        pMainWindow->Project.labels.addLabel(i.toString());
    }
    pMainWindow->updateclass();
    pMainWindow->ui->graphicsView->setLabel(0, pMainWindow->Project.labels[0],
                                            pMainWindow->Project.labels.LabelCount());
    pMainWindow->ui->imgs_listWidget->clear();
    pMainWindow->ui->imgs_listWidget->addItems(ImageList.keys());
}

void ClientUI::Recrive_Image_BandBoxs(const QString &from, const QJsonObject &obj) {
    auto array = obj.find("BandBoxs")->toArray();
    bandBoxs.clear();
    for (const auto &i : array) {
        bandBoxs.push_back(i.toObject());
    }
}

void ClientUI::Proxy_nextimg() {
    if (loadcomplete) {
        Send_Image_BandBox();
        if (pMainWindow->ui->imgs_listWidget->count() > 0) {
            loadcomplete = false;
            TimeoutTimer.start();
            int row = pMainWindow->ui->imgs_listWidget->currentRow() + 1;
            QListWidgetItem *Item = pMainWindow->ui->imgs_listWidget->item(row);
            if (Item != nullptr) {
                pMainWindow->ui->imgs_listWidget->setCurrentRow(row);
                filename_now = Item->text();
                logger.info("send Get image {}", filename_now);
                rcsClient->GET(RCS_Server::__NAME__, "Image", {{"filename", filename_now}});
            } else {
                loadcomplete = true;
                qInfo() << "next Item is null";
            }
        }
    } else
        qInfo("Busy");
}

void ClientUI::Proxy_lastimg() {
    if (loadcomplete) {
        Send_Image_BandBox();
        if (pMainWindow->ui->imgs_listWidget->count() > 0) {
            loadcomplete = false;
            TimeoutTimer.start();
            int row = pMainWindow->ui->imgs_listWidget->currentRow() - 1;
            QListWidgetItem *Item = pMainWindow->ui->imgs_listWidget->item(row);
            if (Item != nullptr) {
                pMainWindow->ui->imgs_listWidget->setCurrentRow(row);
                filename_now = Item->text();
                logger.info("send Get image {}", filename_now);
                rcsClient->GET(RCS_Server::__NAME__, "Image", {{"filename", filename_now}});
            } else {
                loadcomplete = true;
                qInfo() << "next Item is null";
            }
        }
    } else
        qInfo("Busy");
}

void ClientUI::Proxy_Keypress(int Key) {
    switch (Key) {
        case Qt::Key_Q:
            Proxy_lastimg();
            break;
        case Qt::Key_E:
            Proxy_nextimg();
            break;
    }
    if (Key == Qt::Key_QuoteLeft || (Qt::Key_0 <= Key && Key <= Qt::Key_9)) {
        if (Key == Qt::Key_QuoteLeft)
            Key = 0;
        else
            Key -= Qt::Key_0;

        int count = pMainWindow->ui->class_tableWidget->rowCount();
        if (Key < count) {
            pMainWindow->ui->class_tableWidget->setCurrentCell(Key, 0);
            QTableWidgetItem *Item = pMainWindow->ui->class_tableWidget->item(Key, 1);
            if (Item != nullptr) {
                pMainWindow->ui->graphicsView->setLabel(Key, Item->text(),
                                                        pMainWindow->Project.labels.LabelCount());
            } else {
                pMainWindow->ui->graphicsView->setLabel(0, QString(), 0);
            }
        }
    }
}

void ClientUI::Proxy_imgs_listWidget_itemClicked(QListWidgetItem *item) {
    if (!item->text().isEmpty()) {
        Send_Image_BandBox();
        filename_now = item->text();
        logger.info("send Get image {}", filename_now);
        rcsClient->GET(RCS_Server::__NAME__, "Image", {{"filename", filename_now}});
    }
}

void ClientUI::Proxy_comboBox_currentIndexChanged(int index) {
    pMainWindow->ui->imgs_listWidget->clear();
    QStringList allimg = ImageList.keys();
    QStringList imglist;
    switch (index) {
        case all: {
            pMainWindow->ui->imgs_listWidget->addItems(allimg);
            break;
        }
        case Marked: {
            for (const QString &i : allimg) {
                if (ImageList[i])
                    imglist.push_back(i);
            }
            pMainWindow->ui->imgs_listWidget->addItems(imglist);
            break;
        }
        case NoMarked: {
            for (const QString &i : allimg) {
                if (!ImageList[i])
                    imglist.push_back(i);
            }
            pMainWindow->ui->imgs_listWidget->addItems(imglist);
            break;
        }
    }
}

void ClientUI::Proxy_save_triggered() {
    Send_Image_BandBox();
}

void ClientUI::Timer_timeout() {
    if (!loadcomplete) {
        loadcomplete = true;
        QMessageBox::warning(this, tr("warning"), tr("Operation timed out"));
    }
}

void ClientUI::singleProxy() {
    disconnect(pMainWindow->ui->nextimg_pushButton, &QPushButton::clicked,
               pMainWindow, &MainWindow::nextimg_pushButton_clicked);
    disconnect(pMainWindow->ui->lastimg_pushButton, &QPushButton::clicked,
               pMainWindow, &MainWindow::lastimg_pushButton_clicked);
    disconnect(pMainWindow->ui->graphicsView, &BandBoxView::Keypress,
               pMainWindow, &MainWindow::graphicsView_Keypress);
    disconnect(pMainWindow->ui->imgs_listWidget, &QListWidget::itemClicked,
               pMainWindow, &MainWindow::imgs_listWidget_itemClicked);
    disconnect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
               pMainWindow, SLOT(comboBox_currentIndexChanged(int)));
    disconnect(pMainWindow->ui->save, &QAction::triggered,
               pMainWindow, &MainWindow::save_triggered);

    connect(pMainWindow->ui->nextimg_pushButton, &QPushButton::clicked,
            this, &ClientUI::Proxy_nextimg);
    connect(pMainWindow->ui->lastimg_pushButton, &QPushButton::clicked,
            this, &ClientUI::Proxy_lastimg);
    connect(pMainWindow->ui->graphicsView, &BandBoxView::Keypress,
            this, &ClientUI::Proxy_Keypress);
    connect(pMainWindow->ui->imgs_listWidget, &QListWidget::itemClicked,
            this, &ClientUI::Proxy_imgs_listWidget_itemClicked);
    connect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(Proxy_comboBox_currentIndexChanged(int)));
    connect(pMainWindow->ui->save, &QAction::triggered,
            this, &ClientUI::Proxy_save_triggered);
}

void ClientUI::Log_printf(const char *format, ...) {
    char buff[512];
    va_list valist;
            va_start(valist, format);
    vsnprintf(buff, 512, format, valist);
            va_end(valist);
    ui->textBrowser->append(buff);
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void ClientUI::Log_println(const QString &_Log) {
    ui->textBrowser->append(_Log + '\n');
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void ClientUI::loadimg() {
    Data = ImageData(*Image, bandBoxs);
    Data.setRemotemod(true);
    pMainWindow->ui->graphicsView->loadimg(&Data);
    pMainWindow->updatelabel(Data);
    loadcomplete = true;
    TimeoutTimer.stop();
}

void ClientUI::on_connectButton_clicked() {
    auto text = tr("Disconnect");
    if (ui->connectButton->text() == text) {
        delete rcsClient;
        ui->connectButton->setText(tr("Connect"));
        return;
    }

    if (ui->nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("error"), tr("name is empty"));
        return;
    }

    QString IP = ui->addrEdit->text();
    QString name = ui->nameEdit->text();

    try {
        QHostAddress addr(IP);
        if (addr.isNull()) {
            QMessageBox::warning(this, tr("error"), tr("IP address error"));
            return;
        }
        rcsClient = new RCS_Client(name, addr, 8848, this);
    } catch (const std::runtime_error &e) {
        delete rcsClient;
        rcsClient = nullptr;
        logger.error("error: {}", e.what());
        return;
    }
    Log_println(tr("The connection is successful"));
    Log_println(tr("Wait for the server to prepare the signal"));

    pMainWindow->Project.setImgPath("remote");
    pMainWindow->Project.setXmlPath("remote");
    rcsClient->RegisterPushCallBack("ImageList", this, &ClientUI::Receive_Image_List);
    rcsClient->RegisterPushCallBack("BandBoxs", this, &ClientUI::Recrive_Image_BandBoxs);
    rcsClient->RegisterPushCallBack("Image", this, &ClientUI::Receive_Image);
    connect(rcsClient, SIGNAL(BROADCAST(const QString &, const QString &, const QJsonObject &)),
            this, SLOT(BROADCAST(const QString &, const QString &, const QJsonObject &)));
    connect(rcsClient, SIGNAL(disconnected(const QString &)),
            this, SLOT(Client_Disconnected(const QString &)));
    connect(rcsClient, SIGNAL(RETURN(TcpConnect::PACK_TYPE, const QJsonObject &)),
            this, SLOT(RETURN(TcpConnect::PACK_TYPE, const QJsonObject &)));

    ui->connectButton->setText(text);
}

void ClientUI::BROADCAST(const QString &from, const QString &broadcastName, const QJsonObject &data) {
    if (!ready) {
        rcsClient->GET(from, "ImageList");
        singleProxy();
    }
    ready = true;
}

void ClientUI::RETURN(TcpConnect::PACK_TYPE type, const QJsonObject &info) {
    switch (type) {
        case TcpConnect::SERVER_RET:
            Log_printf("service return %s", QJsonDocument(info).toJson().data());
            break;
        case TcpConnect::CLIENT_RET:
            Log_printf("Client return %s", QJsonDocument(info).toJson().data());
            break;
        default:
            break;
    }
}
