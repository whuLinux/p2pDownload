#ifndef FILEMSG_H
#define FILEMSG_H

#include <QDataStream>
#include <QByteArray>
#include <QIODevice>

#include "uniformlabel.h"

class FileMsg
{
private:
    TCPCtrlMsgType msgType;

    /** 本次传送的文件信息
     * token 任务令牌，任务的唯一标识
     * index 任务等待P2P传送的文件分块后，块的唯一标识
     * msg P2P传送的文件
     */
    qint32 token;
    qint32 index;
    QByteArray msg;

public:
    FileMsg();
    FileMsg(TCPCtrlMsgType msgType, qint32 token, qint32 index);

    void setMsg(QByteArray msg);

    inline QByteArray toMsg();

    inline TCPCtrlMsgType getMsgType();
};

QByteArray FileMsg::toMsg()
{
    return this->msg;
}

TCPCtrlMsgType FileMsg::getMsgType()
{
    return this->msgType;
}

#endif // FILEMSG_H

