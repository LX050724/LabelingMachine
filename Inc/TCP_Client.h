#ifndef LABELINGMACHINECMAKE_TCP_CLIENT_H
#define LABELINGMACHINECMAKE_TCP_CLIENT_H

#include <QThread>
#include <QTcpSocket>
#include <QDebug>
#include <QtNetwork/QHostAddress>

class TCP_Client : public QThread {
Q_OBJECT

    QTcpSocket *Socket = nullptr;
    QHostAddress Address;
    QByteArray Data;
    QList<QByteArray> TransmitData;
    volatile bool STOP = false;
    volatile bool Failed = false;
    volatile bool Ready = false;
public:
    inline bool isReady() const { return Ready; }

signals:

    void ReceiveData(const QByteArray &);

    void Disconnected();

public:
    explicit TCP_Client(QObject *parent = nullptr) : QThread(parent) {}

    ~TCP_Client() override;

    bool Connect(const QHostAddress &address);

    void Disconnect();

    void Transmit(const QByteArray &_data);

private:
    void run() override;

private slots:

    void Socket_disconnected();
};


#endif //LABELINGMACHINECMAKE_TCP_CLIENT_H
