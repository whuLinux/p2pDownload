#include "p2ptcpsocket.h"

P2PTcpSocket::P2PTcpSocket()
{

}

void P2PTcpSocket::ensureReadyRead()
{
    qDebug() << "P2PTcpSocket::ensureReadyRead " << "id " << endl;
    emit readyReadFromOthers(this->id);
}

void P2PTcpSocket::ensureError(QAbstractSocket::SocketError error)
{
    qDebug() << "P2PTcpSocket::ensureDisconnected " << "id " << endl;
    emit socketErrorOfOthers(error, this->id);
}

void P2PTcpSocket::ensureDisconnected()
{
    qDebug() << "P2PTcpSocket::ensureDisconnected " << "id " << endl;
    emit disconnectedFromOthers(this->id);
}
