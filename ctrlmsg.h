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
     * 本机密码、端口
     */
    QString pwd;
    quint16 port;

public:
    CtrlMsg();
    CtrlMsg(UDPCtrlMsgType msgType);

    inline void setHostName(QString hostName);
    inline void setPartnerName(QString partnerName);
    inline void setPwd(QString pwd);
    inline void setPort(quint16 port);

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

void CtrlMsg::setPort(quint16 port)
{
    this->port = port;
}

UDPCtrlMsgType CtrlMsg::getMsgType()
{
    return this->msgType;
}

#endif // CTRLMSG_H

