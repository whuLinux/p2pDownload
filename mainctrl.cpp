#include "mainctrl.h"

#include <QEventLoop>

mainctrl::mainctrl()
{
    this->status=ClientStatus::UNKNOWN;//初始化本机状态
    this->local=Client(0,"localhost","127.0.0.1",DEFAULTPORT,DEFAULTFILEPORT);
    this->waitingClients.append(this->local);
    this->tcpSocketUtil = new TCPSocketUtil(DEFAULTPORT, DEFAULTFILEPORT, true, true, "", ".txt", 1000);
}

bool mainctrl::regLocalClients(){
    //主机信息
    //TODO:UI 美化
    //配置文件导入端口
    this->tcpSocketUtil->stablishHost();
    this->tcpSocketUtil->stablishFileHost();

    string temp_pwd,temp_hostName;
    cout<<"input your hostname";cin>>temp_hostName;
    cout<<"input password";cin>>temp_pwd;
    this->hostName=QString::fromStdString(temp_hostName);this->pwd=QString::fromStdString(temp_pwd);

    qDebug()<<this->hostName<<this->local.getFilePort();
    //NOTE: 不知道partneport怎么初始化
    CtrlMsg login_msg=this->msgUtil->createLoginMsg(this->hostName,this->pwd,DEFAULTPORT,DEFAULTFILEPORT);
    this->udpSocketUtil->login(login_msg);

    QObject::connect(this->udpSocketUtil,SIGNAL(loginSuccess()),this,SLOT(statusToIDLE()));
    QObject::connect(this->udpSocketUtil,SIGNAL(oginFailure()),this,SLOT(statusTOOFFLINE()));

    while(this->status==ClientStatus::UNKNOWN);//等待服务器响应请求
    if(this->status==ClientStatus::OFFLINE){
        qDebug()<<"登录失败，请检查信息"<<endl;
        return false;
    }
    else{
        qDebug()<<"登录成功"<<endl;
        return true;
    }
}

void mainctrl::getExistClients(){
    CtrlMsg msg=this->msgUtil->createObtainAllPartners();
    this->udpSocketUtil->obtainAllPartners(msg);
    this->existClients.clear();
    QObject::connect(this->udpSocketUtil,SIGNAL(timeToGetAllPartners()),this,SLOT(initExistClients()));
}

void mainctrl::initExistClients(){
    if(this->status==ClientStatus::IDLING){
        QVector<ClientNode> partners=this->udpSocketUtil->getAllPartners();
        QVector<ClientNode>::iterator iter;

        for(iter=partners.begin();iter!=partners.end();iter++){
            Client *temp=new Client(this->mainctrlutil->createId(),iter->name,iter->ip,iter->port,iter->filePort);
            this->existClients.append(*temp);
        }
    }
}

bool mainctrl::createMission(QString url,QString savePath,QString missionName){
    QString dirName,dirPath;
    //NOTE：debug检查字符串分割
    dirName=savePath.section('/',-1);//取末尾字符串
    dirPath=savePath.section('/',1,-2);
    this->myMission.url=url;
    this->myMission.name=missionName;

    //TODO: url 格式，访问性检查;存储路径检查
    this->myMission.filesize=DownloadManager::getFileSize(url);
    if(this->myMission.filesize==0){
        //请求出错
        qDebug()<<"请求文件失败";

        return false;
    }
    else{
        if(!mainCtrlUtil::createDirectory(dirName,dirPath)){
            this->myMission.savePath=savePath;
        }
        else{
            qDebug()<<savePath<<"有误，采用默认路径./tmp";
            mainCtrlUtil::createDirectory("tmp","./");
            this->myMission.savePath="./tmp";
        }

        return true;
    }
}

bool mainctrl::initWaitingClients(){
    //时限内响应的伙伴机，加入队列
    QTime timer;
    QVector<Client>::iterator iter;
    qint8 tempClientsNum=0;
    CommMsg helpMsg=this->msgUtil->createAskForHelpMsg(this->myMission.url,this->myMission.filesize);

    //for safety,清空先
    this->waitingClients.clear();
    for(iter=this->existClients.begin();iter!=this->existClients.end();iter++){
        this->tcpSocketUtil->sendToPartner(iter->getId(),helpMsg);
        //处理伙伴机响应
        QObject::connect(this->tcpSocketUtil,SIGNAL(timeToInitialTaskForPartner(qint32)),this,SLOT(partnerAccept(qint32)));
        QObject::connect(this->tcpSocketUtil,SIGNAL(refuseToOfferHelpForPartner(qint32)),this,SLOT(partnerReject(qint32)));
    }
    //设定时间循环，等待伙伴机请求
    //TODO:GUI用户友好，可视化响应请求数量
    timer.start();
    while(this->waitingClients.size()<=this->existClients.size()&&
          timer.elapsed()<=10000){
        //未达到伙伴机上限且未达到时间上限，等待
        if(tempClientsNum<this->waitingClients.size()){
            //有新同意的伙伴
            tempClientsNum=this->waitingClients.size();
            timer.restart();
        }
    }

    if(this->waitingClients.size()==1){
        qDebug()<<"无伙伴机响应";
        return false;
    }
    else{
        this->status=ClientStatus::DOWNLOADING;
        this->clientNum=this->waitingClients.size();
        return true;
    }
}

