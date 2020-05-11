#include "TCP_Client.h"

void TCP_Client::run() {
    Socket = new QTcpSocket;
    Socket->connectToHost(Address, 8848);
    if (!Socket->waitForConnected(5000)) {
        qInfo() << "ConnectFailed";
        delete Socket;
        Failed = true;
        this->requestInterruption();
        this->quit();
        return;
    }
    QObject::connect(Socket, &QTcpSocket::disconnected,
                     this, &TCP_Client::Socket_disconnected);

    Ready = true;
    while (true) {
        while (!Socket->waitForReadyRead(20)) {
            if (!TransmitData.isEmpty()) {
                Socket->write(TransmitData.front());
                if (!Socket->waitForBytesWritten())
                    throw Socket;
                TransmitData.pop_front();
            }
            if (STOP) {
                Socket->close();
                return;
            }
        }

        Data = Socket->readAll();
        const char *p = Data.constData();
        int Size = ((const int *) p)[0] + sizeof(int);
        while (Size != Data.size()) {
            if (Size < 0) {
                Data.clear();
                break;
            }
            qDebug() << "incomplete" << Size << Data.size();
            if (Socket->waitForReadyRead(100))
                Data.push_back(Socket->readAll());
            else break;
        }
        emit ReceiveData(Data);
    }
}

TCP_Client::~TCP_Client() {
    if (this->isRunning()) {
        this->requestInterruption();
        this->quit();
        STOP = true;
        this->wait();
    }
    if (Socket) {
        QObject::disconnect(Socket, &QTcpSocket::disconnected,
                            this, &TCP_Client::Socket_disconnected);
        delete Socket;
    }
}

void TCP_Client::Transmit(const QByteArray &_data) {
    TransmitData.push_back(_data);
}

void TCP_Client::Socket_disconnected() {
    qInfo() << "Socket_disconnected";
    Failed = false;
    this->requestInterruption();
    this->quit();
    STOP = true;
    this->wait();
    QObject::disconnect(Socket, &QTcpSocket::disconnected,
                        this, &TCP_Client::Socket_disconnected);
    emit disconnect();
}

bool TCP_Client::Connect(const QHostAddress &address) {
    if (address.isNull())
        return false;
    Address = address;
    Failed = false;
    Ready = false;
    STOP = false;
    this->start();
    while (!Ready)
        if (Failed) {
            this->wait();
            return false;
        }
    return true;
}

void TCP_Client::Disconnect() {
    if (this->isRunning()) {
        this->requestInterruption();
        this->quit();
        STOP = true;
        this->wait();
    }
    QObject::disconnect(Socket, &QTcpSocket::disconnected,
                        this, &TCP_Client::Socket_disconnected);
    Ready = Failed = STOP = false;
    delete Socket;
}
