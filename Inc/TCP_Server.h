#ifndef LABELINGMACHINESEVER_H
#define LABELINGMACHINESEVER_H

#include <QObject>
#include <QMap>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

class SocketThread : public QThread
{
    Q_OBJECT
signals:
    void ReceiveData(const QByteArray, const QHostAddress&);
    void Disconnected(SocketThread*);
protected slots:
    void Scoket_disconnected();

protected:
    qintptr handle;
    QTcpSocket* Socket = nullptr;
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
    const QString& getpeerName() const;
    QTcpSocket* getTCPSocket() const;
    bool isReady() const;
    void Transmit(const QByteArray& Data);
};

class TCP_Server : public QTcpServer
{
    Q_OBJECT
    class myQHostAddress : public QHostAddress
    {
    public:
        myQHostAddress()= default;
        myQHostAddress(QHostAddress addr) : QHostAddress(addr)
        {   }

        bool operator >(const myQHostAddress& a) const;
        bool operator <(const myQHostAddress& a) const;
        bool operator >=(const myQHostAddress& a) const;
        bool operator <=(const myQHostAddress& a) const;
        using QHostAddress::operator=;
        using QHostAddress::operator!=;
        using QHostAddress::operator==;
    };

    QMap<myQHostAddress, SocketThread*> ClientList;
    QString MachineName;

signals:
    void ReceiveComplete(const QByteArray&, const QHostAddress&);
    void TCPNewConnection(const QHostAddress&);
    void TCPDisconnected(const QHostAddress&);

public:
    explicit TCP_Server(QObject *parent = nullptr);
    ~TCP_Server() override;

    bool startListing();
    inline void SendTo(const QHostAddress& Address, const QByteArray& Data) {
        ClientList[Address]->Transmit(Data);
    }
    inline int getCilentConut() { return ClientList.size(); }
    const QList<QHostAddress> getClientAddressList();

protected:
    void incomingConnection(qintptr handle) override;

private slots:
    void relay(const QByteArray Data, const QHostAddress& Address);
    void disconnectedrelay(SocketThread* Link);
};

#endif // LABELINGMACHINESEVER_H
