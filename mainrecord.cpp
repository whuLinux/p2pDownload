#include "mainrecord.h"
#include"mainctrlmacro.h"

qint8 mainRecord::getRecordID() const
{
    return recordID;
}

void mainRecord::setRecordID(const qint8 &value)
{
    recordID = value;
}

qint32 mainRecord::getClientId() const
{
    return clientId;
}

void mainRecord::setClientId(const qint32 &value)
{
    clientId = value;
}

qint32 mainRecord::getToken() const
{
    return token;
}

void mainRecord::setToken(const qint32 &value)
{
    token = value;
}


void mainRecord::addBlockId(blockInfo blockId){
    this->blockIds.append(blockId);
}

QVector<blockInfo> mainRecord::getBlockIds() const
{
    return blockIds;
}

mainRecord::mainRecord()
{
    //伪记录
    this->recordID=FAKERECORD;this->token=FAKERECORD;this->clientId=FAKERECORD;
    //    this->blockIds=QVector<qint8>();//空向量
//    this->recTimer=new RecQTimer();
}

mainRecord::mainRecord(qint8 recordid,qint32 clientId,qint32 token, QVector<qint8> blocks)
{
    this->recordID=recordid;
    this->clientId=clientId;
    this->token=token;
    this->blockIds=blocks;
    this->recTimer=new RecQTimer();
    this->recTimer->start(DDL); //下载时间上限
}
