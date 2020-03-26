#include <QtWidgets/QMessageBox>
#include "Inc/ServerUI.h"
#include "ui_ServerUI.h"

#include "TCP_Server.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

ServerUI::ServerUI(QWidget *parent, MainWindow* pmainwindow) :
    QWidget(parent),
    ui(new Ui::ServerUI)
{
    ui->setupUi(this);
    Server = new  TCP_Server(this);
    pMainWindow = pmainwindow;
    if(!Server->startListing()) {
        Failed = true;
        return;
    }
    Ready = true;

    Log_println("TCP服务器已就绪");

    connect(Server, &TCP_Server::ReceiveComplete,
            this, &ServerUI::Sever_ReceiveComplete);
    connect(Server, &TCP_Server::TCPNewConnection,
            this, &ServerUI::Sever_TCPNewConnection);
    connect(Server, &TCP_Server::TCPDisconnected,
            this, &ServerUI::Sever_TCPDisconnected);
    connect(Server, &TCP_Server::acceptError,
            this, &ServerUI::Sever_acceptError);
}

ServerUI::~ServerUI()
{
    if(!Failed) {
        disconnect(Server, &TCP_Server::ReceiveComplete,
                this, &ServerUI::Sever_ReceiveComplete);
        disconnect(Server, &TCP_Server::TCPNewConnection,
                this, &ServerUI::Sever_TCPNewConnection);
        disconnect(Server, &TCP_Server::TCPDisconnected,
                this, &ServerUI::Sever_TCPDisconnected);
        disconnect(Server, &TCP_Server::acceptError,
                this, &ServerUI::Sever_acceptError);
    }
    delete Server;
    delete ui;
}

void ServerUI::Sever_ReceiveComplete(const QByteArray &Data, const QHostAddress &Address) {
    if(Data.isEmpty()) {
        qWarning() << "Bad Data";
        Log_println("Transmission error. Please try again");
        return;
    }
    const char * pData = Data.data();
    int DataSize = ToInt(pData, 0);
    int flag = ToInt(pData, 1);
    qDebug() << "Size:" << DataSize << "ID:" << flag;
    if(DataSize != Data.size() - (int)sizeof(int)) {
        qWarning() << "Size error" << DataSize + sizeof (int) << Data.size();
        return;
    }
    switch (flag) {
        case Flag_Label_IRP: {
            Send_Labels(Address);
            Log_printf("收到%s的获取标签请求\n",
                    QHostAddress2c_str(Address));
            break;
        }
        case Flag_Image_IRP: {
            int len = ToInt(pData, 2);
            QString name(Data.mid(sizeof(int) * 3, len));
            Send_Image(Address, name);
            Log_printf("收到来自%s的获取图片%s请求\n",
                    QHostAddress2c_str(Address), name.toStdString().c_str());
            break;
        }
        case Flag_Image_BandBoxs_IRP: {
            int len = ToInt(pData, 2);
            QString name(Data.mid(sizeof(int) * 3, len));
            Send_Image_BandBoxs(Address, name);
            Log_printf("收到来自%s的获取%s的Bandbox请求\n",
                    QHostAddress2c_str(Address), name.toStdString().c_str());
            break;
        }
        case Flag_Image_List_IRP: {
            Send_Image_List(Address);
            Log_printf("收到%s的获取图片列表请求\n",
                       QHostAddress2c_str(Address));
            break;
        }
        case Flag_Image_BandBoxs_FB: {
            QString filename = Receive_Image_BandBox(Data);
            Log_printf("收到%s的BandBox,已保存\n",
                       filename.data());
            break;
        }
    }
}

void ServerUI::on_pushButton_clicked() {
    if((ClientConut = Server->getCilentConut()) == 0) {
        QMessageBox::warning(this, "error", "没有客户端连接");
        return;
    }

    ui->pushButton->setDisabled(true);
    ClientList = Server->getClientAddressList();

    Log_printf("停止接入,已接入%d\n分配任务...\n", ClientConut);
    allocation(pMainWindow->Project.no_label_Img());

    Log_printf("分配完成\n总数%d\n", pMainWindow->Project.all_Img().size());
    Log_printf("Server:%d张\n", TaskList.front().size());
    for(int i = 0; i < ClientList.size(); ++i) {
        Log_printf("%s:%d张\n", ClientList[i].toString().toStdString().c_str(),
                   TaskList[i + 1].size());
        SendSingle_Ready(ClientList[i]);
    }

    disconnect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
            pMainWindow, SLOT(comboBox_currentIndexChanged(int)));

    connect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(Proxy_comboBox_currentIndexChanged(int)));
}

