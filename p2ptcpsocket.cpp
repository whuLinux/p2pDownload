#include "p2ptcpsocket.h"

/**
 * @brief The P2PTcpSocket class
 * @author 余宗宪
 * P2P通信中Socket的辅助工具类，主要作为信号传播的中介
 */

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
