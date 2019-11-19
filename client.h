#ifndef Client_H
#define Client_H

#include <QHostAddress>
#include <QString>
#include <QDebug>

/**
 * @brief The Client class
 * 主机信息
 * 请求发起机（即localhost），特殊标记
 */
class Client
{
private:
    qint32 id;
    QString name;
    QString ip;
    quint16 port;
    quint16 filePort;
    bool hasTask;
    quint8 taskNum;//本轮分配的下载任务数量

public:
    Client();
    Client(qint32 id, QString name, QString ip, quint16 port, quint16 filePort);
    ~Client();

    inline void attributeTask();
    inline void endTask();

    inline qint32 getId();
    inline QString getName();
    inline QString getIP();
    inline quint16 getPort();
    inline quint16 getFilePort();
    inline bool gethasTask();
    inline quint8 getTaskNum();

    inline void setTaskNum(quint8 taskNum);
};

void Client::attributeTask()
{
    this->hasTask = true;
}

void Client::endTask()
{
    this->hasTask = false;
}

qint32 Client::getId()
{
    return this->id;
}

QString Client::getName()
{
    return this->name;
}

QString Client::getIP()
{
    return this->ip;
}

quint16 Client::getPort()
{
    return this->port;
}

quint16 Client::getFilePort()
{
    return this->filePort;
}

bool Client::gethasTask()
{
    return this->hasTask;
}

quint8 Client::getTaskNum()
{
    return this->taskNum;
}

void Client::setTaskNum(quint8 taskNum)
{
    this->taskNum=taskNum;
}

#endif // Client_H
