#include "tcpsocketutil.h"

TCPSocketUtil::TCPSocketUtil()
{

}

TCPSocketUtil::TCPSocketUtil(quint16 hostPort, quint16 fileHostPort, bool openHost, bool openGuest, QString fileDir, QString fileType, qint32 groupSize) : hostPort(hostPort), fileHostPort(fileHostPort), openHost(openHost), openGuest(openGuest), fileDir(fileDir), fileType(fileType), sliceSize(groupSize)
{
    this->connectedNum = 0;
    this->fileConnectedNum = 0;
    this->friendName = 0;
}

TCPSocketUtil::~TCPSocketUtil()
{
    this->partnerFileIndex.clear();

    this->guestPort.clear();
    this->fileGuestPort.clear();

    closeHost();
    closeFileHost();

    for (QMap<qint32, Client *>::iterator clientsMapIt = this->clientsMap.begin(); clientsMapIt != this->clientsMap.end(); clientsMapIt++) {
        closeGuest(clientsMapIt.key());
        closeFileGuest(clientsMapIt.key());

        delete clientsMapIt.value();
        clientsMapIt.value() = nullptr;
    }

    this->guests.clear();
    this->p2pGuests.clear();
    this->fileGuests.clear();
    this->p2pFileGuests.clear();

    this->clientsMap.clear();
}

bool TCPSocketUtil::bindClients(QVector<Client *> clients)
{
    if (clients.empty()) {
        qDebug() << "TCPSocketUtil::bindClients " << "初始化伙伴客户端信息失败" << endl;
        return false;
    }

    int len = clients.size();
    for (int i = 0; i < len; i++) {
        addClient(clients.at(i));
    }

    return true;
}

bool TCPSocketUtil::addClient(Client * client)
{
    qDebug() << "TCPSocketUtil::addClient " << "伙伴客户端ID " << client->getId() << endl;

    if (this->clientsMap.contains(client->getId())) {
        qDebug() << "TCPSocketUtil::addClient " << "伙伴客户端不允许重复加入队列" << client->getId() << endl;
        return false;
    }

    this->clientsMap[client->getId()] = client;

    return true;
}

bool TCPSocketUtil::addGuest(qint32 id, quint16 port, quint16 filePort)
{    
    qDebug() << "TCPSocketUtil::addGuest " << "朋友客户端ID " << id << "port" << port << "filePort" << filePort << endl;

    if (!this->clientsMap.contains(id)) {
        qDebug() << "TCPSocketUtil::addGuest " << "陌生客户端不允许加入队列" << id << endl;
        return false;
    }

    this->guestPort[id] = port;
    this->fileGuestPort[id] = filePort;

    bool guestSucceedBind = stablishGuest(id);
    bool fileGuestSucceedBind = stablishFileGuest(id);

    if (!guestSucceedBind || !fileGuestSucceedBind) {
        qDebug() << "TCPSocketUtil::addGuest " << "无法添加新的朋友客户端" << endl;
        return false;
    }

    return true;
}

bool TCPSocketUtil::stablishHost()
{
    if (!this->openHost) {
        qDebug() << "TCPSocketUtil::stablishHost " << "未开启TCP服务器权限" << endl;
        return false;
    }

    createHost();

    qDebug() << this->hostPort << this->fileHostPort << endl;
    if (!listenPort()) {
        qDebug() << "TCPSocketUtil::stablishHost " << "端口侦听失败" << endl;
        qDebug() << "TCPSocketUtil::stablishHost " << this->host->errorString() << endl;

        return false;
    }

    connect(this->host, SIGNAL(newConnection()), this, SLOT(newConnectionWithPartner()));
    return true;
}

bool TCPSocketUtil::stablishGuest(qint32 friendId)
{
    if (!this->openGuest) {
        qDebug() << "TCPSocketUtil::stablishGuest " << "未开启TCP客户端权限" << endl;
        return false;
    }

    createGuest(friendId);

    if (!connectToFriend(friendId)) {
        qDebug() << "TCPSocketUtil::stablishGuest " << "连接朋友客户端失败 " << friendId << endl;

        closeGuest(friendId);
        return false;
    }

    return true;
}

bool TCPSocketUtil::stablishFileHost()
{
    if (!this->openHost) {
        qDebug() << "TCPSocketUtil::stablishFileHost " << "未开启TCP服务器权限" << endl;
        return false;
    }

    createFileHost();

    if (!listenFilePort()) {
        qDebug() << "TCPSocketUtil::stablishFileHost " << "端口侦听失败" << endl;
        qDebug() << "TCPSocketUtil::stablishFileHost " << this->host->errorString() << endl;

        return false;
    }

    connect(fileHost, SIGNAL(newConnection()), this, SLOT(newConnectionWithFilePartner()));
    return true;
}

