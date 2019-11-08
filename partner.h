#ifndef PARTNER_H
#define PARTNER_H

#include <QHostAddress>
#include <QString>
#include <QDebug>

class Partner
{
private:
    qint32 id;
    QString name;
    QString ip;
    quint16 port;
    quint16 filePort;
    bool hasTask;

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


#endif // PARTNER_H
