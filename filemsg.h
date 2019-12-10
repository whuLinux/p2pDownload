#ifndef FILEMSG_H
#define FILEMSG_H

#include <QDataStream>
#include <QByteArray>
#include <QIODevice>

#include "uniformlabel.h"

/**
 * @brief The FileMsg class
 * @author 余宗宪
 * 朋友客户端和伙伴客户端传输文件的控制消息
 */
class FileMsg
{
private:
    TCPCtrlMsgType msgType;

    /** 本次传送的文件信息
     * token 任务令牌，任务的唯一标识
     * index 任务等待P2P传送的文件分块后，块的唯一标识
     * lastOne 最后一块的标识
     * msg P2P传送的文件
     */
    qint32 token;
    qint32 index;
    qint8 lastOne;
    QByteArray msg;

public:
    FileMsg();
    FileMsg(TCPCtrlMsgType msgType, qint32 token, qint32 index, qint8 lastOne);

    void setMsg(QByteArray & msg);

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