bool TCPSocketUtil::stablishFileGuest(qint32 friendId)
{
    if (!this->openGuest) {
        qDebug() << "TCPSocketUtil::stablishFileGuest " << "未开启TCP客户端权限" << endl;
        return false;
    }

    createFileGuest(friendId);

    if (!connectToFileFriend(friendId)) {
        qDebug() << "TCPSocketUtil::stablishFileGuest " << "连接朋友客户端失败 " << friendId << endl;

        closeFileGuest(friendId);
        return false;
    }

    return true;
}

bool TCPSocketUtil::createHost()
{
    this->host = new QTcpServer();
    return true;
}

bool TCPSocketUtil::createGuest(qint32 friendId)
{
    if (this->guests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::createGuest " << "指定连接该朋友客户端的P2PTcpSocket对象此前已建立 " << friendId << endl;
        return false;
    }

    this->guests[friendId] = new QTcpSocket();
    this->p2pGuests[friendId] = new P2PTcpSocket();

    return true;
}

bool TCPSocketUtil::closeHost()
{
    for (QMap<qint32, QTcpSocket *>::iterator it = this->partnerConnections.begin(); it != this->partnerConnections.end(); it++) {
        it.value()->close();
        delete it.value();
        it.value() = nullptr;
    }

    for (QMap<qint32, P2PTcpSocket *>::iterator it = this->partnerP2PConnections.begin(); it != this->partnerP2PConnections.end(); it++) {
        delete it.value();
        it.value() = nullptr;
    }

    this->partnerConnections.clear();
    this->partnerP2PConnections.clear();

    this->host->close();
    delete this->host;

    return true;
}

bool TCPSocketUtil::closeGuest(qint32 friendId)
{
    if (!disConnectToFriend(friendId)) {
        qDebug() << "TCPSocketUtil::closeGuest " << "错误尝试关闭和朋友客户端之间不存在的TCP连接 " << "friendId " << friendId << endl;
        return false;
    }

    if (this->guests[friendId] != nullptr) {
        delete this->guests[friendId];
    }

    if (!this->p2pGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::closeGuest " << "错误尝试关闭和朋友客户端之间不存在的TCP连接的P2P辅助 " << "friendId " << friendId << endl;
        return false;
    }

    if (this->p2pGuests[friendId] != nullptr) {
        delete this->p2pGuests[friendId];
    }

    return true;
}

bool TCPSocketUtil::createFileHost()
{
    this->fileHost = new QTcpServer();
    return true;
}

bool TCPSocketUtil::createFileGuest(qint32 friendId)
{
    if (this->fileGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::createFileGuest " << "指定连接该朋友客户端的P2PTcpSocket对象此前已建立 " << friendId << endl;
        return false;
    }

    this->fileGuests[friendId] = new QTcpSocket();
    this->p2pFileGuests[friendId] = new P2PTcpSocket();

    return true;
}

bool TCPSocketUtil::closeFileHost()
{
    for (QMap<qint32, QTcpSocket *>::iterator it = this->partnerFileConnections.begin(); it != this->partnerFileConnections.end(); it++) {
        it.value()->close();
        delete it.value();
        it.value() = nullptr;
    }

    for (QMap<qint32, P2PTcpSocket *>::iterator it = this->partnerFileP2PConnections.begin(); it != this->partnerFileP2PConnections.end(); it++) {
        delete it.value();
        it.value() = nullptr;
    }

    this->partnerFileConnections.clear();
    this->partnerFileP2PConnections.clear();

    this->fileHost->close();
    delete this->fileHost;

    return true;
}

bool TCPSocketUtil::closeFileGuest(qint32 friendId)
{
    if (!disConnectToFileFriend(friendId)) {
        qDebug() << "TCPSocketUtil::closeFileGuest " << "错误尝试关闭和朋友客户端之间不存在的TCP连接 " << "friendId" << friendId << endl;
        return false;
    }

    if (this->fileGuests[friendId] != nullptr) {
        this->guests[friendId] = nullptr;
    }

    if (!this->p2pFileGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::closeFileGuest " << "错误尝试关闭和朋友客户端之间不存在的TCP连接的P2P辅助 " << "friendId" << friendId << endl;
        return false;
    }

    if (this->p2pFileGuests[friendId] != nullptr) {
        this->p2pGuests[friendId] = nullptr;
    }

    return true;
}

