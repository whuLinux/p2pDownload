#include "mainrecord.h"
#include"mainctrlmacro.h"

mainRecord::mainRecord()
{

}

mainRecord::mainRecord(qint8 recordid,qint32 hostid,qint32 token, QVector<qint8> blocks)
{
    this->recordID=recordid;
    this->hostID=hostid;
    this->token=token;
    this->blockId=blocks;
    this->recTimer=new RecQTimer();
    this->recTimer->start(DDL); //下载时间上限
}
