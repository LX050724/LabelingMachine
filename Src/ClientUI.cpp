#include "Inc/ClientUI.h"
#include "ui_ClientUI.h"

#include <QHostInfo>
#include <QtWidgets/QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

ClientUI::ClientUI(QWidget *parent, MainWindow *pmainwindow) :
    QWidget(parent),
    Client(new TCP_Client(this)),
    pMainWindow(pmainwindow),
    ui(new Ui::ClientUI)
{
    ui->setupUi(this);

    pMainWindow->Project.setXmlPath("remote");
    pMainWindow->Project.setImgPath("remote");

    TimeoutTimer.setInterval(10000);
    TimeoutTimer.setTimerType(Qt::PreciseTimer);

    connect(&TimeoutTimer, &QTimer::timeout, this, &ClientUI::Timer_timeout);
}

ClientUI::~ClientUI() {
    disconnect(&TimeoutTimer, &QTimer::timeout, this, &ClientUI::Timer_timeout);
    delete Image;
    delete Client;
    delete ui;
}

void ClientUI::on_pushButton_clicked() {
    if(Client->isReady()){
        Client->Disconnect();
        ui->pushButton->setText("连接");
    } else {
        QHostAddress SeverAddress(ui->lineEdit->text());
        if(SeverAddress.isNull()){
            QMessageBox::warning(this, "错误", "IP地址错误");
            return;
        }
        Log_println("开始连接" + SeverAddress.toString());
        if(!Client->Connect(SeverAddress)){
            QMessageBox::warning(this, "错误", "连接失败");
            Log_println("连接失败");
            return;
        }
        else Log_println("连接成功\n等待服务器准备信号");

        pMainWindow->Project.setImgPath("remote");
        pMainWindow->Project.setXmlPath("remote");

        QObject::connect(Client, &TCP_Client::ReceiveData,
                         this, &ClientUI::Client_ReceiveComplete,
                         Qt::QueuedConnection);
        QObject::connect(Client, &TCP_Client::Disconnected,
                         this, &ClientUI::Client_Disconnected,
                         Qt::QueuedConnection);

        ui->pushButton->setText("断开");
    }
}

void ClientUI::Client_ReceiveComplete(const QByteArray &array) {
    if(array.isEmpty()) {
        qWarning() << "Bad Data";
        loadcomplete = true;
        Log_println("Transmission error. Please try again");
        return;
    }
    const char * pData = array.data();
    int DataSize = ToInt(pData, 0) + sizeof(int);
    int flag = ToInt(pData, 1);
    qDebug() << "Size:" << DataSize << "ID:" << flag;
    if(DataSize != array.size()) {
        qWarning() << "Size error" << DataSize + sizeof (int) << array.size();
        if(!loadcomplete){
            Send_IRP(Flag_Image_IRP, filename_now);
        }
        return;
    }
    switch (flag) {
        case Flag_Ready: {
            Log_println("服务器启动\n发送获取标签请求");
            ui->pushButton->setDisabled(true);
            singleProxy();
            Send_IRP(Flag_Label_IRP);
            break;
        }
        case Flag_Label_IRP: {
            Log_println("收到服务器标签数据");
            Receive_Labels(array);
            Send_IRP(Flag_Image_List_IRP);
            Log_println("发送获取图片列表请求");
            break;
        }
        case Flag_Image_List_IRP: {
            Log_println("收到服务器图片列表数据");
            Receive_Image_List(array);
            break;
        }
        case Flag_Image_IRP: {
            Log_println("收到服务器图片数据");
            Receive_Image(array);
            Send_IRP(Flag_Image_BandBoxs_IRP, filename_now);
            break;
        }
        case Flag_Image_BandBoxs_IRP: {
            Log_println("收到服务器BandBox数据");
            Recrive_Image_BandBoxs(array);
            loadimg();
            break;
        }
        case Flag_Image_haslabel: {
            Log_println("收到服务器haslabel数据");
            Receive_Image_HasLabel(array);
            break;
        }
    }
}

void ClientUI::Client_Disconnected(){
    Log_println("连接断开");
    ui->pushButton->setText("连接");
}

