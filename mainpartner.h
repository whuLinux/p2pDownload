#ifndef MAINPARTNER_H
#define MAINPARTNER_H

#include"mainrole.h"

/**
 * @brief The MainPartner class
 * 作为伙伴机协助下载
 */
class MainPartner:public MainRole
{
    Q_OBJECT

private:
    qint32 friendId;//登记friendID

    QVector<partnerSlices> sliceScheduler;
    QVector<qint32> rubbishBin;//存储完成任务的token，供最后mission结束时清理零时文件
    //下载任务队列
    QVector<partnerTask> partnerTaskLists;
public:
    MainPartner();
    MainPartner(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
                mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil);


public slots:
    //收到求助请求,选择是否下载
    void recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax);
    //收到ctrl中止当前下载的命令，终止当前下载任务
    void recAbortOrder();
    //收到DOWNLOADTASK，记录下载任务并通知taskStartAsPartner
    void recTaskFromFriend(qint32 friendId, qint32 token, qint64 pos, qint32 len);
    //接受DOWNLOADTASK，开始task下载
    void taskStartAsPartner();
    //task下载完成，向主机发送TASKFINISH,唤起sliceScheduler准备分片发送，检查taskLists是否仍有等待任务，有则唤起下载
    void taskEndAsPartner();
    //响应AREYOUALIVE 汇报下载进度
    void reportTaskProgress(qint32 friendId);
    //slice分片调度器，将对应token的task文件切分并发送给friend，发TASKEXECUTING；完全发送完，发TASKFINISH
    void sliceDivideAndSent(qint32 friendId,qint32 token,qint32 expectIndex);
    //mission完成，状态置为空闲
    void missionEndAsPartner();


signals:
    //唤醒sliceDivideScheduler进行调度
    void callSliceScheduler(qint32 friendId,qint32 token,qint32 expectIndex);
    //下载器空闲，有下载任务，唤起taskStartAsPartner
    void callTaskStartAsPartner();
    //分片下载完成，调用taskEndAsPartner
    void callTaskEndAsPartner(qint32 friendId, qint32 token, qint32 len);

};

#endif // MAINPARTNER_H