bool TCPSocketUtil::listenPort()
{
    if (!this->host->listen(QHostAddress::AnyIPv4, hostPort)) {
        qDebug() << "TCPSocketUtil::listenPort " << "端口侦听失败" << endl;
        qDebug() << "TCPSocketUtil::listenPort " << this->host->errorString() << endl;

        this->host->close();
        return false;
    }

    return true;
}

bool TCPSocketUtil::connectToFriend(qint32 friendId)
{
    if (!this->guests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::connectToFriend " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << "friendId" << friendId << endl;
        createGuest(friendId);
    }

    if (!this->clientsMap.contains(friendId)) {
        qDebug() << "TCPSocketUtil::connectToFriend " << "无法获取指定伙伴的ip地址 " << "friendId " << friendId << endl;
        return false;
    }

    if (!this->guestPort.contains(friendId)) {
        qDebug() << "TCPSocketUtil::connectToFriend " << "未指定侦听的端口 " << "friendId" << friendId << endl;
        return false;
    }

    qDebug() << "TCPSocketUtil::connectToFriend " << "friendId " << friendId << " guestPort " << this->guestPort[friendId] << endl;

    if (!this->guests[friendId]->bind(QHostAddress::AnyIPv4, this->guestPort[friendId])) {
        qDebug() << "TCPSocketUtil::connectToFriend " << "端口侦听失败 " << friendId << endl;
        qDebug() << "TCPSocketUtil::connectToFriend " << this->guests[friendId]->errorString() << endl;
        return false;
    }

    this->p2pGuests[friendId]->setId(friendId);
    this->guests[friendId]->connectToHost(this->clientsMap[friendId]->getIP(), this->clientsMap[friendId]->getPort());
    this->connectedNum++;

    connect(this->guests[friendId], SIGNAL(readyRead()), this->p2pGuests[friendId], SLOT(ensureReadyRead()));
    connect(this->guests[friendId], SIGNAL(error(QAbstractSocket::SocketError)), this->p2pGuests[friendId], SLOT(ensureError(QAbstractSocket::SocketError)));
    connect(this->guests[friendId], SIGNAL(disconnected()), this->p2pGuests[friendId], SLOT(ensureDisconnected()));

    connect(this->p2pGuests[friendId], SIGNAL(readyReadFromOthers(qint32)), this, SLOT(recFromFriend(qint32)));
    connect(this->p2pGuests[friendId], SIGNAL(socketErrorOfOthers(QAbstractSocket::SocketError, qint32)), this, SLOT(failToHelpFriend(QAbstractSocket::SocketError, qint32)));
    connect(this->p2pGuests[friendId], SIGNAL(disconnectedFromOthers(qint32)), this, SLOT(disConnectToFriend(qint32)));

    return true;
}

bool TCPSocketUtil::disConnectToFriend(qint32 friendId)
{
    if (!this->guests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::disConnectToFriend " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << "friendId " << friendId << endl;
        return false;
    }

    this->guests[friendId]->disconnectFromHost();

    if (!this->p2pGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::disConnectToFriend " << "指定连接该伙伴客户端的P2PTcpSocket对象的P2P辅助尚未建立 " << "friendId " << friendId << endl;
        return false;
    }

    this->p2pGuests[friendId]->setId(-1);

    return true;
}

bool TCPSocketUtil::listenFilePort()
{
    if (!this->fileHost->listen(QHostAddress::AnyIPv4, fileHostPort)) {
        qDebug() << "TCPSocketUtil::listenFilePort " << "端口侦听失败" << endl;
        qDebug() << "TCPSocketUtil::listenFilePort " << this->fileHost->errorString() << endl;
        this->fileHost->close();

        return false;
    }

    return true;
}


