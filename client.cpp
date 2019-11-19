#include "client.h"

Client::Client()
{

}

Client::Client(qint32 id, QString name, QString ip, quint16 port, quint16 filePort) : id(id), name(name), ip(ip), port(port), filePort(filePort)
{

}

Client::~Client()
{

}

