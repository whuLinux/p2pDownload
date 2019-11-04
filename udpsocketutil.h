#ifndef UDPSOCKETUTIL_H
#define UDPSOCKETUTIL_H

#include <QObject>
#include <QMap>

#include <QUdpSocket>
#include <QHostAddress>

#include "ctrlmsg.h"
#include "partner.h"
#include "uniformlabel.h"

class UDPSocketUtil : public QObject
{
    Q_OBJECT
private:
    /**
     * 客户端信息
     */
    QUdpSocket * client;
    quint16 clientPort;

    /**
     * 服务器信息
     */
    QString serverIP;
    quint16 serverPort;

    /**
     * 伙伴客户端信息
     */
    QVector<ClientNode> partners;

public:
    UDPSocketUtil();
    UDPSocketUtil(quint16 clientPort, QString serverIP, quint16 serverPort);
    ~UDPSocketUtil();

    bool stablishClient();
    bool createSocket();
    bool bindClientPort();

public slots:
    /**
     * @brief 向服务器端发送消息
     */
    bool login(CtrlMsg * msg);
    bool logout(CtrlMsg * msg);
    bool obtainAllPartners(CtrlMsg * msg);
    bool p2pTrans(CtrlMsg * msg);

    /**
     * @brief 接收服务器端消息
     */
    bool recfromServer();
    bool p2pNeedHole(QJsonObject jsonMsg);
    bool receiveAllPartners(QJsonObject jsonMsg);

    /**
     * @brief 和主控单元交互，提供伙伴客户端数据
     */
    inline void clearPartners();
    inline QVector<ClientNode> getAllPartners();

signals:
    /**
     * 和主控单元交互，提供伙伴客户端数据
     */
    void p2pHoleRequestFromServer(QString name, QString ip, quint16 port);
    void timeToGetAllPartners();
};

/**
 * @brief UDPSocketUtil::clearPartners
 * 每次发送伙伴客户端的信息被处理前，不能清空列表
 */
void UDPSocketUtil::clearPartners()
{
    this->partners.clear();
}

QVector<ClientNode> UDPSocketUtil::getAllPartners()
{
    return this->partners;
}

#endif // UDPSOCKETUTIL_H
