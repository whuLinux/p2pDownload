#include "client.h"

/**
 * @brief The Client class
 * @author 余宗宪、薛国潼
 */

bool Client::getPunchSuccess() const
{
    return punchSuccess;
}

void Client::setPunchSuccess(bool value)
{
    punchSuccess = value;
}

Client::Client()
{

}

Client::Client(qint32 id, QString name, QString ip, quint16 port, quint16 filePort) : id(id), name(name), ip(ip), port(port), filePort(filePort)
{
    this->hasTask=false;
    this->taskNum=0;
    this->punchSuccess=false;
}

Client::~Client()
{

}