bool TCPSocketUtil::connectToFileFriend(qint32 friendId)
{
    if (!this->fileGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::connectToFileFriend " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << "friendId " << friendId << endl;
        createFileGuest(friendId);
    }

    if (!this->clientsMap.contains(friendId)) {
        qDebug() << "TCPSocketUtil::connectToFileFriend " << "无法获取指定伙伴的ip地址" << "friendId " << friendId << endl;
        return false;
    }

    if (!this->fileGuestPort.contains(friendId)) {
        qDebug() << "TCPSocketUtil::connectToFileFriend " << "未指定侦听的端口 " << "friendId " << friendId << endl;
        return false;
    }

    if (!this->fileGuests[friendId]->bind(QHostAddress::AnyIPv4, this->fileGuestPort[friendId])) {
        qDebug() << "TCPSocketUtil::connectToFileFriend " << "端口侦听失败 " << friendId << endl;
        qDebug() << "TCPSocketUtil::connectToFileFriend " << this->fileGuests[friendId]->errorString() << endl;

        this->fileGuests[friendId]->close();
        return false;
    }

    this->p2pFileGuests[friendId]->setId(friendId);
    this->fileGuests[friendId]->connectToHost(this->clientsMap[friendId]->getIP(), this->clientsMap[friendId]->getFilePort());
    this->fileConnectedNum++;

    connect(this->fileGuests[friendId], SIGNAL(readyRead()), this->p2pFileGuests[friendId], SLOT(ensureReadyRead()));
    connect(this->fileGuests[friendId], SIGNAL(error(QAbstractSocket::SocketError)), this->p2pFileGuests[friendId], SLOT(ensureError(QAbstractSocket::SocketError)));
    connect(this->fileGuests[friendId], SIGNAL(disconnected()), this->p2pFileGuests[friendId], SLOT(ensureDisconnected()));

    // connect(this->fileGuests[partnerId], SIGNAL(readyReadFromOthers(qint32)), this, SLOT(recFromFileFriend(qint32)));
    connect(this->p2pFileGuests[friendId], SIGNAL(socketErrorOfOthers(QAbstractSocket::SocketError, qint32)), this, SLOT(failToHelpFriend(QAbstractSocket::SocketError, qint32)));
    connect(this->p2pFileGuests[friendId], SIGNAL(disconnectedFromOthers(qint32)), this, SLOT(disConnectToFileFriend(qint32)));

    return true;
}

bool TCPSocketUtil::disConnectToFileFriend(qint32 friendId)
{
    if (!this->fileGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::disConnectToFileFriend " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << "friendId " << friendId << endl;
        return false;
    }

    this->fileGuests[friendId]->disconnectFromHost();

    if (!this->p2pFileGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::disConnectToFileFriend " << "指定连接该伙伴客户端的P2PTcpSocket对象的P2P辅助尚未建立 " << "friendId " << friendId << endl;
        return false;
    }

    this->p2pFileGuests[friendId]->setId(-1);

    return true;
}

bool TCPSocketUtil::newConnectionWithPartner()
{
    qDebug() << "TCPSocketUtil::newConnectionWithPartner" << endl;

    QTcpSocket * oringinVistor = this->host->nextPendingConnection();
    P2PTcpSocket * vistor = new P2PTcpSocket();

    QString partnerIP = oringinVistor->peerAddress().toString();
    quint16 partnerPort = oringinVistor->peerPort();

    qint32 vistorId = findIdFromClientsByIPAndPort(partnerIP, partnerPort);

    if (vistorId == -1 && !this->clientsMap.contains(vistorId)) {
        qDebug() << "TCPSocketUtil::newConnectionWithPartner " << "陌生客户端非法访问" << endl;
        return false;
    }

    if (this->partnerConnections.contains(vistorId) || this->partnerP2PConnections.contains(vistorId)) {
        qDebug() << "TCPSocketUtil::newConnectionWithPartner " << "伙伴客户端试图重复连接" << endl;
        return false;
    }

    this->partnerConnections[vistorId] = oringinVistor;

    this->partnerP2PConnections[vistorId] = vistor;
    this->partnerP2PConnections[vistorId]->setId(vistorId);

    connect(this->partnerConnections[vistorId], SIGNAL(readyRead()), this->partnerP2PConnections[vistorId], SLOT(ensureReadyRead()));
    connect(this->partnerConnections[vistorId], SIGNAL(error(QAbstractSocket::SocketError)), this->partnerP2PConnections[vistorId], SLOT(ensureError(QAbstractSocket::SocketError)));
    connect(this->partnerConnections[vistorId], SIGNAL(disconnected()), this->partnerP2PConnections[vistorId], SLOT(ensureDisconnected));

    connect(this->partnerP2PConnections[vistorId], SIGNAL(readyReadFromOthers(qint32)), this, SLOT(recFromPartner(qint32)));
    connect(this->partnerP2PConnections[vistorId], SIGNAL(socketErrorOfOthers(QAbstractSocket::SocketError, qint32)), this, SLOT(failToGetHelpFromPartner(QAbstractSocket::SocketError, qint32)));
    connect(this->partnerP2PConnections[vistorId], SIGNAL(disconnectedFromOthers(qint32)), this, SLOT(disConnectToPartner(qint32)));

    return true;
}

