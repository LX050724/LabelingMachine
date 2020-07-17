#include "TCP_Server.h"
#include <QHostInfo>
#include <QtWidgets/QMessageBox>
#include <Inc/publicdefine.h>

TCP_Server::TCP_Server(QObject *parent) :
        QTcpServer(parent) {
    qRegisterMetaType<QHostAddress>("QHostAddress");
}

void TCP_Server::incomingConnection(qintptr handle) {
    auto thread = new SocketThread(this, handle);
    QObject::connect(thread, &SocketThread::ReceiveData,
                     this, &TCP_Server::relay,
                     Qt::QueuedConnection);
    QObject::connect(thread, &SocketThread::Disconnected,
                     this, &TCP_Server::disconnectedrelay,
                     Qt::QueuedConnection);
    thread->start();
    while (!thread->isReady());
    ClientList.push_back(thread);
    qInfo() << "connected" << ClientList.size() << "Deivce(s)";
    emit TCPNewConnection(thread);
}

void TCP_Server::disconnectedrelay(SocketThread *Link) {
    QHostAddress IP = Link->getPeerAddress();
    qInfo() << "disconnectedrelay IP = " << IP.toString();

    QObject::disconnect(Link, &SocketThread::ReceiveData,
                        this, &TCP_Server::relay);
    QObject::disconnect(Link, &SocketThread::Disconnected,
                        this, &TCP_Server::disconnectedrelay);

    ClientList.removeOne(Link);

    delete Link;
    qInfo() << "connected" << ClientList.size() << "Deivce(s)";
    emit TCPDisconnected(IP);
}

TCP_Server::~TCP_Server() {
    for (SocketThread *i : ClientList) {
        QObject::disconnect(i, &SocketThread::ReceiveData,
                            this, &TCP_Server::relay);
        QObject::disconnect(i, &SocketThread::Disconnected,
                            this, &TCP_Server::disconnectedrelay);
        delete i;
    }
}

bool TCP_Server::startListing() {
    if (!this->listen(QHostAddress::Any, TCP_PORT))
        return false;
    MachineName = QHostInfo::localHostName();
    qInfo() << "MachineName" << MachineName;
    return true;
}

void TCP_Server::relay(const QByteArray Data, SocketThread *Socket) {
    emit ReceiveComplete(Data, Socket);
}

void SocketThread::run() {
    Socket = new QTcpSocket();
    Socket->setSocketDescriptor(handle);
    peerAddress = Socket->peerAddress();
    peerName = Socket->peerName();
    QObject::connect(Socket, SIGNAL(disconnected()), this, SLOT(Scoket_disconnected()),
                     Qt::QueuedConnection);
    qInfo() << "SocketThreadID:" << SocketThread::currentThreadId();
    qInfo() << "peerAddress:" << peerAddress;
    qInfo() << "peerName:" << peerName;

    Ready = true;

    while (true) {
        while (!Socket->waitForReadyRead(10)) {
            if (!TransmitData.isEmpty()) {
                Socket->write(TransmitData.front());
                if (!Socket->waitForBytesWritten())
                    throw Socket;
                TransmitData.pop_front();
            }
            if (STOP) {
                QObject::disconnect(Socket, SIGNAL(disconnected()),
                                    this, SLOT(Scoket_disconnected()));
                delete Socket;
                return;
            }
        }

        Data = Socket->readAll();
        const char *p = Data.constData();
        int Size = ((const int *) p)[0] + (int) sizeof(int);
        while (Size != Data.size()) {
            if (Size < 0) {
                Data.clear();
                break;
            }
            qDebug() << "incomplete" << Size << Data.size();
            if (Socket->waitForReadyRead(50))
                Data.push_back(Socket->readAll());
            else break;
        }
        emit ReceiveData(Data, this);
    }
}

void SocketThread::Scoket_disconnected() {
    emit Disconnected(this);
}

SocketThread::SocketThread(QObject *parent, qintptr _handle) :
        QThread(parent) {
    if (_handle == 0)
        throw _handle;
    this->handle = _handle;
}

SocketThread::~SocketThread() {
    if (this->isRunning()) {
        this->requestInterruption();
        this->quit();
        STOP = true;
        this->wait();
    }
}

const QString &SocketThread::getpeerName() const {
    return peerName;
}

QTcpSocket *SocketThread::getTCPSocket() const {
    return Socket;
}

bool SocketThread::isReady() const {
    return Ready;
}

void SocketThread::Transmit(const QByteArray &TData) {
    TransmitData.push_back(TData);
}

const QHostAddress &SocketThread::getPeerAddress() const {
    return peerAddress;
}
