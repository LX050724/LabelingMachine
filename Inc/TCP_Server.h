#ifndef LABELINGMACHINESEVER_H
#define LABELINGMACHINESEVER_H

#include <QObject>
#include <QMap>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

class SocketThread : public QThread {
Q_OBJECT
signals:

    void ReceiveData(const QByteArray, SocketThread *);

    void Disconnected(SocketThread *);

protected slots:

    void Scoket_disconnected();

protected:
    qintptr handle;
    QTcpSocket *Socket = nullptr;
    QHostAddress peerAddress;

protected:
    QString peerName;
    QByteArray Data;
    QList<QByteArray> TransmitData;
    volatile bool STOP = false;
    volatile bool Ready = false;

    void run() override;

public:
    explicit SocketThread(QObject *parent = nullptr, qintptr _handle = 0);

    ~SocketThread() override;

    const QHostAddress &getPeerAddress() const;

    const QString &getpeerName() const;

    QTcpSocket *getTCPSocket() const;

    bool isReady() const;

    void Transmit(const QByteArray &Data);
};

class TCP_Server : public QTcpServer {
Q_OBJECT
    QList<SocketThread *> ClientList;
    QString MachineName;

signals:

    void ReceiveComplete(const QByteArray &, SocketThread *);

    void TCPNewConnection(SocketThread *);

    void TCPDisconnected(const QHostAddress &);

public:
    explicit TCP_Server(QObject *parent = nullptr);

    ~TCP_Server() override;

    bool startListing();

    inline int getCilentConut() { return ClientList.size(); }

    inline const QList<SocketThread *> getClientSocketList() { return ClientList; }

protected:
    void incomingConnection(qintptr handle) override;

private slots:

    void relay(const QByteArray Data, SocketThread *Socket);

public slots:
    void disconnectedrelay(SocketThread *Link);
};

#endif // LABELINGMACHINESEVER_H