bool mainctrl::partnerAccept(qint32 partnerId){
    //将同意协助的伙伴加入等待分配任务队列
    QVector<Client>::iterator iter;
    for(iter=this->existClients.begin();iter!=this->existClients.end();iter++){
        if(iter->getId()==partnerId){
            iter->attributeTask();//开始任务标记
            iter->setTaskNum(INITTASKNUM);
            this->workingClients.append(*iter);
            return true;
        }
    }
    qDebug()<<"ERROR:伙伴机不列表中!";
    return false;
}

bool mainctrl::partnerReject(qint32 partnerId){
    //TODO: UI可视化提示
    return true;
}

bool mainctrl::creatDownloadReq(){

    qint8 blockNum=0;

    //寻求伙伴机帮助
    this->initWaitingClients();
    //下载任务信息更新
    if(this->myMission.filesize/this->clientNum<MAXBLOCKSIZE){
        this->blockSize=this->myMission.filesize/this->clientNum;
    }
    else{
        this->blockSize=MAXBLOCKSIZE;
    }

    blockNum=this->myMission.filesize/this->blockSize;
    for(qint8 i=1;i<=blockNum;i++){
        blockInfo *temp=new blockInfo();
        temp->index=i;
        if(i==blockNum)
            temp->isEndBlock=true;
        else temp->isEndBlock=false;
        this->blockQueue.append(*temp);
    }
    return true;
}

void mainctrl::downLoadSchedule(){
    bool flag=false;
    if(!mainCtrlUtil::isValidMission(this->myMission)){
        qDebug()<<"ERROR:创建下载失败，mission内容不合法"<<this->myMission.url<<this->myMission.savePath;
        //NOTE:状态变化
        this->status=ClientStatus::IDLING;
        return;
    }
    while(flag){
        //检查任务队列
        if(this->blockQueue.isEmpty()){
            if(this->taskTable.isEmpty()){
                //TODO:任务完成，等待请求发起机拼接数据
                flag=true;
            }
            else {
                //TODO:任务分配表异常检测
            }
        }
        else{
            //空闲主机队列不空
            if(!this->waitingClients.isEmpty()){
                //分配任务
                QVector<mainRecord> recordLists;//block下标不连续时，创建多个任务
                Client client=this->waitingClients.dequeue();
                QVector<blockInfo> taskBlockLists=this->getTaskBlocks(client.getTaskNum());//按照client能力去对应个数的block
                recordLists=this->createTaskRecord(taskBlockLists,client.getId());

                if(client.getId()==0 ||client.getIP()=="127.0.0.1"||client.getName()=="localhost"){
                    //为本地机，执行本地下载任务
                    this->downloadManager->setUrl(this->myMission.url);
                    this->localRecordLists=recordLists;
                    this->status=ClientStatus::DOWNLOADING;
                    //发信号让本地执行下载
                    emit(callAssignTaskToLocal());
                }
                else{
                    //给伙伴机分配任务
                    this->assignTaskToPartner(client.getId(),recordLists);
                }
                this->addToTaskTable(recordLists);
                this->workingClients.append(client);//加入工作状态
            }
        }
    }

}

void mainctrl::assignTaskToLocal(){
    if(!this->localRecordLists.isEmpty()){
        mainRecord record=this->localRecordLists.takeFirst();
        QVector<blockInfo> tempBlocks;
        qint64 pos;
        qint32 len;
        QString taskName;
        //对每个record创建msg发送
        tempBlocks=record.getBlockIds();
        if(tempBlocks.constLast().isEndBlock){
            //如果是最后的块
            len=this->myMission.filesize-pos;
        }
        else{
            len=tempBlocks.size()*this->blockSize;
        }
        taskName=QString::number(record.getToken())+".tmp";
        this->downloadManager->setName(taskName);
        this->downloadManager->setBegin(pos);
        this->downloadManager->setEnd(pos+len-1);
        this->downloadManager->start();
        //NOTE:自定义路径功能尚未开发
    }
    else{
        //TODO：task finish，将local 主机放回队列
    }
}

