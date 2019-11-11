#include "p2ptcpsocket.h"

P2PTcpSocket::P2PTcpSocket()
{

}

void P2PTcpSocket::ensureReadyRead()
{
    emit readyReadFromOthers(this->id);
}

void P2PTcpSocket::ensureError(QAbstractSocket::SocketError error)
{
    emit socketErrorOfOthers(error, this->id);
}

void P2PTcpSocket::ensureDisconnected()
{
    emit disconnectedFromOthers(this->id);
}