bool TCPSocketUtil::disConnectToPartner(qint32 partnerId)
{
    if (!this->partnerConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::disConnectToPartner " << "指定连接该伙伴客户端的P2PTcpSocket对象尚未建立 " << "partnerId " << partnerId << endl;
        return false;
    }

    this->partnerConnections[partnerId]->disconnectFromHost();

    if (!this->partnerP2PConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::disConnectToPartner " << "指定连接该伙伴客户端的P2PTcpSocket对象的P2P辅助尚未建立 " << "partnerId " << partnerId << endl;
        return false;
    }

    this->partnerP2PConnections[partnerId]->setId(-1);

    return true;
}

bool TCPSocketUtil::recFromPartner(qint32 partnerId)
{    
    if (!this->partnerConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::recFromPartner " << "P2PTcpSocket::readyReadFromPartner信号的partnerId错误 " << partnerId << endl;
        return false;
    }

    QByteArray msg = this->partnerConnections[partnerId]->readAll();
    QJsonObject jsonMsg = QJsonDocument::fromJson(msg).object();

    if (jsonMsg.value(MSGTYPE).isUndefined()) {
        qDebug() << "TCPSocketUtil::recFromPartner " << "无法解析伙伴客户端发来的消息类型" << endl;
        return false;
    }

    if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::P2PPUNCH) {
        // P2P打洞完成后，告知伙伴客户端下载地址
        emit timeToInitialTaskForPartner(partnerId);

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toDouble()) == TCPCtrlMsgType::ISALIVE) {
        // 伙伴客户端确认存活
        emit whetherToStopTask(partnerId, jsonMsg.value(RATE).toDouble());

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::AGREETOHELP) {
        // 伙伴客户端接受下载协助请求，准备为伙伴客户端分配下载任务
        emit timeForFirstTaskForPartner(partnerId);

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::REFUSETOHELP) {
        // 伙伴客户端拒绝下载协助请求，动态调整任务分配计划
        emit refuseToOfferHelpForPartner(partnerId);

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::TASKFINISH) {
        if (jsonMsg.value(TOKEN).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromPartner " << "伙伴客户端发来的消息不完整" << endl;
            return false;
        }

        qint32 token = qint32(jsonMsg.value(TOKEN).toInt());
        // 文件传送完成，准备在主机组装
        emit timeForNextTaskForPartner(partnerId, token);

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::TASKFAILURE) {
        if (jsonMsg.value(TOKEN).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromPartner " << "伙伴客户端发来的消息不完整" << endl;
            return false;
        }

        qint32 token = qint32(jsonMsg.value(TOKEN).toInt());
        // 文件传送失败，准备重新分配任务或重传
        emit taskFailureForPartner(partnerId, token);
    }

    return true;
}

bool TCPSocketUtil::sendToPartner(qint32 partnerId, CommMsg & msg)
{
    if (!this->partnerConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::sendToPartner " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << partnerId << endl;
        return false;
    }

    if (msg.getMsgType() != TCPCtrlMsgType::AREYOUALIVE && msg.getMsgType() != TCPCtrlMsgType::ASKFORHELP && msg.getMsgType() != TCPCtrlMsgType::DOWNLOADTASK && msg.getMsgType() == TCPCtrlMsgType::THANKYOURHELP && msg.getMsgType() == TCPCtrlMsgType::ENDYOURHELP) {
        qDebug() << "TCPSocketUtil::sendToPartner " << "向伙伴客户端发送的消息类型不合法 " << partnerId << endl;
        return false;
    }

    if (msg.getMsgType() == TCPCtrlMsgType::DOWNLOADTASK) {
        this->partnerFileIndex[msg.getToken()] = 0;
    }

    // 请求伙伴客户端协助下载, 分配下载任务, 通知伙伴客户端可以继续传送文件, 下载已完成，终止任务
    this->partnerConnections[partnerId]->write(msg.toMsg());
    return true;
}

