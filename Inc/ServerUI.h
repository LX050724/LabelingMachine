#ifndef SERVERUI_H
#define SERVERUI_H

#include <QWidget>
#include <QtNetwork/QHostAddress>
#include <QtWidgets/QListWidgetItem>
#include "TCP_Server.h"
#include "bandbox.h"
#include "imagedata.h"

namespace Ui {
class ServerUI;
}

class MainWindow;

class ServerUI : public QWidget
{
    Q_OBJECT

    MainWindow* pMainWindow = nullptr;
    TCP_Server* Server = nullptr;
    int ClientConut = 0;
    volatile bool Ready = false;
    volatile bool Failed = false;

    QVector<QVector<QString>> TaskList;
    QList<QHostAddress> ClientList;

public:
    explicit ServerUI(QWidget *parent = nullptr, MainWindow* pmainwindow = nullptr);
    ~ServerUI();

    inline bool isReady() const { return Ready; }
    inline bool isFailed() const { return Failed; }

private:
#define ToInt(p,Index) (((const int *)p)[Index])
#define ToCharp(p) ((const char *)p)

    enum {all,Marked, NoMarked};

    typedef enum IRP{
        Flag_Ready = 1,
        Flag_Label_IRP,
        Flag_Image_IRP,
        Flag_Image_BandBoxs_IRP,
        Flag_Image_List_IRP,
        Flag_Image_BandBoxs_FB,
        Flag_Image_haslabel
    } IRP;
    Ui::ServerUI *ui;
    void allocation(const QVector<ImageData>& Img);

    void SendSingle_Ready(const QHostAddress& Address);
    void Send_Labels(const QHostAddress& Address);
    void Send_Image(const QHostAddress& Address, const QString &name);
    void Send_Image_List(const QHostAddress& Address);
    void Send_Image_BandBoxs(const QHostAddress& Address, const QString &name);
    void Send_Image_HasLabel(const QHostAddress& Address);
    const QString Receive_Image_BandBox(const QByteArray &array);

    void Log_println(const QString &_Log);
    void Log_putc(char c);
    void Log_printf(const char *format, ...);

    static const char * QHostAddress2c_str(const QHostAddress& Address);

private slots:
    void Sever_ReceiveComplete(const QByteArray& arrary, const QHostAddress& Address);
    void Sever_TCPNewConnection(const QHostAddress& Address);
    void Sever_TCPDisconnected(const QHostAddress& Address);
    void Sever_acceptError(QAbstractSocket::SocketError socketError);

    void on_pushButton_clicked();

//    void Proxy_nextimg();
//    void Proxy_lastimg();
//    void Proxy_Keypress(int Key);
//    void Proxy_deletBox();
//    void Proxy_drawBandBox(const BandBox & Box);
//    void Proxy_imgs_listWidget_itemClicked(QListWidgetItem *item);
//    void Proxy_class_tableWidget_cellClicked(int row, int column);
    void Proxy_comboBox_currentIndexChanged(int index);
};

#endif // SERVERUI_H
