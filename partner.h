#ifndef PARTNER_H
#define PARTNER_H

#include <QHostAddress>
#include <QString>
#include <QDebug>

/**
 * @brief The Partner class
 * 主机信息
 * 请求发起机（即localhost），特殊标记
 */
class Partner
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
    Partner();
    Partner(qint32 id, QString name, QString ip, quint16 port, quint16 filePort);
    ~Partner();

    inline void attributeTask();
    inline void endTask();

    inline qint32 getId();
    inline QString getName();
    inline QString getIP();
    inline quint16 getPort();
    inline quint16 getFilePort();
    inline bool gethasTask();
    inline quint8 getTaskNum();

    void setTaskNum(quint8 taskNum);
};

void Partner::attributeTask()
{
    this->hasTask = true;
}

void Partner::endTask()
{
    this->hasTask = false;
}

qint32 Partner::getId()
{
    return this->id;
}

QString Partner::getName()
{
    return this->name;
}

QString Partner::getIP()
{
    return this->ip;
}

quint16 Partner::getPort()
{
    return this->port;
}

quint16 Partner::getFilePort()
{
    return this->filePort;
}

bool Partner::gethasTask()
{
    return this->hasTask;
}

quint8 Partner::getTaskNum()
{
    return this->taskNum;
}

// ！这里编译报错了，注意查一下！
//void Partner::setTaskNum(quint8 taskNum)
//{
//    this->taskNum=taskNum;
//}

#endif // PARTNER_H
