#include "ctrlmsg.h"

CtrlMsg::CtrlMsg()
{
    this->clientNum = 0;
    this->clientIndex = 0;
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

    if (this->msgType == UDPCtrlMsgType::LOGIN) {
        jsonMsg.insert(HOSTNAME, this->hostName);
        jsonMsg.insert(PWD, this->pwd);
        jsonMsg.insert(IP, this->ip);
        jsonMsg.insert(PORT, this->port);

    } else if (this->msgType == UDPCtrlMsgType::LOGOUT) {
        jsonMsg.insert(HOSTNAME, this->hostName);
        jsonMsg.insert(PWD, this->pwd);

    } else if (this->msgType == UDPCtrlMsgType::P2PTRANS) {
        jsonMsg.insert(PARTNERNAME, this->partnerName);

    } else if (this->msgType == UDPCtrlMsgType::RETURNALLPARTNERS) {
        // 仅供解析服务器端数据参考

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
        // 仅供解析服务器端数据参考

        QJsonObject jsonClient {
            {PARTNERNAME, this->clients[0].name},
            {IP, this->clients[0].ip},
            {PORT, this->clients[0].port}
        };

        jsonMsg.insert(FRIEND, jsonClient);
    }

    return QJsonDocument(jsonMsg).toJson();
}