QVector<blockInfo> mainctrl::getTaskBlocks(quint8 taskNum){
    QVector<blockInfo> taskBlockLists;
    int countBlock=0;
    int totalBlocks=this->blockQueue.size();
    while(countBlock<totalBlocks&&countBlock<taskNum){
        taskBlockLists.append(this->blockQueue.takeFirst());
        countBlock++;
    }
    return taskBlockLists;
}

QVector<mainRecord> mainctrl::createTaskRecord(QVector<blockInfo> blockLists, qint32 clientId){
    QVector<mainRecord> recordLists;//block下标不连续时，创建多个任务
    mainRecord *recordP=new mainRecord();
    blockInfo tempBlock=blockLists.takeFirst();
    int counter=1;
    int preBlockId=-100;
    int totalBlocks=blockLists.size();
    while(counter<=totalBlocks){
        if(preBlockId+1!=tempBlock.index){
            //block不连续，旧的blocks创建record，入队；创建新record存储block
            if(recordP->getClientId()!=FAKERECORD){
                recordLists.append(*recordP);//先前blocks记录创建
            }
            recordP=new mainRecord();
            //TODO:启动计时器
            recordP->setRecordID(this->mainctrlutil->createRecordId());
            recordP->setClientId(clientId);recordP->setToken(this->mainctrlutil->createTokenId());//每个任务创建唯一Id
        }
        recordP->addBlockId(tempBlock);
        counter++;
        tempBlock=blockLists.takeFirst();
    }

    return  recordLists;
}

void mainctrl::addToTaskTable(QVector<mainRecord> recordLists){
    while(!recordLists.isEmpty()){
        this->taskTable.append(recordLists.takeFirst());
    }
}

void mainctrl::deleteFromTaskTablePartner(qint32 clientID){
    QVector<mainRecord>::iterator iter;
    QVector<blockInfo> tempBlocks;
    blockInfo *tempBlock;
    for(iter=this->taskTable.begin();iter!=taskTable.end();){
        if(iter->getClientId()==clientID){
            //插入历史记录表
            historyRecord *hRecord=new historyRecord();
            hRecord->token=iter->getToken();
            hRecord->recordID=iter->getRecordID();
            hRecord->clientID=iter->getClientId();
            tempBlocks=iter->getBlockIds();
            for(int i=0;i<tempBlocks.size();i++){
                tempBlock=new blockInfo(tempBlocks[i]);
                hRecord->blockId.append(*tempBlock); //复制块信息到历史记录中保存
            }
            iter=this->taskTable.erase(iter);//销毁

            //入历史记录
            this->addToHistoryTable(*hRecord);
        }
        else{
            iter++;
        }
    }
}

void mainctrl::addToHistoryTable(historyRecord &hRecord){
    this->historyTable.append(hRecord);
}

void mainctrl::assignTaskToPartner(qint32 partnerID,QVector<mainRecord> recordLists){
    QVector<mainRecord>::iterator iter;
    QVector<blockInfo> tempBlocks;
    qint64 pos;
    qint32 len;
    for(iter=recordLists.begin();iter!=recordLists.end();iter++){
        //对每个record创建msg发送
        tempBlocks=iter->getBlockIds();
        pos=tempBlocks.constFirst().index * this->blockSize;//下载起始地址
        if(tempBlocks.constLast().isEndBlock){
            //如果是最后的块
            len=this->myMission.filesize-pos;
        }
        else{
            len=tempBlocks.size()*this->blockSize;
        }
        CommMsg msg=this->msgUtil->createDownloadTaskMsg(iter->getToken(),pos,len);
        this->tcpSocketUtil->sendToPartner(partnerID,msg);
    }
}

void mainctrl::recParnterSlice(qint32 partnerId, qint32 token, qint32 index){
    //收到slice，发送THANKYOURHELP
    CommMsg msg=this->msgUtil->createThankYourHelpMsg(token,index+1);//期待收的下一个slice index
    this->tcpSocketUtil->sendToPartner(partnerId,msg);
}

void mainctrl::work2wait(qint32 clientId){
    for(int i=0;i<this->workingClients.size();i++){
        if(clientId==workingClients[i].getId()){
            this->waitingClients.enqueue(workingClients.takeAt(i));
            break;
        }
    }
}

void mainctrl::taskEndAsLocal(){
    this->taskEndConfig(this->local.getId());
}

void mainctrl::taskEndConfig(qint32 clientId){
    //TODO：完整性检查，出错重发

    //任务状态更新
    this->deleteFromTaskTablePartner(clientId);
    //伙伴状态更新
    this->work2wait(clientId);
}

/*———————————伙伴端（协助下载端）方法—————————————*/

