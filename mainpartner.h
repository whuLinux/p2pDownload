#ifndef MAINPARTNER_H
#define MAINPARTNER_H

#include"mainrole.h"

/**
 * @brief The MainPartner class
 * 作为伙伴机协助下载
 */
class MainPartner:public QObject,public MainRole
{
    Q_OBJECT

private:
    qint32 friendId;//登记friendID
    QVector<partnerTask> sliceScheduler;
public:
    MainPartner();
    MainPartner(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
                mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil);


public slots:
    //收到求助请求,选择是否下载
    void recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax);
    //收到ctrl中止当前下载的命令，终止当前下载任务
    void recAbortOrder();
    //接受DOWNLOADTASK，开始task下载
    void taskStartAsPartner(qint32 friendId, qint32 token, qint64 pos, qint32 len);
    //task下载完成，向主机发送TASKFINISH
    void taskEndAsPartner(qint32 friendId, qint32 token,qint32 len);
    //响应AREYOUALIVE 汇报下载进度
    void reportTaskProgress(qint32 friendId);
    //slice分片调度器，切分并发送slice.
    void sliceDivideAndSent(qint32 friendId,qint32 token,qint32 expectIndex);
    //mission完成，状态置为空闲
    //TODO：删除协助下载的文件块
    void missionEndAsPartner();


signals:
    //唤醒sliceDivideScheduler进行调度
    void callSliceScheduler(qint32 friendId,qint32 token,qint32 expectIndex);
    //分片下载完成，调用taskEndAsPartner
    void callTaskEndAsPartner(qint32 friendId, qint32 token, qint32 len);

};

#endif // MAINPARTNER_H
