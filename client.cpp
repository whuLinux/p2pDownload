#include "client.h"

Client::Client()
{

}

Client::Client(qint32 id, QString name, QString ip, quint16 port, quint16 filePort) : id(id), name(name), ip(ip), port(port), filePort(filePort)
{
    this->hasTask=false;
    this->taskNum=0;
}

Client::~Client()
{

}