bool TCPSocketUtil::failToGetHelpFromPartner(QAbstractSocket::SocketError error, qint32 partnerId)
{
    qDebug() << "TCPSocketUtil::failToGetHelpFromPartner " << "无法和伙伴客户端建立稳定连接 " << partnerId << endl;
    qDebug() << "TCPSocketUtil::failToGetHelpFromPartner " << this->partnerConnections[partnerId]->errorString() << endl;
    qDebug() << "TCPSocketUtil::failToGetHelpFromPartner " << "SocketError " << error << endl;

    this->partnerConnections[partnerId]->close();
    delete this->partnerConnections[partnerId];
    this->partnerConnections[partnerId] = nullptr;
    this->partnerConnections.remove(partnerId);

    return true;
}

bool TCPSocketUtil::recFromFriend(qint32 friendId)
{
    QJsonObject jsonMsg = QJsonDocument::fromJson(this->guests[friendId]->readAll()).object();
    if (jsonMsg.value(MSGTYPE).isUndefined()) {
        qDebug() << "TCPSocketUtil::recFromFriend " << "无法解析朋友客户端发来的消息类型" << endl;
        return false;
    }

    if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::AREYOUALIVE) {
        // 确认存活，评估下载进度
        emit tellTaskProcess(friendId);

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::ASKFORHELP) {
        if (jsonMsg.value(DOWNLOADADDRESS).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromFriend " << "朋友客户端发来的消息不完整" << endl;
            return false;
        }

        // 获取文件下载的目标地址和总大小
        emit whetherToHelpFriend(friendId, jsonMsg.value(DOWNLOADADDRESS).toString(), qint32(jsonMsg.value(LENMAX).toInt()));

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::DOWNLOADTASK) {
        if (jsonMsg.value(TOKEN).isUndefined() || jsonMsg.value(POS).isUndefined() || jsonMsg.value(LEN).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromFriend " << "朋友客户端发来的消息不完整" << endl;
            return false;
        }

        // 确定具体任务
        emit startToDownload(friendId, qint32(jsonMsg.value(TOKEN).toInt()), qint64(jsonMsg.value(POS).toInt()), qint32(jsonMsg.value(LEN).toInt()));

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::ABORTTASK) {
        if (jsonMsg.value(TOKEN).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromFriend " << "朋友客户端发来的消息不完整" << endl;
            return false;
        }

       // 终止并清除当前下载任务
       emit abortDownloadTask(friendId, qint32(jsonMsg.value(TOKEN).toInt()));

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::THANKYOURHELP) {
        if (jsonMsg.value(TOKEN).isUndefined() || jsonMsg.value(INDEX).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromFriend " << "朋友客户端发来的消息不完整" << endl;
            return false;
        }

       // 本次小块传送结束，伙伴客户端准备传送下一块
       emit timeForNextSliceForFriend(friendId, qint32(jsonMsg.value(TOKEN).toInt()), qint32(jsonMsg.value(INDEX).toInt()));

    } else if (static_cast<TCPCtrlMsgType>(jsonMsg.value(MSGTYPE).toInt()) == TCPCtrlMsgType::ENDYOURHELP) {
        if (jsonMsg.value(TOKEN).isUndefined()) {
            qDebug() << "TCPSocketUtil::recFromFriend " << "朋友客户端发来的消息不完整" << endl;
            return false;
        }

        // 结束任务，伙伴客户端停止传送文件
        emit taskHasFinishedForFriend(friendId, qint32(jsonMsg.value(TOKEN).toInt()));
    }

    return true;
}

bool TCPSocketUtil::sendToFriend(qint32 friendId, CommMsg & msg)
{
    if (!this->guests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::sendToFriend " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << friendId << endl;
        createGuest(friendId);
    }

    if (msg.getMsgType() != TCPCtrlMsgType::P2PPUNCH && msg.getMsgType() != TCPCtrlMsgType::ISALIVE && msg.getMsgType() != TCPCtrlMsgType::AGREETOHELP && msg.getMsgType() == TCPCtrlMsgType::REFUSETOHELP && msg.getMsgType() == TCPCtrlMsgType::TASKFINISH && msg.getMsgType() == TCPCtrlMsgType::TASKFAILURE) {
        qDebug() << "TCPSocketUtil::sendToFriend " << "向朋友客户端发送的消息类型不合法 " << friendId << endl;
        return false;
    }

    this->guests[friendId]->write(msg.toMsg());
    return true;
}

bool TCPSocketUtil::failToHelpFriend(QAbstractSocket::SocketError error, qint32 friendId)
{
    qDebug() << "TCPSocketUtil::failToHelpFriend " << "无法和伙伴客户端建立稳定连接 " << friendId << endl;
    qDebug() << "TCPSocketUtil::failToHelpFriend " << this->guests[friendId]->errorString() << endl;
    qDebug() << "TCPSocketUtil::failToHelpFriend " << "SocketError " << error << endl;

    this->guests[friendId]->close();
    delete this->guests[friendId];
    this->guests[friendId] = nullptr;
    this->guests.remove(friendId);

    return true;
}

