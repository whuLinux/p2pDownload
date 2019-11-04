#include "ctrlmsg.h"

CtrlMsg::CtrlMsg()
{

}

CtrlMsg::CtrlMsg(UDPCtrlMsgType msgType)
{
    this->msgType = msgType;
    this->clientNum = 0;
    this->clientIndex = 0;
}

QByteArray CtrlMsg::toMsg()
{
    QJsonObject jsonMsg {
        {MSGTYPE, qint8(this->msgType)}
    };

    if (this->msgType == UDPCtrlMsgType::LOGIN || this->msgType == UDPCtrlMsgType::LOGOUT) {
        jsonMsg.insert(HOSTNAME, this->hostName);
    } else if (this->msgType == UDPCtrlMsgType::P2PTRANS) {
        jsonMsg.insert(PARTNERNAME, this->partnerName);
    }

    // 仅供解析服务器端数据参考
    else if (this->msgType == UDPCtrlMsgType::RETURNALLPARTNERS) {
        QJsonArray jsonClients;

        for (int i = 0; i < this->clientNum; i++) {
            QJsonObject jsonClient {
                {PARTNERNAME, this->clients[i].name},
                {IP, this->clients[i].ip},
                {PORT, this->clients[i].port}
            };

            jsonClients.append(jsonClient);
        }

        jsonMsg.insert(PARTNERVECTOR, jsonClients);

    } else if (this->msgType == UDPCtrlMsgType::P2PNEEDHOLE) {
        QJsonObject jsonClient {
            {PARTNERNAME, this->clients[0].name},
            {IP, this->clients[0].ip},
            {PORT, this->clients[0].port}
        };

        jsonMsg.insert(FRIEND, jsonClient);
    }

    return QJsonDocument(jsonMsg).toJson();
}
