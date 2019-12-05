#ifndef MAINCTRL_H
#define MAINCTRL_H

#include<QDebug>
#include"mainpartner.h"
#include"mainfriend.h"
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

class mainctrl:public QObject
{
    Q_OBJECT

private:

    //工具类
    UDPSocketUtil * udpSocketUtil;
    TCPSocketUtil * tcpSocketUtil;
    mainCtrlUtil * mainctrlutil;
    MsgUtil * msgUtil;

public:
    mainctrl();
    //角色
    MainPartner *partner;
    MainFriend *local;

    //统一进行信号槽连接
    void signalsConnect();

    MainPartner *getPartner() const;
    MainFriend *getLocal() const;
};


#endif // MAINCTRL_H