bool TCPSocketUtil::newConnectionWithFilePartner()
{
    qDebug() << "TCPSocketUtil::newConnectionWithFilePartner" << endl;

    QTcpSocket * oringinVistor = this->fileHost->nextPendingConnection();
    P2PTcpSocket * vistor = new P2PTcpSocket();

    QString partnerIP = oringinVistor->peerAddress().toString();
    quint16 partnerPort = oringinVistor->peerPort();

    qint32 vistorId = findIdFromClientsByIPAndPort(partnerIP, partnerPort);

    qDebug() << "TCPSocketUtil::newConnectionWithFilePartner " << "vistorId " << vistorId << endl;

    if (vistorId == -1 && !this->clientsMap.contains(vistorId)) {
        qDebug() << "TCPSocketUtil::newConnectionWithFilePartner " << "陌生客户端非法访问" << endl;
        return false;
    }

    if (this->partnerFileConnections.contains(vistorId) || this->partnerFileP2PConnections.contains(vistorId)) {
        qDebug() << "TCPSocketUtil::newConnectionWithFilePartner " << "伙伴客户端试图重复连接" << endl;
        return false;
    }

    this->partnerFileConnections[vistorId] = oringinVistor;

    this->partnerFileP2PConnections[vistorId] = vistor;
    this->partnerFileP2PConnections[vistorId]->setId(vistorId);

    connect(this->partnerFileConnections[vistorId], SIGNAL(readyRead()), this->partnerFileP2PConnections[vistorId], SLOT(ensureReadyRead()));
    connect(this->partnerFileConnections[vistorId], SIGNAL(error(QAbstractSocket::SocketError)), this->partnerFileP2PConnections[vistorId], SLOT(ensureError(QAbstractSocket::SocketError)));
    connect(this->partnerFileConnections[vistorId], SIGNAL(disconnected()), this->partnerFileP2PConnections[vistorId], SLOT(ensureDisconnected));

    connect(this->partnerFileP2PConnections[vistorId], SIGNAL(readyReadFromOthers(qint32)), this, SLOT(recFromFilePartner(qint32)));
    connect(this->partnerFileP2PConnections[vistorId], SIGNAL(socketErrorOfOthers(QAbstractSocket::SocketError, qint32)), this, SLOT(failToGetHelpFromFilePartner(QAbstractSocket::SocketError, qint32)));
    connect(this->partnerFileP2PConnections[vistorId], SIGNAL(disconnectedFromOthers(qint32)), this, SLOT(disConnectToFilePartner(qint32)));

    return true;
}

bool TCPSocketUtil::disConnectToFilePartner(qint32 partnerId)
{
    if (!this->partnerFileConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::disConnectToFilePartner " << "指定连接该伙伴客户端的P2PTcpSocket对象尚未建立 " << "partnerId " << partnerId << endl;
        return false;
    }

    this->partnerFileConnections[partnerId]->disconnectFromHost();

    if (!this->partnerFileP2PConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::disConnectToFilePartner " << "指定连接该伙伴客户端的P2PTcpSocket对象的P2P辅助尚未建立 " << "partnerId " << partnerId << endl;
        return false;
    }

    this->partnerFileP2PConnections[partnerId]->setId(-1);

    return true;
}