void ClientUI::Send_Image_BandBox() {
    /*int size, int flag = Flag_Image_BandBoxs_FB, int w, int h, int len, char[len] filename, int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    if(filename_now.isEmpty())
        return;

    QByteArray data;
    QVector<BandBox> bandboxs = Data.getBandBoxs();

    ImageList[filename_now] = !bandboxs.isEmpty();

    QSize imgsize;
    if(Image != nullptr)
        imgsize = Image->size();

    qDebug() << imgsize;

    int head[] = {
        Flag_Image_BandBoxs_FB,
        imgsize.width(),
        imgsize.height(),
        filename_now.length() };

    data.push_back(QByteArray(ToCharp(head), sizeof(head)));
    data.push_back(filename_now.toLocal8Bit());

    int BandBoxsCount = bandboxs.size();
    data.push_back(QByteArray(ToCharp(&BandBoxsCount), sizeof(int)));

    for(const BandBox &i : bandboxs) {
        int bandbox[] = {
                i.ID,
                i.Rect.x(),
                i.Rect.y(),
                i.Rect.width(),
                i.Rect.height() };
        data.push_back(QByteArray(ToCharp(bandbox), sizeof(bandbox)));
    }
    int flag = Flag_Image_BandBoxs_FB;
    data.push_front(QByteArray(ToCharp(&flag), sizeof(int)));
    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Client->Transmit(data);

    Log_println("已发送BandBox数据");
}

void ClientUI::Send_IRP(ClientUI::IRP n) {
    /*int sizeof(int), int flag */
    int buff[] = {sizeof(int), n};
    Client->Transmit(QByteArray(ToCharp(buff), sizeof(buff)));
}

void ClientUI::Send_IRP(ClientUI::IRP n, const QString &filename) {
    /*int size, int flag, int len,  char[len] filename*/
    int flag[] = {n, filename.length()};
    QByteArray data(ToCharp(flag), sizeof(flag));
    data.push_back(filename.toLocal8Bit());

    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Client->Transmit(data);
}

void ClientUI::Receive_Labels(const QByteArray &array) {
    /*int size, int flag = Flag_Label_IRP, int count, <int len, char[len] label> * count */
    QByteArray DataCopy(array);
    const char * pData = DataCopy.constData();
    int count = ToInt(pData, 2);
    DataCopy.remove(0, sizeof(int) * 3);

    for(int i = 0; i < count; ++i) {
        const char * p = DataCopy.constData();
        int len = ToInt(p, 0);
        pMainWindow->Project.labels.addLabel(QString(DataCopy.mid(sizeof(int), len)));
        DataCopy.remove(0, len + sizeof(int));
    }
    pMainWindow->updateclass();
    pMainWindow->ui->graphicsView->setLabel(0, pMainWindow->Project.labels[0], pMainWindow->Project.labels.LabelCount());
}

void ClientUI::Receive_Image(const QByteArray &array) {
    /*int size, int flag = Flag_Image_IRP, int w, int h, int format, int ImageSize, char[ImaneSize] imgRAW*/
    const char * pData = array.constData();
    int w = ToInt(pData, 2);
    int h = ToInt(pData, 3);
    int format = ToInt(pData, 4);
    ImageRAWData = array;
    ImageRAWData.remove(0, sizeof(int) * 6);
    uchar *ImgData = (uchar *)ImageRAWData.data();
    QImage Img(ImgData, w, h, (QImage::Format)format);
    delete Image;
    Image = new QImage(ImgData, w, h, (QImage::Format)format);
}

void ClientUI::Receive_Image_List(const QByteArray &array) {
    /*int size, int imgCount, <int len, char[len] filename> * imgCount */
    QByteArray DataCopy(array);
    const char * pData = DataCopy.constData();
    int imgCount = ToInt(pData, 2);
    DataCopy.remove(0, sizeof(int) * 3);

    //QStringList imglist;
    ImageList.clear();
    for(int i = 0; i < imgCount; ++i) {
        const char * p = DataCopy.constData();
        int len = ToInt(p, 0);
        ImageList.insertMulti(DataCopy.mid(sizeof(int), len), false);
        //imglist.push_back(QString(DataCopy.mid(sizeof(int), len)));
        DataCopy.remove(0, len + sizeof(int));
    }
    pMainWindow->ui->imgs_listWidget->clear();
    pMainWindow->ui->imgs_listWidget->addItems(ImageList.keys());
}