void mainctrl::recFriendHelp(qint32 friendId,QString downloadAddress, qint32 lenMax){
    //TODO: GUI选择是否帮助
    CommMsg msg;
    bool decision=true;
    if(decision){
        msg=this->msgUtil->creteAgreeToHelpMsg();
//        msg=CommMsg(TCPCtrlMsgType::AGREETOHELP);
        this->tcpSocketUtil->sendToFriend(friendId,msg);
        //切换状态，告诉主控准备下载
        this->status=ClientStatus::HELPING;
        this->friendId=friendId;
        this->myMission.url=downloadAddress;//登记下载地址
        this->blockSize=lenMax;//单次下载长度上限
    }
    else{
        msg=this->msgUtil->creteRefuseToHelpMsg();
//        msg=CommMsg(TCPCtrlMsgType::REFUSETOHELP);
        this->tcpSocketUtil->sendToFriend(friendId,msg);
    }
}

void mainctrl::taskStartAsPartner(qint32 friendId, qint32 token, qint64 pos, qint32 len){
    QString taskName=QString::number(token)+".tmp";
    this->downloadManager->setUrl(this->myMission.url);
    this->downloadManager->setBegin(pos);
    this->downloadManager->setEnd(pos+len-1);//转为[begin,end)
    this->downloadManager->setName(taskName);
    //TODO: 下载地址配置
    this->downloadManager->start();
    //下载结束后发信号，并补上newTask.downloadFile的流指针
    emit(this->callTaskEndAsPartner(friendId,token,len));
}

void mainctrl::taskEndAsPartner(qint32 friendId, qint32 token, qint32 len){
    this->friendId=friendId;
    partnerTask *newTask=new partnerTask();
    newTask->token=token;newTask->index=newTask->sentLength=0;newTask->maxLength=len;
    this->sliceScheduler.append(*newTask);

    //下载结束后发信号，并补上newTask.downloadFile的流指针
    emit(this->callSliceScheduler(token,0));
}

void mainctrl::sliceDivideAndSent(qint32 token,qint32 expectIndex){
    int pos,sliceSize;
    qint8 lastOne;
    FileMsg msg;
    partnerTask *task=mainCtrlUtil::findParnterTask(token,this->sliceScheduler);
    QByteArray slice;
    sliceSize=this->tcpSocketUtil->getSliceSize();
    pos=sliceSize*(task->index)+1;
    if(pos+sliceSize>task->maxLength){
        //最后一块
        slice=task->downloadFile->mid(pos,-1);
        lastOne=1;
    }
    else{
        slice=task->downloadFile->mid(pos,blockSize);
        lastOne=0;
    }
    msg=this->msgUtil->createTaskExecuingMsg(task->token,task->index,lastOne,slice);

    this->tcpSocketUtil->sendToFileFriend(this->friendId,msg);
}

void mainctrl::missionEndAsPartner(){
    //TODO:删除本机下载文件
    this->status=ClientStatus::IDLING;
}

/*————————————————————信号槽———————————————————*/
void mainctrl::signalsConnect(){
    //服务器连接成功
    QObject::connect(this->udpSocketUtil,SIGNAL(loginOk),this,SLOT(statusToIDLE));
    //TASKEXECUING 接收伙伴机文件
    QObject::connect(this->tcpSocketUtil,SIGNAL(timeForNextSliceForPartner),this,SLOT(recParnterSlice));
    //TASKFINISH 本轮Task接收完成
    QObject::connect(this->tcpSocketUtil,SIGNAL(timeForNextTaskForPartner),this,SLOT(taskEndConfig));
    //本地下载
    QObject::connect(this,SIGNAL(callAssignTaskToLocal),this,SLOT(assignTaskToLocal));

    //接收本地下载完成的通知
//    QObject::connect(this->downloadManager,SIGNAL(),this)
    //本地下载完成
    QObject::connect(this,SIGNAL(callTaskEndAsLocal),this,SLOT(taskEndAsLocal));



    //DOWNLOADTASK 伙伴机开始下载
    QObject::connect(this->tcpSocketUtil,SIGNAL(starToDownload),this,SLOT(taskStartAsPartner));
    //分片下载完成，伙伴机准备发送
    QObject::connect(this,SIGNAL(callTaskEndAsPartner),this,SLOT(taskEndAsPartner));
    //THANKYOURHELP Task分片分发调度
    QObject::connect(this->tcpSocketUtil,SIGNAL(timeForNextSliceForFriend),this,SLOT(sliceDivideScheduler));
    //ENDYOURHELP 帮助下载的任务结束
    QObject::connect(this->tcpSocketUtil,SIGNAL(taskHasFinishedForFriend),this,SLOT(missionEndAsPartner));
    //唤起slice调度器,发送slice
    QObject::connect(this,SIGNAL(callSliceScheduler),this,SLOT(sliceDivideAndSent));
}
