#include "filemsg.h"

FileMsg::FileMsg()
{

}

FileMsg::FileMsg(TCPCtrlMsgType msgType, qint32 token, qint32 index) : msgType(msgType), token(token), index(index)
{

}

void FileMsg::setMsg(QByteArray & msg)
{
    //使用数据流写入数据
    QDataStream out(&(this->msg),QIODevice::WriteOnly);

    //设置数据流的版本，客户端和服务器端使用的版本要相同
    out.setVersion(QDataStream::Qt_5_13);

    out << qint8(0) << qint32(0) << qint32(0) << msg;
}