void ServerUI::Sever_TCPNewConnection(const QHostAddress &Address) {
    QList<QString> Addresslist;
    if(ClientConut > 0) {
        if(ClientList.indexOf(Address) >= 0) {
            Log_printf("%s重新接入\n", ToCharp(Address.toString().toLocal8Bit()));
            Log_println("发送开始信号");
            SendSingle_Ready(Address);
        } else return;
    } else {
        Log_printf("%s 接入\n", ToCharp(Address.toString().toLocal8Bit()));
    }
    for(const QHostAddress& i : Server->getClientAddressList())
        Addresslist.push_back(i.toString());
    ui->listWidget->clear();
    ui->listWidget->addItems(Addresslist);
    ui->listWidget->sortItems();
}

void ServerUI::Sever_TCPDisconnected(const QHostAddress &Address) {
    if(ClientConut >= 0 && ClientList.indexOf(Address) == -1)
        return;
    QList<QString> Addresslist;
    for(const QHostAddress& i : Server->getClientAddressList())
        Addresslist.push_back(i.toString());
    ui->listWidget->clear();
    ui->listWidget->addItems(Addresslist);
    ui->listWidget->sortItems();
    Log_printf("%s断开连接\n", Address.toString().toStdString().c_str());
}

void ServerUI::Sever_acceptError(QAbstractSocket::SocketError socketError) {
    qWarning() << "on_HostSever_acceptError" << socketError;
}

