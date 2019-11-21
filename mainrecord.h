#ifndef MAINRECORD_H
#define MAINRECORD_H

#include<qvector.h>
#include<qstring.h>
#include<QTimer>
#include"recqtimer.h"
#include"mainctrlmacro.h"
/**
 * @brief The mainRecord class
 * 任务分配记录，作为任务分配表的表项
 */
class mainRecord//:public QObject
{
//    Q_OBJECT

private:
    qint8 recordID; //单个记录标识
    qint32 clientId; //下载主机的标识
    qint32 token; //任务token
    QVector<blockInfo> blockIds; //下载文件块的下标
    //参照p2ptcpsocket, tcpsocketutil::newConnectionWithPartner 强化设计
    //RecQTimer *recTimer; //计时

public:
    mainRecord();
    mainRecord(qint8 recordid,qint32 clientId,qint32 token);

    qint8 getRecordID() const;
    void setRecordID(const qint8 &value);
    qint32 getClientId() const;
    void setClientId(const qint32 &value);
    qint32 getToken() const;
    void setToken(const qint32 &value);
    void addBlockId(blockInfo blockId);
    QVector<blockInfo> getBlockIds() const;
};

#endif // MAINRECORD_H
