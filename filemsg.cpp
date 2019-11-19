#include "filemsg.h"

FileMsg::FileMsg()
{

}

FileMsg::FileMsg(TCPCtrlMsgType msgType, qint32 token, qint32 index, qint8 lastOne) : msgType(msgType), token(token), index(index), lastOne(lastOne)
{

}

void FileMsg::setMsg(QByteArray & msg)
{
    //使用数据流写入数据
    QDataStream out(&(this->msg),QIODevice::WriteOnly);

    //设置数据流的版本，客户端和服务器端使用的版本要相同
    out.setVersion(QDataStream::Qt_5_13);

    out << qint8(this->msgType) << qint32(this->token) << qint32(this->index) << qint8(lastOne) << msg;
}
