#include "mainpartner.h"

MainPartner::MainPartner()
{

}

MainPartner::MainPartner(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
                       mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil):
    MainRole(udpSocketUtil,tcpSocketUtil,mainctrlutil,msgUtil)
{
    this->downloadManager=new DownloadManager();

}


void MainPartner::recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax){
    //NOTE: GUI选择是否帮助
    CommMsg msg;
    qDebug()<<"MainPartner::recFriendHelp  "<<"接收伙伴机请求，选择是否协助下载"<<endl;
    qDebug()<<"MainPartner::recFriendHelp  friendId>>"<<friendId
           <<"  |  downloadAddress>>"<<downloadAddress<<" | lenMax>>"<<lenMax<<endl;
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

void MainPartner::recAbortOrder(){
    qDebug()<<"MainPartner::recAbortOrder  终止当前下载任务>>"<<this->downloadManager->getName()<<endl;
    this->downloadManager->abort();
}

void MainPartner::recTaskFromFriend(qint32 friendId, qint32 token, qint64 pos, qint32 len){
    partnerTask newTask;
    newTask.friendId=friendId;newTask.token=token;newTask.pos=pos;newTask.len=len;//记录任务表
    this->partnerTaskLists.append(newTask);
    qDebug()<<"MainPartner::recTaskFromFriend  接收任务 friendId>> "<<friendId<<" | token>> "<<token<<endl;
    if(this->partnerTaskLists.size()==1){
        //伙伴机空闲，当前为第一个下载task，发起下载
        emit(callTaskStartAsPartner());
    }
}

void MainPartner::reportTaskProgress(qint32 friendId){
    double progress=this->downloadManager->getProgress();
    CommMsg msg=this->msgUtil->createIsAliveMsg(progress);
    qDebug()<<"MainPartner::reportTaskProgress 汇报下载进度："<<progress<<" to friend:"<<friendId<<endl;
    this->tcpSocketUtil->sendToFriend(friendId,msg);
}

void MainPartner::taskStartAsPartner(){
    partnerTask newTask=this->partnerTaskLists.first();
    QString taskName=QString::number(newTask.token)+".tmp";
    this->downloadManager->setUrl(this->myMission.url);
    this->downloadManager->setBegin(newTask.pos);
    this->downloadManager->setEnd(newTask.pos+newTask.len-1);//转为[begin,end)
    this->downloadManager->setName(taskName);
    //使用默认存储路径
    qDebug()<<"MainPartner::taskStartAsPartner 开始任务下载 "<<this->downloadManager->getUrl()<<this->downloadManager->getName()<<endl;
    this->downloadManager->start();

    qDebug()<<"MainPartner::taskStartAsPartner connect::taskFinished"<<endl;
    QObject::connect(this->downloadManager,SIGNAL(taskFinished()),this,SLOT(taskEndAsPartner()));
}

void MainPartner::taskEndAsPartner(){
    QString path=this->downloadManager->getPath(),fileName;
    partnerTask finishedTask=this->partnerTaskLists.takeFirst();
    //拼接得待传输文件名
    QFile *file;
    partnerSlices *newTask=new partnerSlices();

    qDebug()<<"MainPartner::taskEndAsPartner task下载完成，从partnerTaskLists弹出，准备发送，token>> "<<finishedTask.token<<endl;
    fileName=path+QString::number(finishedTask.token)+".tmp";
    this->friendId=finishedTask.friendId;
    newTask->token=finishedTask.token;newTask->index=newTask->sentLength=0;newTask->maxLength=finishedTask.len;

    if(this->mainctrlutil->isFileExist(fileName)){
        file=new QFile(fileName);
    }
    else{
        //NOTE:UI回显报错信息
        qDebug()<<"MainPartner::taskEndAsPartner "<<fileName<<"待发送文件不存在，返回"<<endl;
        return;
    }

    file->open(QIODevice::ReadOnly);
    qDebug()<<"MainPartner::taskEndAsPartner 创建待发送QByteArray"<<fileName<<endl;
    newTask->downloadFile=new QByteArray(file->readAll());
    this->sliceScheduler.append(*newTask);

    emit(this->callSliceScheduler(friendId,finishedTask.token,0));
    emit
    file->close(); delete file;
    this->rubbishBin.push_back(finishedTask.token);//token入回收站

    if(!this->partnerTaskLists.isEmpty()){
        //仍有任务待下载
        emit(this->callTaskStartAsPartner());
    }
}

void MainPartner::sliceDivideAndSent(qint32 friendId,qint32 token,qint32 expectIndex){
    //slice index从0开始，expectIndex为期待本次发送slice的index
    int pos,sliceSize;
    qint8 lastOne;
    FileMsg msg;
    partnerSlices *task=this->mainctrlutil->findParnterTaskSlices(token,this->sliceScheduler);
    QByteArray slice;
    sliceSize=this->tcpSocketUtil->getSliceSize();
    pos=sliceSize*expectIndex;//起始地址
    if(pos>task->maxLength){
        //完成传送，发送TaskFinish
        qDebug()<<"MainPartner::sliceDivideAndSent 传输完成，向friend发送TASKFINIHSH msg. friend>> "
               <<friendId<<" | tokend>> "<<token<<endl;
        CommMsg msg=this->msgUtil->createTaskFinishMsg(token);
        this->tcpSocketUtil->sendToFriend(this->friendId,msg);
    }
    else{
        //尚未完成发送
        if(pos+sliceSize>=task->maxLength){
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
}

void MainPartner::missionEndAsPartner(){
    qDebug()<<"MainPartner::missionEndAsPartner  清空全部协助下载文件"<<endl;
    this->status=ClientStatus::IDLING;
    QString path=this->downloadManager->getPath();
    QString fileName;
    for(int i=0;i<this->rubbishBin.size();i++){
        fileName=QString::number(this->rubbishBin[i])+".tmp";
        this->mainctrlutil->deleteFile(path,fileName);
    }
    this->rubbishBin.clear();
}
