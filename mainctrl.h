#ifndef MAINCTRL_H
#define MAINCTRL_H
#include<QQueue>
#include<QDebug>
#include"mainrecord.h"
#include"mainctrlmacro.h"
#include"partner.h"

/**
 * @brief The mainctrl class
 * 主控模块，执行下述功能：
 * 1. 主机注册
 * 2. 下载任务分配、调度
 */
class mainctrl
{

private:
    QVector<mainRecord> taskTable;
    QVector<blockInfo> blockQueue;
    QQueue<Partner> waitingHost;
    quint32 hostNum;
    quint32 blockSize;

public:
    mainctrl();
    /**
     * @brief regLocalHost 将本机信息注册到服务器
     * @return
     */
    bool regLocalHost();
    /**
     * @brief initWaitingHost 初始化空闲主机队列
     * @return
     */
    bool initWaitingHost();
    /**
     * @brief creatDownloadReq 发起下载请求
     * 请求分如下步骤：
     * 1. 获取下载文件的大小等信息
     * 2. 询问伙伴机
     * 3. 初始化 hostNum, blockSize
     * 4. 创建任务块队列
     * @return
     */
    bool creatDownloadReq();
    /**
     * @brief downLoadSchedule
     * 下载管理，最核心的任务调度
     */
    void downLoadSchedule();
};


#endif // MAINCTRL_H
