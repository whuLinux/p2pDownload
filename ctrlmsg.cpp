#include "ctrlmsg.h"

/**
 * @brief The CtrlmMsg class
 * @author 余宗宪
 * 服务器和客户端UDP通信的控制消息
 */

CtrlMsg::CtrlMsg()
{

}

CtrlMsg::CtrlMsg(UDPCtrlMsgType msgType)
{
    this->msgType = msgType;
}

QByteArray CtrlMsg::toMsg()
{
    QJsonObject jsonMsg {
        {MSGTYPE, qint8(this->msgType)}
    };

    if (this->msgType == UDPCtrlMsgType::LOGIN) {
        jsonMsg.insert(HOSTNAME, this->hostName);
        jsonMsg.insert(PWD, this->pwd);
        jsonMsg.insert(PORT, this->port);
        jsonMsg.insert(FILEPORT, this->filePort);

    } else if (this->msgType == UDPCtrlMsgType::LOGOUT) {
        jsonMsg.insert(HOSTNAME, this->hostName);
        jsonMsg.insert(PWD, this->pwd);

    } else if (this->msgType == UDPCtrlMsgType::OBTAINALLPARTNERS) {
        jsonMsg.insert(HOSTNAME, this->hostName);
        jsonMsg.insert(PWD, this->pwd);

    } else if (this->msgType == UDPCtrlMsgType::P2PTRANS) {
        jsonMsg.insert(HOSTNAME, this->hostName);
        jsonMsg.insert(PWD, this->pwd);
        jsonMsg.insert(PARTNERNAME, this->partnerName);

    }

    return QJsonDocument(jsonMsg).toJson();
}
