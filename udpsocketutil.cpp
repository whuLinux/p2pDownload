#include "udpsocketutil.h"

UDPSocketUtil::UDPSocketUtil()
{

}

UDPSocketUtil::UDPSocketUtil(quint16 clientPort, QString serverIP, quint16 serverPort) : clientPort(clientPort), serverIP(serverIP), serverPort(serverPort)
{

}

UDPSocketUtil::~UDPSocketUtil()
{
    delete this->client;
    this->client = nullptr;
}

bool UDPSocketUtil::stablishClient()
{
    createSocket();

    if (!bindClientPort()) {
        qDebug() << "UDPSocketUtil::stablishClient " << "客户端无法监听端口" << endl;
        qDebug() << "UDPSocketUtil::stablishClient " << this->client->errorString() << endl;

        return false;
    }

    connect(this->client, SIGNAL(readyRead()), this, SLOT(recfromServer()));

    return true;
}

bool UDPSocketUtil::createSocket()
{
    this->client = new QUdpSocket();
    return true;
}

bool UDPSocketUtil::bindClientPort()
{
    if (!this->client->bind(QHostAddress::AnyIPv4, this->clientPort)) {
        qDebug() << "UDPSocketUtil::bindClientPort " << "客户端无法监听端口" << endl;
        qDebug() << "UDPSocketUtil::bindClientPort " << this->client->errorString() << endl;

        this->client->close();
        return false;
    }

    return true;
}

bool UDPSocketUtil::login(CtrlMsg & msg)
{
    qDebug()<<"UDPSocketUtil::login?  "<<this->serverIP<<this->serverPort;
    if (msg.getMsgType() == UDPCtrlMsgType::LOGIN) {
        qDebug()<<"UDPSocketUtil::login  "<<this->serverIP<<this->serverPort;
        qDebug()<<msg.toMsg()<<endl;
        qDebug()<<msg.toMsg().size()<<endl;

        this->client->writeDatagram(msg.toMsg(), QHostAddress(this->serverIP), this->serverPort);
        return true;
    }

    return false;
}

bool UDPSocketUtil::logout(CtrlMsg & msg)
{
    if (msg.getMsgType() == UDPCtrlMsgType::LOGOUT) {
        this->client->writeDatagram(msg.toMsg(), QHostAddress(this->serverIP), this->serverPort);
        return true;
    }

    return false;
}

bool UDPSocketUtil::obtainAllPartners(CtrlMsg & msg)
{
    if (msg.getMsgType() == UDPCtrlMsgType::OBTAINALLPARTNERS) {
        this->client->writeDatagram(msg.toMsg(), QHostAddress(this->serverIP), this->serverPort);
        return true;
    }

    return false;
}

bool UDPSocketUtil::p2pTrans(CtrlMsg & msg)
{
    if (msg.getMsgType() == UDPCtrlMsgType::P2PTRANS) {
        this->client->writeDatagram(msg.toMsg(), QHostAddress(this->serverIP), this->serverPort);
        return true;
    }

    return false;
}

bool UDPSocketUtil::recfromServer()
{
    while (this->client->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(this->client->pendingDatagramSize()));

        this->client->readDatagram(datagram.data(), datagram.size());

        QJsonObject jsonMsg;
        jsonMsg = QJsonDocument::fromJson(datagram).object();

        if (jsonMsg.value(MSGTYPE).isUndefined()) {
            qDebug() << "UDPSocketUtil::recfromServer " << "无法解析服务器端发来的消息类型" << endl;
            return false;
        }

        if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::RENAME)) {
            return rename();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::LOGINSUCCESS)) {
            return loginSuccess();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::LOGINFAILURE)) {
            return loginFailure();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::LOGOUTSUCCESS)) {
            return logoutSuccess();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::LOGOUTFAILURE)) {
            return logoutFailure();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::RETURNALLPARTNERS)) {
            return receiveAllPartners(jsonMsg);
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::OBTAINSUCCESS)) {
            return obtainSuccess();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::OBTAINFAILURE)) {
            return obtainFailure();
        } else if (jsonMsg.value(MSGTYPE).toInt() == qint8(UDPCtrlMsgType::P2PNEEDHOLE)) {
            return p2pNeedHole(jsonMsg);
        }   
    }

    return false;
}

bool UDPSocketUtil::rename()
{
    emit renameNow();
    return true;
}

bool UDPSocketUtil::loginSuccess()
{
    emit loginOk();
    return true;
}

bool UDPSocketUtil::loginFailure()
{
    emit loginAgain();
    return true;
}

bool UDPSocketUtil::logoutSuccess()
{
    emit logoutOk();
    return true;
}

bool UDPSocketUtil::logoutFailure()
{
    emit logoutAgain();
    return true;
}

bool UDPSocketUtil::obtainSuccess()
{
    emit obtainOk();
    return true;
}

bool UDPSocketUtil::obtainFailure()
{
    emit obtainAgain();
    return true;
}

bool UDPSocketUtil::p2pNeedHole(QJsonObject & jsonMsg)
{
    if (jsonMsg.value(FRIEND).isUndefined()) {
        qDebug() << "UDPSocketUtil::p2pNeedHole " << "服务器端发来的消息不完整" << endl;
        return false;
    }

    QJsonObject client = jsonMsg.value(FRIEND).toObject();

    if (client.value(PARTNERNAME).isUndefined() || client.value(IP).isUndefined() || client.value(PORT).isUndefined()) {
        qDebug() << "UDPSocketUtil::p2pNeedHole " << "服务器端发来的消息不完整" << endl;
        return false;
    }

    emit p2pHoleRequestFromServer(client.value(PARTNERNAME).toString(), client.value(IP).toString(), quint16(client.value(PORT).toInt()));
    return true;
}

bool UDPSocketUtil::receiveAllPartners(QJsonObject & jsonMsg)
{
    if (jsonMsg.value(PARTNERVECTOR).isUndefined()) {
        qDebug() << "UDPSocketUtil::receiveAllPartners " << "服务器端发来的消息不完整" << endl;
        return false;
    }

    QJsonArray jsonClients = jsonMsg.value(PARTNERVECTOR).toArray();
    int len = jsonClients.size();

    for (int i = 0; i < len; i++) {
        QJsonObject jsonClient = jsonClients.at(i).toObject();

        if (jsonClient.value(PARTNERNAME).isUndefined() || jsonClient.value(IP).isUndefined() || jsonClient.value(PORT).isUndefined()) {
            qDebug() << "UDPSocketUtil::receiveAllPartners " << "服务器端发来的消息不完整" << endl;
            continue;
        }

        ClientNode client;
        client.name = jsonClient.value(PARTNERNAME).toString();
        client.ip = jsonClient.value(IP).toString();
        client.port = quint16(jsonClient.value(PORT).toInt());

        this->partners.append(client);
    }

    emit timeToGetAllPartners();
    return this->partners.empty();
}