bool TCPSocketUtil::recFromFilePartner(qint32 partnerId)
{
    if (!this->partnerConnections.contains(partnerId)) {
        qDebug() << "TCPSocketUtil::recFromFilePartner " << "P2PTcpSocket::readyReadFromPartner信号的partnerId错误 " << partnerId << endl;
        return false;
    }

    QByteArray msg = this->partnerFileConnections[partnerId]->readAll();
    TCPCtrlMsgType msgType = TCPCtrlMsgType(msg.left(1).toInt());

    if (msgType == TCPCtrlMsgType::TASKEXECUING) {
        qint32 token = qint32(msg.mid(1,4).toInt());
        qint32 index = qint32(msg.mid(5,4).toInt());
        qint8 lastOne = qint8(msg.mid(9,1).toInt());

        if (index != this->partnerFileIndex[token]) {
            qDebug() << "TCPSocketUtil::recFromFilePartner " << "伙伴客户端数据发送顺序错误 " << partnerId << " 失败的任务令牌 " << token << " 索引 " << index << endl;
            emit timeForNextSliceForPartner(partnerId, token, this->partnerFileIndex[token]);
            return false;
        }

        QFile * file = new QFile(fileDir + QString::number(token) + fileType);
        if (!file->open(QIODevice::WriteOnly)) {
            qDebug() << "TCPSocketUtil::recFromFilePartner " << "伙伴客户端数据发送失败 " << partnerId << " 失败的任务令牌 " << token << " 索引 " << index << endl;
            return false;
        }

        if (lastOne != 1 && (msg.length() - 10)< this->sliceSize) {
            qDebug() << "TCPSocketUtil::recFromFilePartner " << "伙伴客户端数据发送不完整 " << partnerId << " 失败的任务令牌 " << token << " 索引 " << index << endl;
            return false;
        }

        file->write(msg.mid(10));

        if (lastOne == 1) {
            this->partnerFileIndex.remove(token);
            emit taskHasFinishedForFriend(partnerId, token);
        }

        return true;
    }

    qDebug() << "TCPSocketUtil::recFromFilePartner " << "向伙伴客户端发送的消息类型不合法 " << partnerId << endl;
    return false;
}

bool TCPSocketUtil::failToGetHelpFromFilePartner(QAbstractSocket::SocketError error, qint32 partnerId)
{
    qDebug() << "TCPSocketUtil::failToGetHelpFromFilePartner " << "无法和伙伴客户端建立稳定连接 " << partnerId << endl;
    qDebug() << "TCPSocketUtil::failToGetHelpFromFilePartner " << this->partnerFileConnections[partnerId]->errorString() << endl;
    qDebug() << "TCPSocketUtil::failToGetHelpFromFilePartner " << "SocketError " << error << endl;

    this->partnerFileConnections[partnerId]->close();
    delete this->partnerFileConnections[partnerId];
    this->partnerFileConnections[partnerId] = nullptr;
    this->partnerFileConnections.remove(partnerId);

    return true;
}

bool TCPSocketUtil::sendToFileFriend(qint32 friendId, FileMsg & msg)
{
    if (!this->fileGuests.contains(friendId)) {
        qDebug() << "TCPSocketUtil::sendToFileFriend " << "指定连接该朋友客户端的P2PTcpSocket对象尚未建立 " << friendId << endl;
        createGuest(friendId);
    }

    if (!this->clientsMap.contains(friendId)) {
        qDebug() << "TCPSocketUtil::sendToFileFriend " << "无法获取指定伙伴的ip地址 " << friendId << endl;
        return false;
    }

    if (msg.getMsgType() != TCPCtrlMsgType::TASKEXECUING) {
        qDebug() << "TCPSocketUtil::sendToFileFriend " << "向朋友客户端发送的消息类型不合法 " << friendId << endl;
        return false;
    }

    this->fileGuests[friendId]->write(msg.toMsg());
    return true;
}

bool TCPSocketUtil::failToHelpFileFriend(QAbstractSocket::SocketError error, qint32 friendId)
{
    qDebug() << "TCPSocketUtil::failToHelpFileFriend " << "无法和伙伴客户端建立稳定连接 " << friendId << endl;
    qDebug() << "TCPSocketUtil::failToHelpFileFriend " << this->fileGuests[friendId]->errorString() << endl;
    qDebug() << "TCPSocketUtil::failToHelpFileFriend " << "SocketError " << error << endl;

    this->fileGuests[friendId]->close();
    delete this->fileGuests[friendId];
    this->fileGuests[friendId] = nullptr;
    this->fileGuests.remove(friendId);

    return true;
}

qint32 TCPSocketUtil::findIdFromClientsByIPAndPort(QString ip, quint16 port)
{
    qint32 id = -1;
    qDebug() << "TCPSocketUtil::findIdFromClientsByIPAndPort " << "ip " << ip << " port " << port << endl;

    for(QMap<qint32, Client *>::iterator it = this->clientsMap.begin(); it != this->clientsMap.end(); it++) {
        qDebug() << "TCPSocketUtil::findIdFromClientsByIPAndPort " << "ip " << it.value()->getIP()
                 << " port " << it.value()->getPort() << endl;

        if(it.value()->getIP() == ip && it.value()->getPort() == port){
            id = it.value()->getId();
            break;
        }
    }

    qDebug() << "TCPSocketUtil::findIdFromClientsByIPAndPort " << "id " << id << endl;
    return id;
}
