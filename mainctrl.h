#ifndef MAINCTRL_H
#define MAINCTRL_H
#include<algorithm>
#include<QQueue>
#include<QDebug>
#include"udpsocketutil.h"
#include"tcpsocketutil.h"
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
    QQueue<Client> workingClients;//任务进行中的主机
    QVector<mainRecord> taskTable;//进行中的任务分配表
    QVector<historyRecord> historyTable;//历史记录表
    ClientStatus status;

    mission myMission;//待下载文件信息结构体
    //TODO:配置文件
    QString hostName;
    QString pwd;

    //辅助下载变量
    qint32 clientNum;//参与下载的书籍数量
    qint32 blockSize;

    //工具类
    UDPSocketUtil udpSocketUtil;
    TCPSocketUtil tcpSocketUtil;
    mainCtrlUtil mainctrlutil;


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
    void addToTaskTable();
    //删除任务
    void deleteFromTaskTable();
    //增加历史记录
    void addToHistoryTable();
    //根据client的能力（taskNum），从blockQueue中取出对应数量的block
    QVector<blockInfo> getTaskBlocks(quint8 taskNum);
    //检查blcok是否连续，创建任务记录
    QVector<mainRecord> createTaskRecord(QVector<blockInfo> blockLists,qint32 clientId,qint32 token);
    //分配任务，发消息给伙伴机
    void assignTaskToPartner();
    /*———————————伙伴端（协助下载端）方法—————————————*/
    void recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax);//收到求助请求


    /*————————————————槽———————————————————————————*/
public slots:
    void statusToIDLE(){
        this->status=ClientStatus::IDLING;
    }
    void statusTOOFFLINE(){
        this->status=ClientStatus::OFFLINE;
    }
};


#endif // MAINCTRL_H
