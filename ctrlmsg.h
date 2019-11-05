#ifndef CTRLMSG_H
#define CTRLMSG_H

#include <QByteArray>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "uniformlabel.h"

class CtrlMsg
{
private:
    UDPCtrlMsgType msgType;

    /**
     * 本机名、打洞对象名
     */
    QString hostName;
    QString partnerName;

    /**
     * 本机密码、ip和端口
     */
    QString pwd;
    QString ip;
    quint16 port;

    /**
     * 伙伴客户端信息
     */
    qint32 clientNum;
    qint32 clientIndex;
    ClientNode * clients;

public:
    CtrlMsg();
    CtrlMsg(UDPCtrlMsgType msgType);

    inline void setHostName(QString hostName);
    inline void setPartnerName(QString partnerName);
    inline void setPwd(QString pwd);
    inline void setIP(QString ip);
    inline void setPort(quint16 port);
    inline void setClientNum(qint32 clientNum);
    inline void addClient(ClientNode client);

    QByteArray toMsg();

    inline UDPCtrlMsgType getMsgType();
};

void CtrlMsg::setHostName(QString hostName)
{
    this->hostName = hostName;
}

void CtrlMsg::setPartnerName(QString partnerName)
{
    this->partnerName = partnerName;
}

void CtrlMsg::setPwd(QString pwd)
{
    this->pwd = pwd;
}

void CtrlMsg::setIP(QString ip)
{
    this->ip = ip;
}

void CtrlMsg::setPort(quint16 port)
{
    this->port = port;
}

void CtrlMsg::setClientNum(qint32 clientNum)
{
    this->clientNum = clientNum;
    this->clients = new ClientNode[clientNum];
}

void CtrlMsg::addClient(ClientNode client)
{
    if (this->clientIndex < this->clientNum) {
        this->clients[this->clientIndex] = client;
        this->clientIndex++;
    }
}

UDPCtrlMsgType CtrlMsg::getMsgType()
{
    return this->msgType;
}

#endif // CTRLMSG_H

