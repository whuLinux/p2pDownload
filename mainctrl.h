#ifndef MAINCTRL_H
#define MAINCTRL_H
#include<algorithm>
#include<QQueue>
#include<QDebug>
#include"udpsocketutil.h"
#include"tcpsocketutil.h"
#include"msgutil.h"
#include"downloadmanager.h"
#include"commmsg.h"
#include"filemsg.h"
#include"ctrlmsg.h"
#include"mainrecord.h"
#include"mainctrlutil.h"
#include"mainctrlmacro.h"
#include"client.h"

//TODO:测试用控制台 UI待制作
#include<string>
#include<iostream>  //cin cout
#include<stdio.h> //printf
using namespace std;

/**
 * @brief The mainctrl class
 * 主控模块，执行下述功能：
 * 1. 主机注册
 * 2. 下载任务分配、调度
 */

//TODO:输出使用qInstallMsgHandler 配置到日志LOGPATH中
class mainctrl:public QObject
{
    Q_OBJECT

private:
    QVector<blockInfo> blockQueue;
    Client local;//本地主机
    QVector<Client> existClients;//服务器中注册的主机
    QQueue<Client> waitingClients;//下载任务中待分配的主机
    QVector<Client> workingClients;//任务进行中的主机
    QVector<mainRecord> taskTable;//进行中的任务分配表
    QVector<historyRecord> historyTable;//历史记录表
    ClientStatus status;

    mission myMission;//待下载文件信息结构体
    //TODO:配置文件
    QString hostName;
    QString pwd;

    //作为伙伴机
    //登记friendID
    qint32 friendId;
    QVector<partnerTask> sliceScheduler;

    //作为朋友机
    //本地下载任务
    QVector<mainRecord> localRecordLists;
    //辅助下载变量
    qint32 clientNum;//参与下载的书籍数量
    qint32 blockSize;

    //工具类
    UDPSocketUtil * udpSocketUtil;
    TCPSocketUtil * tcpSocketUtil;
    mainCtrlUtil * mainctrlutil;
    DownloadManager * downloadManager;
    MsgUtil * msgUtil;


public:
    mainctrl();

    /**
     * @brief regLocalClients 将本机信息注册到服务器
     * @return
     */
    bool regLocalClients();

    /**
     * @brief getExistClient 询问服务器当前注册主机信息
     */
    void getExistClients();
    void initExistClients();

    //统一进行信号槽连接
    void signalsConnect();

    /*———————————朋友端（请求发起端）方法—————————————*/
    /**
     * @brief initWaitingClients 初始化空闲主机队列
     * @return
     */
    bool initWaitingClients();
    bool partnerAccept(qint32 partnerId);//加入waitingClients
    bool partnerReject(qint32 partnerId);

    /**
     * @brief createMission 创建下载任务，检测url可达性，获取文件大小，设置存储路径
     * @param url
     * @param savePath 下载文件的存储路径，默认在软件中创建tmp文件夹
     * @return
     */
    bool createMission(QString url,QString savePath="./tmp",QString missionName="temp");

    /**
     * @brief creatDownloadReq 发起下载请求
     * 请求分如下步骤：
     * 1. 询问伙伴机
     * 2. 初始化 hostNum, blockSize
     * 3. 创建任务块队列
     * @return
     */
    bool creatDownloadReq();

    /**
     * @brief downLoadSchedule
     * 下载管理，最核心的任务调度
     */
    void downLoadSchedule();

    //注册任务
    void addToTaskTable(QVector<mainRecord> recordLists);
    //删除任务,调用addToHistoryTable，将任务登记为历史记录
    //响应伙伴机信号或自身下载完成信号
    void deleteFromTaskTableLocal(qint32 clientID);
    void deleteFromTaskTablePartner(qint32 clientID);
    //增加历史记录
    void addToHistoryTable(historyRecord &hRecord);
    //查询历史记录
    void searchHistoryTable();
    //根据client的能力（taskNum），从blockQueue中取出对应数量的block
    QVector<blockInfo> getTaskBlocks(quint8 taskNum);
    //检查blcok是否连续，创建任务记录
    QVector<mainRecord> createTaskRecord(QVector<blockInfo> blockLists,qint32 clientId);
    //分配任务，发消息给伙伴机，token从reacordLists中取
    void assignTaskToPartner(qint32 partnerID,QVector<mainRecord> recordLists);
    //分配任务给本机，执行下载
    void assignTaskToLocal();
    //TASKEXECUING 接收到伙伴机文件分片,发送THANKYOURHELP
    void recParnterSlice(qint32 partnerId, qint32 token, qint32 index);
    //从工作队列挪到空闲队列
    void work2wait(qint32 clientId);
    //（上层封装，调用taskEndConfig）本地主机完成当前任务
    void taskEndAsLocal();
    //从任务表中删除记录，确认任务完成，将Partner转移至空闲队列
    void taskEndConfig(qint32 clientId);

    /*———————————伙伴端（协助下载端）方法—————————————*/
    //收到求助请求,选择是否下载
    void recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax);
    //接受DOWNLOADTASK，开始task下载
    void taskStartAsPartner(qint32 friendId, qint32 token, qint64 pos, qint32 len);
    //task下载完成，向主机发送TASKFINISH
    void taskEndAsPartner(qint32 friendId, qint32 token,qint32 len);
    //mission完成，状态置为空闲
    //TODO：删除协助下载的文件块
    void missionEndAsPartner();
    //slice分片调度器，切分并发送slice.
    void sliceDivideAndSent(qint32 token,qint32 expectIndex);

    /*——————————————————————槽————————————————————*/
public slots:
    void statusToIDLE(){
        qDebug()<<this->local.getName()<<"status turn to iding";
        this->status=ClientStatus::IDLING;
    }
    void statusTOOFFLINE(){
        this->status=ClientStatus::OFFLINE;
    }

signals:
    //调用assignTaskToLocal，执行本地下载
    void callAssignTaskToLocal();
    //本地下载完成，调用taskEndAsLocal处理相关任务表、状态的变更
    void callTaskEndAsLocal();


    //唤醒sliceDivideScheduler进行调度
    void callSliceScheduler(qint32 token,qint32 expectIndex);
    //分片下载完成，调用taskEndAsPartner
    void callTaskEndAsPartner(qint32 friendId, qint32 token, qint32 len);
};


#endif // MAINCTRL_H
