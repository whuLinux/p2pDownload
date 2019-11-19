#ifndef COMMMSG_H
#define COMMMSG_H

#include <QHostAddress>

#include <QByteArray>

#include <QJsonDocument>
#include <QJsonObject>

#include "uniformlabel.h"

/**
 * @brief The CommMsg class
 * 客户端交流的控制信息
 */
class CommMsg
{
private:
    TCPCtrlMsgType msgType;

    /**
     * 文件下载进度
     */
    qint8 rate;

    /** 文件下载信息
     * downloadAddress 文件下载地址
     * lenMax 文件最大长度
     */
    QString downloadAddress;
    qint32 lenMax;

    /** 本次传送的文件信息
     * token 任务令牌，任务的唯一标识
     * index 任务等待P2P传送的文件分块后，块的唯一标识
     * pos 任务规定文件的下载起始位置，用于HTTP断点续传
     * len 任务规定文件的下载长度
     */
    qint32 token;
    qint32 index;
    qint64 pos;
    qint32 len;

public:
    CommMsg();
    CommMsg(TCPCtrlMsgType msgType);
    CommMsg(TCPCtrlMsgType msgType, qint8 rate);
    CommMsg(TCPCtrlMsgType msgType, QString downloadAddress, int lenMax);
    CommMsg(TCPCtrlMsgType msgType, qint32 token, qint64 pos, qint32 len);
    CommMsg(TCPCtrlMsgType msgType, qint32 token, qint32 index);

    QByteArray toMsg();

    inline void setToken(qint32 token);
    inline void setIndex(qint32 index);
    inline TCPCtrlMsgType getMsgType();
    inline qint32 getToken();
    inline qint32 getIndex();
};

void CommMsg::setToken(qint32 token)
{
    this->token = token;
}

void CommMsg::setIndex(qint32 index)
{
    this->index = index;
}

TCPCtrlMsgType CommMsg::getMsgType()
{
    return this->msgType;
}

qint32 CommMsg::getToken()
{
    return this->token;
}

qint32 CommMsg::getIndex()
{
    return this->index;
}

#endif // COMMMSG_H
