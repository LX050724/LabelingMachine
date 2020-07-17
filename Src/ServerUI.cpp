#include <QtWidgets/QMessageBox>
#include "Inc/ServerUI.h"
#include "ui_ServerUI.h"

#include "TCP_Server.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

ServerUI::ServerUI(QWidget *parent, MainWindow *pmainwindow) :
        QWidget(parent),
        ui(new Ui::ServerUI) {
    ui->setupUi(this);
    Server = new TCP_Server(this);
    pMainWindow = pmainwindow;
    if (!Server->startListing()) {
        Failed = true;
        return;
    }
    Ready = true;

    Log_println(tr("The TCP server is ready"));

    connect(Server, &TCP_Server::ReceiveComplete,
            this, &ServerUI::Sever_ReceiveComplete);
    connect(Server, &TCP_Server::TCPNewConnection,
            this, &ServerUI::Sever_TCPNewConnection);
    connect(Server, &TCP_Server::TCPDisconnected,
            this, &ServerUI::Sever_TCPDisconnected);
    connect(Server, &TCP_Server::acceptError,
            this, &ServerUI::Sever_acceptError);
}

ServerUI::~ServerUI() {
    if (!Failed) {
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

void ServerUI::Sever_ReceiveComplete(const QByteArray &Data, SocketThread *Socket) {
    if (Data.isEmpty()) {
        qWarning() << "Bad Data";
        Log_println(tr("Transmission error. Please try again"));
        return;
    }
    const char *pData = Data.data();
    int DataSize = ToInt(pData, 0);
    int flag = ToInt(pData, 1);
    qDebug() << "Size:" << DataSize << "ID:" << flag;
    if (DataSize != Data.size() - (int) sizeof(int)) {
        qWarning() << "Size error" << DataSize + sizeof(int) << Data.size();
        return;
    }
    switch (flag) {
        case Flag_Label_IRP: {
            Send_Labels(Socket);
            Log_println("A get label request from " + Socket->getPeerAddress().toString() + " was received");
//                       QHostAddress2c_str(Socket->getPeerAddress()));
            break;
        }
        case Flag_Image_IRP: {
            int len = ToInt(pData, 2);
            QString name(Data.mid(sizeof(int) * 3, len));
            Send_Image(Socket, name);
            Log_println("Received a request from " + Socket->getPeerAddress().toString() + " to get a picture " + name);
            break;
        }
        case Flag_Image_BandBoxs_IRP: {
            int len = ToInt(pData, 2);
            QString name(Data.mid(sizeof(int) * 3, len));
            Send_Image_BandBoxs(Socket, name);
            Log_println("Received a Bandbox request from " + Socket->getPeerAddress().toString() + " to get " + name);
            break;
        }
        case Flag_Image_List_IRP: {
            Send_Image_List(Socket);
            Log_println("Received a request for a list of images from " + Socket->getPeerAddress().toString());
            break;
        }
        case Flag_Image_BandBoxs_FB: {
            QString filename = Receive_Image_BandBox(Data);
            Log_println("Received " + filename + " BandBox, saved");
            break;
        }
    }
}

void ServerUI::on_pushButton_clicked() {
    QList<SocketThread *> ClientSocketList = Server->getClientSocketList();

    for (SocketThread *ClientSocket : ClientSocketList) {
        if (clientVector.indexOf(ClientSocket->getPeerAddress()) == -1)
            clientVector.push_back(ClientSocket->getPeerAddress());
    }

    qDebug() << clientVector;

    if (clientVector.size() == 0) {
        QMessageBox::warning(this, tr("error"), tr("No client connection"));
        return;
    }

    ui->pushButton->setDisabled(true);

    Log_printf("Stopped accessing,%d is accessed\nAssign tasks...\n", clientVector.size());
    allocation(pMainWindow->Project.no_label_Img());

    Log_printf("Assigned to complete\nThe total number of %d\n", pMainWindow->Project.all_Img().size());
    Log_printf("Server:%d\n", TaskList.front().size());
    for (int i = 0; i < Server->getClientSocketList().size(); ++i) {
        SocketThread *socketThread = Server->getClientSocketList()[i];
        int index = clientVector.indexOf(socketThread->getPeerAddress());
        Log_println(socketThread->getPeerAddress().toString() + ':' + QString::number(TaskList[index + 1].size()));
        SendSingle_Ready(socketThread);
    }

    disconnect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
               pMainWindow, SLOT(comboBox_currentIndexChanged(int)));

    connect(pMainWindow->ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(Proxy_comboBox_currentIndexChanged(int)));
}

void ServerUI::Sever_TCPNewConnection(SocketThread *Socket) {
    int count = 0;
    for (SocketThread *ClientSocket : Server->getClientSocketList()) {
        if (ClientSocket->getPeerAddress() == Socket->getPeerAddress()) {
            count++;
        }
    }
    /* 发现此IP的链接大于1，断开此链接 */
    if (count > 1) {
        Server->disconnectedrelay(Socket);
        return;
    }
    /* 如果已注册的客户端IP非空，检查新的链接是不是已注册 */
    if (!clientVector.empty()) {
        if (clientVector.indexOf(Socket->getPeerAddress()) >= 0) {
            /* 是已注册的IP，发生开始信号 */
            Log_println(Socket->getPeerAddress().toString() + " To access");
            Log_println("Start signal");
            SendSingle_Ready(Socket);
        } else return;
    } else {
        /* 没有已注册的IP，说明未开始，链接待命 */
        Log_println(Socket->getPeerAddress().toString() + " access");
        QStringList Addresslist;
        for (const SocketThread *i : Server->getClientSocketList())
            Addresslist.push_back(i->getPeerAddress().toString() + ' ' + i->getpeerName());
        ui->listWidget->clear();
        ui->listWidget->addItems(Addresslist);
        ui->listWidget->sortItems();
    }
}

void ServerUI::Sever_TCPDisconnected(const QHostAddress &Address) {
    /* 刷新链接列表 */
    QList<QString> Addresslist;
    for (const SocketThread *i : Server->getClientSocketList())
        Addresslist.push_back(i->getPeerAddress().toString() + ' ' + i->getpeerName());
    ui->listWidget->clear();
    ui->listWidget->addItems(Addresslist);
    ui->listWidget->sortItems();
    Log_println(Address.toString() + " disconnect");
}

void ServerUI::Sever_acceptError(QAbstractSocket::SocketError socketError) {
    qWarning() << "on_HostSever_acceptError" << socketError;
}

void ServerUI::Proxy_comboBox_currentIndexChanged(int index) {
    if (TaskList.isEmpty())
        return;

    QStringList Itmes;
    switch (index) {
        case all: {
            for (const QString &i : TaskList.front())
                Itmes.push_back(i);
            break;
        }
        case Marked: {
            for (const QString &i : TaskList.front()) {
                int Index = pMainWindow->Project.findImage(i);
                if (pMainWindow->Project.Images[Index].isHasLabel())
                    Itmes.push_back(i);
            }
            break;
        }
        case NoMarked: {
            for (const QString &i : TaskList.front()) {
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
    int div = Img.size() / (clientVector.size() + 1);
    QVector<QString> Task;
    for (int n = 0; n < clientVector.size(); ++n) {
        for (int i = 0; i < div; ++i) {
            Task.push_back(Img[div * n + i].getImageFilename());
        }
        TaskList.push_back(Task);
        Task.clear();
    }
    for (int i = div * clientVector.size(); i < Img.size(); ++i) {
        Task.push_back(Img[i].getImageFilename());
    }
    TaskList.push_back(Task);
    Proxy_comboBox_currentIndexChanged(all);
}

void ServerUI::SendSingle_Ready(SocketThread *Socket) {
    /*int 0x04, int 0x01 */
    int buff[] = {sizeof(int), Flag_Ready};
    Socket->Transmit(QByteArray(ToCharp(buff), sizeof(buff)));
}

void ServerUI::Send_Labels(SocketThread *Socket) {
    /*int size, int flag = Flag_Label_IRP, int count, <int len, char[len] label> * count */
    QVector<QString> labels = pMainWindow->Project.labels.getLabels();
    int count = labels.size();
    QByteArray data(ToCharp(&count), sizeof(int));
    for (const QString &i : labels) {
        int Len = i.length();
        data.push_back(QByteArray(ToCharp(&Len), sizeof(int)));
        data.push_back(i.toLocal8Bit());
    }

    int flag = Flag_Label_IRP;
    data.push_front(QByteArray(ToCharp(&flag), sizeof(int)));
    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Socket->Transmit(data);
}

void ServerUI::Send_Image(SocketThread *Socket, const QString &name) {
    /*int size, int flag = Flag_Image_IRP, int ImageSize, char[ImageSize] imgRAW*/
    int Index = pMainWindow->Project.findImage(name);
    QString ImagePath = pMainWindow->Project.all_Img()[Index].getImagePath();
    QString ImageFilename = pMainWindow->Project.all_Img()[Index].getImageFilename();
    QFile file;
    file.setFileName(ImagePath + '/' + ImageFilename);
    file.open(QFile::ReadOnly);
    QByteArray imgdata = file.readAll();
    file.close();

    int head[] = {
            Flag_Image_IRP,
            imgdata.size()};
    QByteArray data(ToCharp(head), sizeof(head));
    data.push_back(imgdata);

    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    qDebug() << size << head[1];
    Socket->Transmit(data);
}

void ServerUI::Send_Image_List(SocketThread *Socket) {
    /*int size, int imgCount, <int len, char[len] filename> * imgCount */
    int index = clientVector.indexOf(Socket->getPeerAddress());
    QVector<QString> imgs = TaskList[index + 1];
    int imgCount = imgs.size();
    QByteArray data(ToCharp(&imgCount), sizeof(int));

    for (const QString &i : imgs) {
        int len = i.length();
        data.push_back(QByteArray(ToCharp(&len), sizeof(int)));
        data.push_back(i.toLocal8Bit());
    }

    int flag = Flag_Image_List_IRP;
    data.push_front(QByteArray(ToCharp(&flag), sizeof(int)));
    int size = data.size();
    data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
    Socket->Transmit(data);
    Send_Image_HasLabel(Socket);
}

void ServerUI::Send_Image_BandBoxs(SocketThread *Socket, const QString &name) {
    /*int size, int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    int Index = pMainWindow->Project.findImage(name);
    QVector<BandBox> BandBoxs = pMainWindow->Project.all_Img()[Index].getBandBoxs();

    int BandBoxsCount = BandBoxs.size();
    QByteArray data(ToCharp(&BandBoxsCount), sizeof(int));
    for (const BandBox &i : BandBoxs) {
        int buff[5] = {i.ID,
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
    Socket->Transmit(data);
}

const QString ServerUI::Receive_Image_BandBox(const QByteArray &array) {
    /*int size, int flag = Flag_Image_BandBoxs_FB, int w, int h, int len, char[len] filename, int BandBoxsCount, <int ID, int x, int y, int w, int h> * BandBoxsCount */
    QByteArray DataCopy(array);
    const char *pData = DataCopy.constData();
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
    for (int i = 0; i < BandBoxsCount; ++i) {
        const int *p = (const int *) DataCopy.constData();
        QString Label = pMainWindow->Project.labels[p[0]];
        bandBoxs.push_back(BandBox(
                QRect(p[1], p[2], p[3], p[4]),
                Label, p[0]));
        DataCopy.remove(0, sizeof(int) * 5);
    }
    int Index = pMainWindow->Project.findImage(filename);
    if (Index >= 0) {
        qDebug() << w << h;
        pMainWindow->Project.Images[Index].setsize(QSize(w, h));
        pMainWindow->Project.Images[Index].setBandBoxs(bandBoxs);
        pMainWindow->Project.Images[Index].saveXml();
    } else
        qWarning("Receive_Image_BandBox error");
    return filename;
}

const char *ServerUI::QHostAddress2c_str(const QHostAddress &Address) {
    return Address.toString().toLocal8Bit().data();
}

void ServerUI::Send_Image_HasLabel(SocketThread *Socket) {
    /*int size, int flag = Flag_Image_haslabel, int len, char[len] haslabel*/
    if (!Server->getClientSocketList().isEmpty()) {
        int index = clientVector.indexOf(Socket->getPeerAddress());
        QVector<QString> imagelist = TaskList[index + 1];
        QByteArray data;
        int head[] = {Flag_Image_haslabel, imagelist.size()};
        data.push_back(QByteArray(ToCharp(&head), sizeof(head)));
        for (const QString &filename : imagelist) {
            int i = pMainWindow->Project.findImage(filename);
            char haslabel = pMainWindow->Project.Images[i].isHasLabel();
            data.push_back(haslabel);
        }
        int size = data.size();
        data.push_front(QByteArray(ToCharp(&size), sizeof(int)));
        Socket->Transmit(data);
    }
}
