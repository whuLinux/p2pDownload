#ifndef P2PTCPSOCKET_H
#define P2PTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

/**
 * @brief The P2PTcpSocket class
 * @author 余宗宪
 * P2P通信中Socket的辅助工具类，主要作为信号传播的中介
 */
class P2PTcpSocket : public QObject
{
    Q_OBJECT
private:
    qint32 id;

public:
    P2PTcpSocket();
    inline void setId(qint32 id);
    inline qint32 getId();

public slots:
    void ensureReadyRead();
    void ensureError(QAbstractSocket::SocketError error);
    void ensureDisconnected();

signals:
    void readyReadFromOthers(qint32 partnerId);
    void socketErrorOfOthers(QAbstractSocket::SocketError error, qint32 partnerId);
    void disconnectedFromOthers(qint32 partnerId);
};

void P2PTcpSocket::setId(qint32 id)
{
    this->id = id;
}

qint32 P2PTcpSocket::getId()
{
    return this->id;
}

#endif // P2PTCPSOCKET_H
