#ifndef SERVERUI_H
#define SERVERUI_H

#include <QWidget>
#include <QtNetwork/QHostAddress>
#include <QtWidgets/QListWidgetItem>
#include <RCS_Server.h>
#include <RCS_Client.h>
#include <spdlogger.h>
#include "bandbox.h"
#include "imagedata.h"

namespace Ui {
    class ServerUI;
}

class MainWindow;

class ServerUI : public QWidget {
Q_OBJECT
    MainWindow *pMainWindow = nullptr;
    RCS_Server *rcsServer = nullptr;
    RCS_Client *rcsClient = nullptr;
    spdlogger logger;
    QMap<QString, QVector<QString>> TaskMap;
    volatile bool ready = false;

public:
    explicit ServerUI(QWidget *parent = nullptr, MainWindow *pmainwindow = nullptr);

    ~ServerUI();

private:
#define ToInt(p, Index) (((const int *)p)[Index])
#define ToCharp(p) ((const char *)p)

    enum {
        all, Marked, NoMarked
    };

    Ui::ServerUI *ui;

    void allocation(const QVector<ImageData> &Img);

    QJsonObject Send_Labels(const QString &name, const QJsonObject &info);

    QJsonObject Send_Image(const QString &name, const QJsonObject &info);

    QJsonObject Send_Image_List(const QString& name, const QJsonObject &info);

    void Receive_Image_BandBox(const QString &name, const QJsonObject &info);

    void Log_println(const QString &_Log);

    void Log_putc(char c);

    void Log_printf(const char *format, ...);

private slots:
    void Sever_TCPNewConnection(const QHostAddress &addr, const QString &name);

    void on_pushButton_clicked();

    void Proxy_comboBox_currentIndexChanged(int type);
};

#endif // SERVERUI_H
