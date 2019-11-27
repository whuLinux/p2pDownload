#include "mainpartner.h"

MainPartner::MainPartner()
{

}

MainPartner::MainPartner(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
                       mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil):
    MainRole(udpSocketUtil,tcpSocketUtil,mainctrlutil,msgUtil)
{

}


void MainPartner::recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax){
    //TODO: GUI选择是否帮助
    CommMsg msg;
    qDebug()<<"MainPartner::recFriendHelp  "<<"接收伙伴机请求，选择是否协助下载"<<endl;
    bool decision=true;
    if(decision){
        msg=this->msgUtil->creteAgreeToHelpMsg();
        this->tcpSocketUtil->sendToFriend(friendId,msg);
        //切换状态，告诉主控准备下载
        this->status=ClientStatus::HELPING;
        this->friendId=friendId;
        this->myMission.url=downloadAddress;//登记下载地址
        this->myMission.filesize=lenMax;//单次下载长度上限
        qDebug()<<"MainPartner::recFriendHelp  "<<"接收伙伴机请求，协助下载"<<endl;
    }
    else{
        msg=this->msgUtil->creteRefuseToHelpMsg();
        this->tcpSocketUtil->sendToFriend(friendId,msg);
        qDebug()<<"MainPartner::recFriendHelp  "<<"接收伙伴机请求，拒绝协助"<<endl;
    }
}

void MainPartner::taskStartAsPartner(qint32 friendId, qint32 token, qint64 pos, qint32 len){
    QString taskName=QString::number(token)+".tmp";
    this->downloadManager->setUrl(this->myMission.url);
    this->downloadManager->setBegin(pos);
    this->downloadManager->setEnd(pos+len-1);//转为[begin,end)
    this->downloadManager->setName(taskName);
    //使用默认存储路径
    qDebug()<<"MainPartner::taskStartAsPartner 开始任务下载 "<<this->downloadManager->getUrl()<<this->downloadManager->getName()<<endl;
    this->downloadManager->start();

    //TODO 下载结束后发信号，并补上newTask.downloadFile的流指针
}

void MainPartner::taskEndAsPartner(qint32 friendId, qint32 token, qint32 len){
    QString fileName=this->downloadManager->getPath()+this->downloadManager->getName();//拼接得待传输文件名
    QFile *file;
    partnerTask *newTask=new partnerTask();

    this->friendId=friendId;
    newTask->token=token;newTask->index=newTask->sentLength=0;newTask->maxLength=len;

    if(this->mainctrlutil->isFileExist(fileName)){
        file=new QFile(fileName);
    }
    else{
        //TODO:UI回显报错信息
        qDebug()<<"MainPartner::taskEndAsPartner "<<fileName<<"待发送文件不存在，返回"<<endl;
        return;
    }

    file->open(QIODevice::ReadOnly);
    newTask->downloadFile=new QByteArray(file->readAll());
    this->sliceScheduler.append(*newTask);

    emit(this->callSliceScheduler(friendId,token,0));

    file->close(); delete file;
}

void MainPartner::sliceDivideAndSent(qint32 friendId,qint32 token,qint32 expectIndex){
    int pos,sliceSize;
    qint8 lastOne;
    FileMsg msg;
    partnerTask *task=mainCtrlUtil::findParnterTask(token,this->sliceScheduler);
    QByteArray slice;
    sliceSize=this->tcpSocketUtil->getSliceSize();
    pos=sliceSize*expectIndex+1;
    if(pos+sliceSize>task->maxLength){
        //最后一块
        slice=task->downloadFile->mid(pos,-1);
        lastOne=1;
    }
    else{
        slice=task->downloadFile->mid(pos,sliceSize);//按照tcp最大sliceSize长度发送
        lastOne=0;
    }
    qDebug()<<"MainPartner::sliceDivideAndSent\t"<<"向主机发送slice:"<<task->token<<"(pos:"<<pos<<")"<<endl;
    msg=this->msgUtil->createTaskExecuingMsg(task->token,task->index,lastOne,slice);
    this->tcpSocketUtil->sendToFileFriend(this->friendId,msg);
    //更新task记录
    task->index=expectIndex;
    task->sentLength=pos;
}

void MainPartner::missionEndAsPartner(){
    //TODO:删除本机下载文件
    this->status=ClientStatus::IDLING;
}
