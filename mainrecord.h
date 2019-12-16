#ifndef MAINRECORD_H
#define MAINRECORD_H

#include<qvector.h>
#include<qstring.h>
#include<QTimer>
#include<QDebug>
#include"mainctrlmacro.h"
/**
 * @brief The mainRecord class
 * @author Vincent Xue
 * 任务分配记录，作为任务分配表的表项
 */
class mainRecord:public QObject
{
    Q_OBJECT

private:
    qint8 recordID; //单个记录标识
    qint32 clientId; //下载主机的标识
    qint32 token; //任务token
    QVector<blockInfo> blockIds; //下载文件块的下标
    QTimer *timer;

public:
    mainRecord();
    mainRecord(const mainRecord &obj);//拷贝构造函数
    mainRecord(qint8 recordid,qint32 clientId,qint32 token);

    qint8 getRecordID() const;
    void setRecordID(const qint8 &value);
    qint32 getClientId() const;
    void setClientId(const qint32 &value);
    qint32 getToken() const;
    void setToken(const qint32 &value);
    void addBlockId(blockInfo blockId);
    QVector<blockInfo> getBlockIds() const;
    //初始化timer,设定时间(毫秒为单位）并开始计时，isSingle决定timer是否循环预报
    void createTimer(qint64 gap,bool isSingle);
    //删除当前timer
    void deleteTimer()  {delete this->timer;this->timer=nullptr;}
    //timer空指针判断
    bool isNullTimer()  {return this->timer==nullptr;}
    
public slots:
    //接收timer timeout信号并转发sendTimeOutToCtrl给主控模块
    void recAlert();
    
signals:
    //任务超时信号
    void sendTimeOutToCtrl(qint32 token);
};

#endif // MAINRECORD_H
