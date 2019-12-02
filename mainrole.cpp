#include "mainrole.h"

MainRole::MainRole()
{

}

void MainRole::setMissionUrl(QString url){
    qDebug()<<"MainRole::setMissionUrl url>>"<<url<<endl;
    this->myMission.url=url;
}

void MainRole::setMissionName(QString name){
    qDebug()<<"MainRole::setMissionName name>>"<<name<<endl;
    this->myMission.name=name;
}

void MainRole::setMissionPath(QString path){
    qDebug()<<"MainRole::setMissionPath path>>"<<path<<endl;
    this->myMission.savePath=path;
}

MainRole::MainRole(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
                   mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil):
    udpSocketUtil(udpSocketUtil),tcpSocketUtil(tcpSocketUtil),mainctrlutil(mainctrlutil),msgUtil(msgUtil)
{
    this->status=ClientStatus::UNKNOWN;
}

void MainRole::setDownloadPath(QString path){
    this->downloadManager->setPath(path);
}

QString MainRole::getDownloadPath(){
    return this->downloadManager->getPath();
}
