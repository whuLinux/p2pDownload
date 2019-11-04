#include "p2ptcpsocket.h"

P2PTcpSocket::P2PTcpSocket()
{

}

void P2PTcpSocket::ensureReadyRead()
{
    emit readyReadFromPartner(this->id);
}

void P2PTcpSocket::ensureError(QAbstractSocket::SocketError error)
{
    emit socketErrorOfPartner(error, this->id);
}

void P2PTcpSocket::ensureDisconnected()
{
    emit disconnectedFromPartner(this->id);
}
