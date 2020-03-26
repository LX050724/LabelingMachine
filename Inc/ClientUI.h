#ifndef CONNECTTOSEVER_H
#define CONNECTTOSEVER_H

#include <QWidget>
#include <QtWidgets/QListWidgetItem>
#include <QTimer>
#include "TCP_Client.h"
#include "bandbox.h"
#include "imagedata.h"

namespace Ui {
class ClientUI;
}

class MainWindow;

class ClientUI : public QWidget
{
    Q_OBJECT
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

    TCP_Client* Client = nullptr;
    MainWindow* pMainWindow = nullptr;

    QString filename_now;
    QVector<BandBox> bandBoxs;
    QByteArray ImageRAWData;
    QImage *Image = nullptr;
    ImageData Data;
    volatile bool loadcomplete = true;

    QMap<QString, bool> ImageList;

    QTimer TimeoutTimer;

public:
    explicit ClientUI(QWidget *parent = nullptr, MainWindow *pmainwindow = nullptr);
    ~ClientUI() override;

private:
#define ToInt(p,Index) (((const int *)p)[Index])
#define ToCharp(p) ((const char *)p)

    Ui::ClientUI *ui;

    void singleProxy();
    void loadimg();

    void Send_IRP(IRP n);
    void Send_IRP(IRP n, const QString &filename);
    void Send_Image_BandBox();

    void Receive_Labels(const QByteArray &array);
    void Receive_Image(const QByteArray &array);
    void Receive_Image_List(const QByteArray &array);
    void Recrive_Image_BandBoxs(const QByteArray &array);
    void Receive_Image_HasLabel(const QByteArray &array);

    void Log_println(const QString &_Log);
    void Log_putc(char c);
    void Log_printf(const char *format, ...);

private slots:
    void on_pushButton_clicked();
    void Client_ReceiveComplete(const QByteArray& array);
    void Client_Disconnected();

    void Proxy_nextimg();
    void Proxy_lastimg();
    void Proxy_Keypress(int Key);
    void Proxy_imgs_listWidget_itemClicked(QListWidgetItem *item);
    void Proxy_comboBox_currentIndexChanged(int index);
    void Proxy_save_triggered();

    void Timer_timeout();
};

#endif // CONNECTTOSEVER_H
