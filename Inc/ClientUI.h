#ifndef CONNECTTOSEVER_H
#define CONNECTTOSEVER_H

#include <QWidget>
#include <QtWidgets/QListWidgetItem>
#include <QTimer>
#include <RCS_Client.h>
#include "bandbox.h"
#include "imagedata.h"

namespace Ui {
    class ClientUI;
}

class MainWindow;

class ClientUI : public QWidget {
Q_OBJECT
    enum {
        all, Marked, NoMarked
    };

    spdlogger logger;
    RCS_Client *rcsClient = nullptr;
    MainWindow *pMainWindow = nullptr;

    QString filename_now;
    QVector<BandBox> bandBoxs;
    QByteArray ImageRAWData;
    QImage *Image = nullptr;
    ImageData Data;
    volatile bool loadcomplete = true;
    volatile bool ready = false;

    QMap<QString, bool> ImageList;

    QTimer TimeoutTimer;

public:
    explicit ClientUI(QWidget *parent = nullptr, MainWindow *pmainwindow = nullptr);

    ~ClientUI() override;

private:
    Ui::ClientUI *ui;

    void singleProxy();

    void loadimg();

    void Send_Image_BandBox();

    void Receive_Image(const QString &from, const QJsonObject &obj);

    void Receive_Image_List(const QString &from, const QJsonObject &obj);

    void Recrive_Image_BandBoxs(const QString &from, const QJsonObject &obj);

    void Log_println(const QString &_Log);

    void Log_printf(const char *format, ...);

private slots:
    void BROADCAST(const QString &from, const QString &broadcastName, const QJsonObject &data);

    void RETURN(TcpConnect::PACK_TYPE type, const QJsonObject &info);

    void Client_Disconnected(const QString &);

    void Proxy_nextimg();

    void Proxy_lastimg();

    void Proxy_Keypress(int Key);

    void Proxy_imgs_listWidget_itemClicked(QListWidgetItem *item);

    void Proxy_comboBox_currentIndexChanged(int index);

    void Proxy_save_triggered();

    void Timer_timeout();

    void on_connectButton_clicked();
};

#endif // CONNECTTOSEVER_H