void ClientUI::Recrive_Image_BandBoxs(const QByteArray &array) {
    /*int size, int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    QByteArray DataCopy(array);
    const char * pData = DataCopy.constData();
    int BandBoxsCount = ToInt(pData, 2);
    DataCopy.remove(0, sizeof(int) * 3);

    bandBoxs.clear();
    for(int i = 0; i < BandBoxsCount; ++i) {
        const int * p = (const int *)DataCopy.constData();
        QString Label = pMainWindow->Project.labels[p[0]];
        bandBoxs.push_back(BandBox(QRect(p[1], p[2], p[3], p[4]),
                                   Label, p[0]));
        DataCopy.remove(0, sizeof(int) * 5);
    }
}

void ClientUI::Receive_Image_HasLabel(const QByteArray &array) {
    /*int size, int flag = Flag_Image_haslabel, int len, char[len] haslabel*/
    const char *pData = array.constData();
    const char *phaslabel = pData + sizeof(int) * 3;
    int len = ToInt(pData, 2);
    QVector<QString> filename = ImageList.keys().toVector();
    for(int i = 0; i < len; ++i) {
        ImageList[filename[i]] = phaslabel[i];
    }
}

void ClientUI::Proxy_nextimg() {
    if(loadcomplete) {
        Send_Image_BandBox();
        if(pMainWindow->ui->imgs_listWidget->count() > 0) {
            loadcomplete = false;
            TimeoutTimer.start();
            int row = pMainWindow->ui->imgs_listWidget->currentRow() + 1;
            QListWidgetItem* Item = pMainWindow->ui->imgs_listWidget->item(row);
            if(Item != nullptr) {
                pMainWindow->ui->imgs_listWidget->setCurrentRow(row);
                filename_now = Item->text();
                Send_IRP(Flag_Image_IRP, filename_now);
            } else qInfo() << "next Item is null";
        }
    } else qInfo("Busy");
}

void ClientUI::Proxy_lastimg() {
    if(loadcomplete) {
        Send_Image_BandBox();
        if(pMainWindow->ui->imgs_listWidget->count() > 0) {
            loadcomplete = false;
            TimeoutTimer.start();
            int row = pMainWindow->ui->imgs_listWidget->currentRow() - 1;
            QListWidgetItem* Item = pMainWindow->ui->imgs_listWidget->item(row);
            if(Item != nullptr) {
                pMainWindow->ui->imgs_listWidget->setCurrentRow(row);
                filename_now = Item->text();
                Send_IRP(Flag_Image_IRP, filename_now);
            } else qInfo() << "next Item is null";
        }
    } else qInfo("Busy");
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
    if(Key == Qt::Key_QuoteLeft || (Qt::Key_0 <= Key && Key <= Qt::Key_9)) {
        if(Key == Qt::Key_QuoteLeft)
            Key = 0;
        else
            Key -= Qt::Key_0;

        int count = pMainWindow->ui->class_tableWidget->rowCount();
        if(Key < count) {
            pMainWindow->ui->class_tableWidget->setCurrentCell(Key, 0);
            QTableWidgetItem* Item = pMainWindow->ui->class_tableWidget->item(Key, 1);
            if(Item != nullptr) {
                pMainWindow->ui->graphicsView->setLabel(Key, Item->text(),
                                                        pMainWindow->Project.labels.LabelCount());
            } else {
                pMainWindow->ui->graphicsView->setLabel(0, QString(), 0);
            }
        }
    }
}

void ClientUI::Proxy_imgs_listWidget_itemClicked(QListWidgetItem *item) {
    if(loadcomplete){
        if(!item->text().isEmpty()) {
            Send_Image_BandBox();
            filename_now = item->text();
            Send_IRP(Flag_Image_IRP, filename_now);
        }
    } else qInfo("Busy");
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
            for(const QString &i : allimg) {
                if(ImageList[i])
                    imglist.push_back(i);
            }
            pMainWindow->ui->imgs_listWidget->addItems(imglist);
            break;
        }
        case NoMarked: {
            for(const QString &i : allimg) {
                if(!ImageList[i])
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
    if(!loadcomplete) {
        loadcomplete = true;
        QMessageBox::warning(this, "warning", "操作超时");
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

inline void ClientUI::Log_printf(const char *format, ...) {
    char buff[200];
    va_list valist;
    va_start(valist, format);
    vsnprintf(buff, 200, format, valist);
    va_end(valist);
    ui->textBrowser->append(buff);
    ui->textBrowser->moveCursor(QTextCursor::End);
}

inline void ClientUI::Log_println(const QString& _Log) {
    ui->textBrowser->append(_Log + '\n');
    ui->textBrowser->moveCursor(QTextCursor::End);
}

inline void ClientUI::Log_putc(char c) {
    ui->textBrowser->append(QString(c));
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
