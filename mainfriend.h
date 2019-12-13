#ifndef MAINFRIEND_H
#define MAINFRIEND_H

#include<QQueue>
#include<QTimer>
#include <QEventLoop>
#include"mainctrlmacro.h"
#include"mainrole.h"
#include"mainrecord.h"
#include"client.h"

/**
 * @brief The MainFriend class
 * @author Vincent Xue
 * 作为friedn朋友机，即请求发起机，执行下载
 */
class MainFriend:public MainRole
{
    Q_OBJECT

private:
    //TODO:使用配置文件
    QString hostName;//登记在服务器的名字与密码
    QString pwd;
    Client *local;//本地主机
    QTimer *loginTimer;
    //本地下载任务
    QVector<mainRecord*> localRecordLists;
    //下载控制
    qint32 clientNum;//参与下载的主机数量
    QVector<blockInfo> blockQueue;
    QQueue<Client*> waitingClients;//下载任务中待分配的主机
    QVector<Client*> workingClients;//任务进行中的主机
    //TODO:检查、释放mainRecord指针
    QVector<mainRecord*> taskTable;//进行中的任务分配表
    QVector<historyRecord> historyTable;//历史记录表

public:
    MainFriend();
    MainFriend(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
               mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil);
    ~MainFriend();//退出登录


    /**
     * @brief regLocalClients 将本机信息注册到服务器
     * @return
     */
    void regLocalClients();

    /**
     * @brief logoutLocalClients
     * 退出登录，向服务器发送logout msg
     */
    void logoutLocalClients();

    /**
     * @brief getExistClient 询问服务器当前注册主机信息
     */
    void getExistClients();

    /**
     * @brief initWaitingClients 初始化空闲主机队列
     * @return
     */
    bool initWaitingClients();

    /**
     * @brief createMission 创建下载任务，检测url可达性，获取文件大小，设置存储路径
     * @param url
     * @param savePath 下载文件的存储路径，默认在软件中创建tmp文件夹
     * @return
     */
    bool createMission(QString url,QString savePath="./tmp",QString missionName="temp");

    /**
     * @brief creatDownloadReq 发起下载请求
     * 1. 初始化 hostNum, blockSize
     * 2. 创建任务块队列
     * 3. callDownloadSchedule
     * @return
     */
    bool createDownloadReq();

    //p2pTrans 向设定数量内的伙伴机发送打洞请求
    void sendPunchToPartners();

    //注册任务
    void addToTaskTable(QVector<mainRecord*> recordLists);

    //根据progress调整任务、taskNum，若超过50%则继续；否则减半该主机taskNum，废弃本次任务，并放回等待队列
    void adjustLocalTask(mainRecord *record,double progress);

    //删除taskTable中对应token任务,调用addToHistoryTable，将任务登记为历史记录,响应伙伴机信号或自身下载完成信号
    void deleteFromTaskTable(qint32 clientID,qint32 token);

    //增加历史记录
    void addToHistoryTable(historyRecord &hRecord);

    //查询历史记录
    void searchHistoryTable();

    //根据client的能力（taskNum），从blockQueue中取出对应数量的block
    QVector<blockInfo> getTaskBlocks(quint8 taskNum);

    //检查blcok是否连续，创建任务记录
    QVector<mainRecord*> createTaskRecord(QVector<blockInfo> blockLists,qint32 clientId);

    //分配任务，发消息给伙伴机，token从reacordLists中取
    void assignTaskToPartner(qint32 partnerID,QVector<mainRecord*> recordLists);

    //从工作队列挪到空闲队列
    void work2wait(qint32 clientId);



    QString getHostName() const;
    QString getPwd() const;

public slots:
    void setHostName(const QString &value);
    void setPwd(const QString &value);

    /**
     * @brief downLoadSchedule
     * 下载管理，最核心的任务调度
     */
    void downLoadSchedule();

    //登录检查
    bool checkLoginStatus();

    //响应服务器回复主机信息
    void initExistClients();

    //接收punch信号，调整existClient的punchSuccess bool值
    void recPunchFromPartner(qint32 partnerId);

    //伙伴机是否响应帮助
    bool partnerAccept(qint32 partnerId);//加入waitingClients
    bool partnerReject(qint32 partnerId);

    //TASKEXECUING 接收到伙伴机文件分片,发送THANKYOURHELP
    void recPartnerSlice(qint32 partnerId, qint32 token, qint32 index);

    //接收超时的任务信号，检查任务进度
    void checkTimeOutTask(qint32 token);

    //接收伙伴机进度,若超过50%则继续；否则减半该主机taskNum，废弃本次任务，并放回等待队列
    void recPartnerProgress(qint32 partnerId,double progress);

    //从任务表中删除记录，确认任务完成，将Partner转移至空闲队列
    void taskEndConfig(qint32 clientId,qint32 token);

    //分配任务给本机，执行下载
    void assignTaskToLocal();

    //本地主机完成1个task，在taskTable、localRecordList中删除记录，
    //并且发callAssignTaskToLocal开始后续可能的下载任务
    void taskEndAsLocal();

    //接收文件最终状态信息
    void recMissionValidation(bool success);

signals:
    //进行下载调度 唤起下载调度方法
    void callDownLoadSchedule();

    //将建立好的exist client lists复制给同主机的mainPartner对象
    void copyExistClientsToMainPartner(QVector<Client*> existClients);

    //调用assignTaskToLocal，执行本地下载
    void callAssignTaskToLocal();

    //本地下载完成，调用taskEndAsLocal处理相关任务表、状态的变更
    void callTaskEndAsLocal();

    //全部文件下载完成，调用missionIntegrityCheck
    void callMissionIntegrityCheck(QVector<historyRecord> historyTable,QString missionName, QString filePath,qint32 fileSize);

};

#endif // MAINFRIEND_H