void ServerUI::Proxy_comboBox_currentIndexChanged(int index) {
    if(TaskList.isEmpty())
        return;

    QStringList Itmes;
    switch (index) {
        case all: {
            for(const QString &i : TaskList.front())
                Itmes.push_back(i);
            break;
        }
        case Marked: {
            for(const QString &i : TaskList.front()) {
                int Index = pMainWindow->Project.findImage(i);
                if(pMainWindow->Project.Images[Index].isHasLabel())
                    Itmes.push_back(i);
            }
            break;
        }
        case NoMarked: {
            for(const QString &i : TaskList.front()) {
                int Index = pMainWindow->Project.findImage(i);
                if(!pMainWindow->Project.Images[Index].isHasLabel())
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

inline void ServerUI::Log_println(const QString& _Log) {
    ui->textBrowser->append(_Log + '\n');
    ui->textBrowser->moveCursor(QTextCursor::End);
}

inline void ServerUI::Log_putc(char c) {
    ui->textBrowser->append(QString(c));
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void ServerUI::allocation(const QVector<ImageData> &Img) {
    int div = Img.size() / (ClientConut + 1);
    QVector<QString> Task;
    for(int n = 0; n < ClientConut; ++n) {
        for(int i = 0; i < div; ++i) {
            Task.push_back(Img[div * n + i].getImageFilename());
        }
        TaskList.push_back(Task);
        Task.clear();
    }
    for(int i = div * ClientConut; i < Img.size(); ++i) {
        Task.push_back(Img[i].getImageFilename());
    }
    TaskList.push_back(Task);
    Proxy_comboBox_currentIndexChanged(all);
}

void ServerUI::SendSingle_Ready(const QHostAddress &Address) {
    /*int 0x04, int 0x01 */
    int buff[] = {sizeof(int), Flag_Ready};
    Server->SendTo(Address, QByteArray(ToCharp(buff), sizeof(buff)));
}

void ServerUI::Send_Labels(const QHostAddress &Address) {
    /*int size, int flag = Flag_Label_IRP, int count, <int len, char[len] label> * count */
    QVector<QString> labels = pMainWindow->Project.labels.getLabels();
    int count = labels.size();
    QByteArray data(ToCharp(&count), sizeof(int));
    for(const QString &i : labels) {
        int Len = i.length();
        data.push_back(QByteArray(ToCharp(&Len), sizeof(int)));
        data.push_back(i.toLocal8Bit());
    }

    int flag = Flag_Label_IRP;
    data.push_front(QByteArray(ToCharp(&flag), sizeof(int)));
    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Server->SendTo(Address, data);
}

void ServerUI::Send_Image(const QHostAddress &Address, const QString &name) {
    /*int size, int flag = Flag_Image_IRP, int w, int h, int format, int ImageSize, char[ImaneSize] imgRAW*/
    int Index = pMainWindow->Project.findImage(name);
    QImage Img = pMainWindow->Project.all_Img()[Index].loadImage();
    int head[] = {
            Flag_Image_IRP,
            Img.width(),
            Img.height(),
            Img.format(),
            (int)Img.sizeInBytes() };

    QByteArray data(ToCharp(head), sizeof(head));
    data.push_back(QByteArray(ToCharp(Img.constBits()), head[4]));

    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    qDebug() << size << Img.sizeInBytes();
    Server->SendTo(Address, data);
}

void ServerUI::Send_Image_List(const QHostAddress &Address) {
    /*int size, int imgCount, <int len, char[len] filename> * imgCount */
    QVector<QString> imgs = TaskList[ClientList.indexOf(Address) + 1];
    int imgCount = imgs.size();
    QByteArray data(ToCharp(&imgCount), sizeof(int));

    for(const QString &i : imgs) {
        int len = i.length();
        data.push_back(QByteArray(ToCharp(&len), sizeof(int)));
        data.push_back(i.toLocal8Bit());
    }

    int flag = Flag_Image_List_IRP;
    data.push_front(QByteArray(ToCharp(&flag), sizeof(int)));
    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Server->SendTo(Address, data);

    Send_Image_HasLabel(Address);
}

void ServerUI::Send_Image_BandBoxs(const QHostAddress& Address, const QString &name) {
    /*int size, int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    int Index = pMainWindow->Project.findImage(name);
    QVector<BandBox> BandBoxs = pMainWindow->Project.all_Img()[Index].getBandBoxs();

    int BandBoxsCount = BandBoxs.size();
    QByteArray data(ToCharp(&BandBoxsCount), sizeof(int));
    for(const BandBox &i : BandBoxs) {
        int buff[5] = { i.ID,
                        i.Rect.x(),
                        i.Rect.y(),
                        i.Rect.width(),
                        i.Rect.height()};
        data.push_back(QByteArray(ToCharp(buff), sizeof(int) * 5));
    }

    int flag = Flag_Image_BandBoxs_IRP;
    data.push_front(QByteArray(ToCharp(&flag), sizeof(int)));
    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Server->SendTo(Address, data);
}

const QString ServerUI::Receive_Image_BandBox(const QByteArray &array) {
    /*int size, int flag = Flag_Image_BandBoxs_FB, int w, int h, int len, char[len] filename, int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    QByteArray DataCopy(array);
    const char * pData = DataCopy.constData();
    int w = ToInt(pData, 3);
    int h = ToInt(pData, 4);
    int len = ToInt(pData, 5);
    QString filename(DataCopy.mid(sizeof(int) * 6, len));
    DataCopy.remove(0, len + sizeof(int) * 6);

    /*int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    pData = DataCopy.constData();
    int BandBoxsCount = ToInt(pData, 0);
    DataCopy.remove(0, sizeof(int));
    QVector<BandBox> bandBoxs;
    for(int i = 0; i < BandBoxsCount; ++i) {
        const int * p = (const int *)DataCopy.constData();
        QString Label = pMainWindow->Project.labels[p[0]];
        bandBoxs.push_back(BandBox(
                QRect(p[1], p[2], p[3], p[4]),
                Label, p[0]));
        DataCopy.remove(0, sizeof(int) * 5);
    }
    int Index = pMainWindow->Project.findImage(filename);
    if(Index >= 0) {
        qDebug() << w << h;
        pMainWindow->Project.Images[Index].setsize(QSize(w, h));
        pMainWindow->Project.Images[Index].setBandBoxs(bandBoxs);
        pMainWindow->Project.Images[Index].saveXml();
    } else qWarning("Receive_Image_BandBox error");
    return filename;
}

const char *ServerUI::QHostAddress2c_str(const QHostAddress &Address) {
    return Address.toString().toUtf8().data();
}

void ServerUI::Send_Image_HasLabel(const QHostAddress &Address) {
    /*int size, int flag = Flag_Image_haslabel, int len, char[len] haslabel*/
    if(!ClientList.isEmpty()) {
        int Index = ClientList.indexOf(Address);
        QVector<QString> imagelist = TaskList[Index + 1];
        QByteArray data;
        int head[] = {Flag_Image_haslabel, imagelist.size() };
        data.push_back(QByteArray(ToCharp(&head), sizeof(head)));
        for(const QString &filename : imagelist) {
            int i = pMainWindow->Project.findImage(filename);
            char haslabel = pMainWindow->Project.Images[i].isHasLabel();
            data.push_back(haslabel);
        }
        int size = data.size();
        data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
        Server->SendTo(Address, data);
    }
}
