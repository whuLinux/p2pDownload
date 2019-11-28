#include "mainrole.h"

MainRole::MainRole()
{

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
