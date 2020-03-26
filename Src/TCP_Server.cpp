#include "TCP_Server.h"
#include <QHostInfo>
#include <QNetworkInterface>

TCP_Server::TCP_Server(QObject *parent) :
    QTcpServer(parent)
{
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
    ClientList.insertMulti(thread->getPeerAddress(), thread);
    qInfo() << "connected" << ClientList.size() << "Deivce(s)";
    emit TCPNewConnection(thread->getPeerAddress());
}

void TCP_Server::disconnectedrelay(SocketThread* Link) {
    QHostAddress IP = Link->getPeerAddress();
    qInfo() << "disconnectedrelay IP =" << IP.toString();

    QObject::disconnect(Link, &SocketThread::ReceiveData,
                        this, &TCP_Server::relay);
    QObject::disconnect(Link, &SocketThread::Disconnected,
                        this, &TCP_Server::disconnectedrelay);

    ClientList.remove(Link->getPeerAddress());

    delete Link;
    qInfo() << "connected" << ClientList.size() << "Deivce(s)";
    emit TCPDisconnected(IP);
}

TCP_Server::~TCP_Server() {
    for(SocketThread* i : ClientList)
        delete i;
}

bool TCP_Server::startListing() {
    if(!this->listen(QHostAddress::Any, 8848))
        return false;
    MachineName = QHostInfo::localHostName();
    qInfo() << "MachineName" << MachineName;
    return true;
}

void TCP_Server::relay(const QByteArray Data, const QHostAddress& Address) {
    emit ReceiveComplete(Data, Address);
}

const QList<QHostAddress> TCP_Server::getClientAddressList() {
    QList<QHostAddress> tmp;
    for(myQHostAddress i : ClientList.keys())
        tmp.push_back(QHostAddress(i));
    return tmp;
}

bool TCP_Server::myQHostAddress::operator >(const TCP_Server::myQHostAddress &a) const {
    return this->toIPv4Address() > a.toIPv4Address();
}

bool TCP_Server::myQHostAddress::operator <(const TCP_Server::myQHostAddress &a) const {
    return this->toIPv4Address() < a.toIPv4Address();
}

bool TCP_Server::myQHostAddress::operator >=(const TCP_Server::myQHostAddress &a) const {
    return this->toIPv4Address() >= a.toIPv4Address();
}

bool TCP_Server::myQHostAddress::operator <=(const TCP_Server::myQHostAddress &a) const {
    return this->toIPv4Address() <= a.toIPv4Address();
}

void SocketThread::Scoket_disconnected() {
    emit Disconnected(this);
}

void SocketThread::run() {
    Socket = new QTcpSocket();
    Socket->setSocketDescriptor(handle);
    peerAddress = Socket->peerAddress();
    peerName = Socket->peerName();
    QObject::connect(Socket, SIGNAL(disconnected()), this, SLOT(Scoket_disconnected()));
    qInfo() << "SocketThreadID:" << SocketThread::currentThreadId();
    qInfo() << "peerAddress:" << peerAddress;
    qInfo() << "peerName:" << peerName;

    Ready = true;

    while (true){
        while (!Socket->waitForReadyRead(10)) {
            if (!TransmitData.isEmpty()) {
                Socket->write(TransmitData.front());
                if(!Socket->waitForBytesWritten())
                    throw Socket;
                TransmitData.pop_front();
            }
            if(STOP) return;
        }

        Data = Socket->readAll();
        const char *p = Data.constData();
        int Size = ((const int *)p)[0] + (int)sizeof(int);
        while (Size != Data.size()) {
            if(Size < 0){
                Data.clear();
                break;
            }
            qDebug() << "incomplete" << Size << Data.size();
            if(Socket->waitForReadyRead(50))
                Data.push_back(Socket->readAll());
            else break;
        }
        emit ReceiveData(Data, peerAddress);
    }
}

SocketThread::SocketThread(QObject *parent, qintptr _handle) :
    QThread(parent)
{
    if(_handle == 0)
        throw _handle;
    this->handle = _handle;
}

SocketThread::~SocketThread() {
    if(this->isRunning()) {
        this->requestInterruption();
        this->quit();
        STOP = true;
        this->wait();
    }
    if(Socket) {
        QObject::disconnect(Socket, SIGNAL(disconnected()),
                            this, SLOT(Scoket_disconnected()));
        delete Socket;
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

void SocketThread::Transmit(const QByteArray & TData){
    TransmitData.push_back(TData);
}

const QHostAddress &SocketThread::getPeerAddress() const {
    return peerAddress;
}
