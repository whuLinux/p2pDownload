#ifndef MAINROLE_H
#define MAINROLE_H

#include"mainctrlmacro.h"
#include"client.h"
#include<QDebug>
#include"mainctrlutil.h"
#include"udpsocketutil.h"
#include"tcpsocketutil.h"
#include"msgutil.h"
#include"downloadmanager.h"

//TODO:测试用控制台 UI待制作
#include<string>
#include<iostream>  //cin cout
#include<stdio.h> //printf
using namespace std;

class MainRole
{
public:
    ClientStatus status;
    mission myMission;//待下载文件信息结构体
    qint32 blockSize;
    //工具类
    UDPSocketUtil * udpSocketUtil;
    TCPSocketUtil * tcpSocketUtil;
    mainCtrlUtil * mainctrlutil;
    DownloadManager * downloadManager;
    MsgUtil * msgUtil;

public:
    MainRole();
    MainRole(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
             mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil);

    //设置下载路径
    void setDownloadPath(QString path);
    QString getDownloadPath();

public slots:
    void statusToIDLE(){
        qDebug()<<"MainRole::statusToIDLE "<<"status turn to iding";
        this->status=ClientStatus::IDLING;
    }
    void statusTOOFFLINE(){
        qDebug()<<"MainRole::statusToOFFLINE "<<"status turn to offline";
        this->status=ClientStatus::OFFLINE;
    }
};

#endif // MAINROLE_H
