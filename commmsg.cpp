#include "commmsg.h"

CommMsg::CommMsg()
{

}

CommMsg::CommMsg(TCPCtrlMsgType msgType) : msgType(msgType)
{

}

CommMsg::CommMsg(TCPCtrlMsgType msgType, double rate) : msgType(msgType), rate(rate)
{

}

CommMsg::CommMsg(TCPCtrlMsgType msgType, QString downloadAddress, qint32 lenMax) : msgType(msgType), downloadAddress(downloadAddress), lenMax(lenMax)
{

}

CommMsg::CommMsg(TCPCtrlMsgType msgType, qint32 token, qint64 pos, qint32 len) : msgType(msgType), token(token), pos(pos), len(len)
{

}

CommMsg::CommMsg(TCPCtrlMsgType msgType, qint32 token, qint32 index) : msgType(msgType), token(token), index(index)
{

}

QByteArray CommMsg::toMsg()
{
    QJsonObject jsonMsg {
        {MSGTYPE, qint8(this->msgType)}
    };

    if (this->msgType == TCPCtrlMsgType::ISALIVE) {
        jsonMsg.insert(RATE, this->rate);

    } else if (this->msgType == TCPCtrlMsgType::ASKFORHELP) {
        jsonMsg.insert(DOWNLOADADDRESS, this->downloadAddress);
        jsonMsg.insert(LENMAX, this->lenMax);

    } else if (this->msgType == TCPCtrlMsgType::DOWNLOADTASK) {
        jsonMsg.insert(TOKEN, this->token);
        jsonMsg.insert(POS, this->pos);
        jsonMsg.insert(LEN, this->len);

    } else if (this->msgType == TCPCtrlMsgType::TASKFINISH || this->msgType == TCPCtrlMsgType::TASKFAILURE) {
        jsonMsg.insert(TOKEN, this->token);

    } else if (this->msgType == TCPCtrlMsgType::THANKYOURHELP) {
        jsonMsg.insert(TOKEN, this->token);
        jsonMsg.insert(INDEX, this->index);
    }

    return QJsonDocument(jsonMsg).toJson();
}
