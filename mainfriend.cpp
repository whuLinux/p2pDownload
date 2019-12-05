#include "mainfriend.h"

QString MainFriend::getHostName() const
{
    return hostName;
}

void MainFriend::setHostName(const QString &value)
{
    qDebug()<<"MainFriend::setHostName  hostName>>"<<value<<endl;
    hostName = value;
}

QString MainFriend::getPwd() const
{
    return pwd;
}

void MainFriend::setPwd(const QString &value)
{
    qDebug()<<"MainFriend::setPwd  Pwd>>"<<value<<endl;
    pwd = value;
}

MainFriend::MainFriend()
{
    
}

MainFriend::MainFriend(UDPSocketUtil *udpSocketUtil,TCPSocketUtil * tcpSocketUtil,
                       mainCtrlUtil * mainctrlutil,MsgUtil * msgUtil):
    MainRole(udpSocketUtil,tcpSocketUtil,mainctrlutil,msgUtil)
{
    this->downloadManager=new DownloadManager();

    //NOTE:UI 美化
    if (!this->tcpSocketUtil->stablishHost() || !this->tcpSocketUtil->stablishFileHost()) {
        qDebug() << "MainFriend::MainFriend " << "TcpSocket 对象建立失败" << endl;
    }

    //将本机加入等待队列，保证无论如何可以执行单机下载
    Client *localhost=new Client(0,"localhost","127.0.0.1",0,0);
    localhost->setPunchSuccess(true);
    this->waitingClients.append(localhost);

}

MainFriend::~MainFriend(){
    CtrlMsg msg=this->msgUtil->createLogoutMsg(this->hostName,this->pwd);
    this->udpSocketUtil->logout(msg);
}

void MainFriend::regLocalClients(){
    if (!this->udpSocketUtil->stablishClient()) {
        qDebug() << "MainFriend::regLocalClients " << "UdpSocket 对象建立失败" << endl;
    }

    qDebug()<<"MainFriend::regLocalClients "<<this->hostName<<this->local.getFilePort();
    CtrlMsg login_msg=this->msgUtil->createLoginMsg(this->hostName,this->pwd,DEFAULTPORT,DEFAULTFILEPORT);
    if(this->udpSocketUtil->login(login_msg))
        qDebug()<<"MainFriend::regLocalClients 连接请求msg 已发送"<<endl;
    else{
        qDebug()<<"MainFriend::regLocalClients 连接请求msg 发送失败"<<endl;
    }
    qDebug()<<"MainFriend::regLocalClients connect::loginOk"<<endl;
    QObject::connect(this->udpSocketUtil,SIGNAL(loginOk()),this,SLOT(statusToIDLE()));
    qDebug()<<"MainFriend::regLocalClients connect::loginAgain"<<endl;
    QObject::connect(this->udpSocketUtil,SIGNAL(loginAgain()),this,SLOT(statusTOOFFLINE()));

    qDebug()<<"MainFriend::regLocalClients 开启计时器，倒计时30s检查登录"<<endl;
    this->loginTimer=new QTimer();
    this->loginTimer->setSingleShot(true);
    this->loginTimer->start(10000);//给10s登录时间响应
    QObject::connect(this->loginTimer,SIGNAL(timeout()),this,SLOT(checkLoginStatus()));

}

bool MainFriend::checkLoginStatus(){
    //TODO:发alert提醒
    delete this->loginTimer;
    this->loginTimer=nullptr;
    if(this->status==ClientStatus::OFFLINE || this->status==ClientStatus::UNKNOWN){
        qDebug()<<"MainFriend::checkLoginStatus 登录失败，请检查信息"<<endl;
        return false;
    }
    else{
        qDebug()<<"MainFriend::checkLoginStatus 登录成功"<<endl;
        return true;
    }
}

void MainFriend::getExistClients(){
    qDebug()<<"MainFriend::getExistClients  "<<"向服务器请求全部clients"<<endl;
    CtrlMsg msg=this->msgUtil->createObtainAllPartners(this->hostName,this->pwd);
    this->udpSocketUtil->obtainAllPartners(msg);
    this->existClients.clear();
    QObject::connect(this->udpSocketUtil,SIGNAL(timeToGetAllPartners()),this,SLOT(initExistClients()));
}

void MainFriend::initExistClients(){
    if(this->status==ClientStatus::IDLING){
        qDebug()<<"MainFriend::initExistClients  friend机处于IDLING，初始化existClients"<<endl;
        QVector<ClientNode> partners=this->udpSocketUtil->getAllPartners();
        QVector<ClientNode>::iterator iter;

        for(iter=partners.begin();iter!=partners.end();iter++){
            Client *temp=new Client(this->mainctrlutil->createId(),iter->name,iter->ip,iter->port,iter->filePort);
            qDebug()<<"MainFriend::initExistClients  创建伙伴机："<<temp->getIP()<<"  "
                   <<temp->getName()<<" ID: "<<temp->getId()<<" Port:"<<temp->getPort()<<endl;
            this->existClients.append(temp);
        }
        //TCP 打洞用
        qDebug()<<"MainFriend::initExistClients  bindClients 执行"<<endl;
        this->tcpSocketUtil->bindClients(this->existClients);

        //复制存在client list给partner模块
        qDebug()<<"MainFriend::initExistClients  copyExistClientsToMainPartner 信号发送"<<endl;
        emit(this->copyExistClientsToMainPartner(this->existClients));
    }
}

void MainFriend::sendPunchToPartners(){
    CtrlMsg msg;
    qDebug()<<"MainFriend::givePunchToPartners making P2PTRANS"<<endl;
    for(int i=0;i<this->existClients.size()&&i<MAXPARTNERSPUNCH;i++){
        msg=this->msgUtil->createP2PTrans(this->hostName,this->pwd,this->existClients[i]->getName());
        this->udpSocketUtil->p2pTrans(msg);
        qDebug()<<"MainFriend::givePunchToPartners partnerName>>"<<this->existClients[i]->getName()<<endl;
    }
}

bool MainFriend::createMission(QString url,QString savePath,QString missionName){
    QString dirName,dirPath;
    //NOTE：debug检查字符串分割
    dirName=savePath.section('/',-1);//取末尾字符串
    dirPath=savePath.section('/',1,-2);
    qDebug()<<"MainFriend::createMission 设置mission url>>"<<url
           <<" | path>>"<<savePath<<" | name>>"<<missionName<<endl;

//    this->myMission.url=url;
//    this->myMission.name=missionName;

    //NOTE: url 格式，访问性检查;存储路径检查
    this->downloadManager->setUrl(url);
    this->myMission.filesize=DownloadManager::getFileSize(url);
    qDebug()<<"MainFriend::createMission 目标文件大小："<<this->myMission.filesize<<endl;
    if(this->myMission.filesize==0){
        //请求出错
        qDebug()<<"MainFriend::createMission ERROR！ 请求下载文件失败";

        return false;
    }
    else{
        if(savePath==""){
            qDebug()<<savePath<<"MainFriend::createMission 采用默认路径";
            this->downloadManager->setPath(savePath);
        }
        else{
            if(!mainCtrlUtil::createDirectory(dirName,dirPath)){
                qDebug()<<"MainFriend::createMission  设置下载路径成功"<<savePath;
    //            this->myMission.savePath=savePath;
                this->downloadManager->setPath(savePath);
            }
            else{
                qDebug()<<savePath<<"MainFriend::createMission ERROR！有误，采用默认路径";
    //            mainCtrlUtil::createDirectory("tmp","./");
    //            this->myMission.savePath="./";
            }
        }
        return true;
    }
}

bool MainFriend::initWaitingClients(){
    //时限内响应的伙伴机，加入队列
    QTime timer;
    QVector<Client*>::iterator iter;
    qint8 tempClientsNum=0;
    CommMsg helpMsg=this->msgUtil->createAskForHelpMsg(this->myMission.url,this->myMission.filesize);
    qDebug()<<"MainFriend::initWaitingClients  向partner发送请求"<<endl;
    //for safety,清空先
    this->waitingClients.clear();
    for(iter=this->existClients.begin();iter!=this->existClients.end();iter++){
        if((*iter)->getPunchSuccess()){
            //已经成功打洞
            qDebug()<<"MainFriend::initWaitingClients askforhelp to partner>>"<<(*iter)->getName()<<(*iter)->getId()<<endl;
            this->tcpSocketUtil->sendToPartner((*iter)->getId(),helpMsg);
            //处理伙伴机响应
            QObject::connect(this->tcpSocketUtil,SIGNAL(timeForFirstTaskForPartner(qint32)),this,SLOT(partnerAccept(qint32)));
            QObject::connect(this->tcpSocketUtil,SIGNAL(refuseToOfferHelpForPartner(qint32)),this,SLOT(partnerReject(qint32)));
        }
    }
    //设定时间循环，等待伙伴机请求
    //NOTE:GUI用户友好，可视化响应请求数量
    timer.start();

    qDebug()<<"MainFriend::initWaitingClients  当前waiting>>"<<this->waitingClients.size()
           <<"当前exist:"<<this->existClients.size();
    while(this->waitingClients.size()<=this->existClients.size()&&
          timer.elapsed()<=10000){
        //未达到伙伴机上限且未达到时间上限，等待
        if(tempClientsNum<this->waitingClients.size()){
            //有新同意的伙伴
            tempClientsNum=this->waitingClients.size();
            timer.restart();
        }
    }

    if(this->waitingClients.size()<=1){
        //仅有本机
        qDebug()<<"MainFriend::initWaitingClients  无伙伴机响应";
        return false;
    }
    else{
        this->status=ClientStatus::DOWNLOADING;
        this->clientNum=this->waitingClients.size();
        qDebug()<<"MainFriend::initWaitingClients  请求成功，当前可供下载clients数量： "<<this->clientNum<<endl;
        return true;
    }
}

void MainFriend::recPunchFromPartner(qint32 partnerId){
    bool found=false;
    for(int i=0;i<this->existClients.size();i++){
        if(this->existClients[i]->getId()==partnerId){
            this->existClients[i]->setPunchSuccess(true);
            found=true;
            break;
        }
    }
    if(!found){
        qDebug()<<"MainFriend::recPunchFromPartner  ERROR!partner not found in existClients>>"<<partnerId<<endl;
    }
}

bool MainFriend::partnerAccept(qint32 partnerId){
    //将同意协助的伙伴加入等待分配任务队列
    QVector<Client*>::iterator iter;
    for(iter=this->existClients.begin();iter!=this->existClients.end();iter++){
        if((*iter)->getId()==partnerId){
            (*iter)->attributeTask();//开始任务标记
            (*iter)->setTaskNum(INITTASKNUM);
            this->workingClients.append(*iter);
            qDebug()<<"MainFriend::partnerAccept  伙伴机同意请求 partnerId>>"<<partnerId<<endl;
            return true;
        }
    }
    qDebug()<<"MainFriend::partnerAccept  ERROR!伙伴机不列表中 partnerId>>"<<partnerId<<endl;
    return false;
}

bool MainFriend::partnerReject(qint32 partnerId){
    //NOTE: UI可视化提示
    qDebug()<<"MainFriend::partnerReject  伙伴机拒绝请求 partnerId>>"<<partnerId<<endl;
    return true;
}

bool MainFriend::creatDownloadReq(){
    qint8 blockNum=0;
    qDebug()<<" MainFriend::creatDownloadReq  创建下载任务"<<endl;
    //寻求伙伴机帮助
    this->initWaitingClients();
    //下载任务信息更新
    if(this->myMission.filesize/this->clientNum<MAXBLOCKSIZE){
        this->blockSize=this->myMission.filesize/this->clientNum;
    }
    else{
        this->blockSize=MAXBLOCKSIZE;
    }

    qDebug()<<"MainFriend::creatDownloadReq  blockSize>>"<<this->blockSize<<endl;
    blockNum=this->myMission.filesize/this->blockSize;
    qDebug()<<"MainFriend::creatDownloadReq  blockNum>>"<<blockNum<<endl;
    for(qint8 i=1;i<=blockNum;i++){
        //创建任务块
        blockInfo *temp=new blockInfo();
        temp->index=i;
        if(i==blockNum)
            temp->isEndBlock=true;
        else temp->isEndBlock=false;
        this->blockQueue.append(*temp);
    }
    qDebug()<<" MainFriend::creatDownloadReq  划分下载任务块，块数共"<<this->blockQueue.size()<<endl;
    return true;
}

void MainFriend::downLoadSchedule(){
    bool flag=false;

    qDebug()<<"MainFriend::downLoadSchedule 下载Mission调度"<<endl;
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
                qDebug()<<"MainFriend::downLoadSchedule "<<"任务完成，等待拼接、检查数据"<<endl;
                flag=true;
                emit(this->callMissionIntegrityCheck(this->historyTable,this->myMission.name,this->downloadManager->getPath(),this->myMission.filesize));
            }
        }
        else{
            //空闲主机队列不空
            if(!this->waitingClients.isEmpty()){
                //分配任务
                QVector<mainRecord*> recordLists;//block下标不连续时，创建多个任务
                Client *client=this->waitingClients.dequeue();
                QVector<blockInfo> taskBlockLists=this->getTaskBlocks(client->getTaskNum());//按照client能力去对应个数的block
                recordLists=this->createTaskRecord(taskBlockLists,client->getId());

                if(client->getId()==0 ||client->getIP()=="127.0.0.1"||client->getName()=="localhost"){
                    //为本地机，执行本地下载任务
                    qDebug()<<"MainFriend::downLoadSchedule local client空闲，分配任务"<<endl;
                    this->downloadManager->setUrl(this->myMission.url);
                    this->localRecordLists=recordLists;
                    this->status=ClientStatus::DOWNLOADING;
                    //发信号让本地执行下载
                    emit(callAssignTaskToLocal());
                }
                else{
                    //给伙伴机分配任务
                    qDebug()<<"MainFriend::downLoadSchedule local client空闲，分配任务 clientID:"<<client->getId()<<endl;
                    this->assignTaskToPartner(client->getId(),recordLists);
                }
                this->addToTaskTable(recordLists);
                this->workingClients.append(client);//加入工作状态
            }
        }
    }

}

void MainFriend::assignTaskToLocal(){
    qDebug()<<"MainFriend::assignTaskToLocal 执行本地下载任务"<<endl;
    if(!this->localRecordLists.isEmpty()){
        mainRecord *record=this->localRecordLists.front();
        QVector<blockInfo> tempBlocks;
        qint64 pos;
        qint64 len;
        QString taskName;
        tempBlocks=record->getBlockIds();
        pos=tempBlocks.constFirst().index * this->blockSize;
        if(tempBlocks.constLast().isEndBlock){
            //如果是最后的块
            len=this->myMission.filesize-pos;
        }
        else{
            len=tempBlocks.size()*this->blockSize;
        }
        taskName=QString::number(record->getToken())+".tmp";
        this->downloadManager->setName(taskName);
        this->downloadManager->setBegin(pos);
        this->downloadManager->setEnd(pos+len-1);
        this->downloadManager->start();
        //NOTE:自定义路径功能尚未开发
        QObject::connect(this->downloadManager,SIGNAL(taskFinished()),this,SLOT(taskEndAsLocal()));
    }
    else{
        //TODO：task finish，将local 主机放回队列
    }
}

QVector<blockInfo> MainFriend::getTaskBlocks(quint8 taskNum){
    QVector<blockInfo> taskBlockLists;
    int countBlock=0;
    int totalBlocks=this->blockQueue.size();
    while(countBlock<totalBlocks&&countBlock<taskNum){
        taskBlockLists.append(this->blockQueue.takeFirst());
        countBlock++;
    }
    return taskBlockLists;
}

QVector<mainRecord*> MainFriend::createTaskRecord(QVector<blockInfo> blockLists, qint32 clientId){
    QVector<mainRecord*> recordLists;//block下标不连续时，创建多个任务
    mainRecord *recordP=new mainRecord();
    blockInfo tempBlock=blockLists.takeFirst();
    int counter=1;
    int preBlockId=-100;
    int totalBlocks=blockLists.size();
    qint64 gap=0;//多个任务记录时，每多一个任务，DDL+gap，以防timer同时到期
    qDebug()<<"MainFriend::createTaskRecord 创建任务记录列表"<<endl;
    while(counter<=totalBlocks){
        if(preBlockId+1!=tempBlock.index){
            //block不连续，旧的blocks创建record，入队；创建新record存储block
            if(recordP->getClientId()!=FAKERECORD){
                recordLists.append(recordP);//先前blocks记录创建
                gap+=RECORDGAP;
            }
            recordP=new mainRecord();
            recordP->setRecordID(this->mainctrlutil->createRecordId());
            recordP->setClientId(clientId);recordP->setToken(this->mainctrlutil->createTokenId());//每个任务创建唯一Id
            recordP->createTimer(DDL+gap,true);//设置计时器并开启
            QObject::connect(recordP,SIGNAL(sendTimeOutToCtrl(qint32)),this,SLOT(checkTimeOutTask(qint32)));
            qDebug()<<"MainFriend::createTaskRecord  connect::sendTimeOutToCtrl 连接计时器"<<endl;
        }
        recordP->addBlockId(tempBlock);
        counter++;
        tempBlock=blockLists.takeFirst();
    }

    //若块一直连续，循环中未能将记录入vector，此处才入
    if(recordP->getClientId()!=FAKERECORD){
        recordLists.append(recordP);//blocks记录创建
    }

    return  recordLists;
}

void MainFriend::addToTaskTable(QVector<mainRecord*> recordLists){
    qDebug()<<"MainFriend::addToTaskTable 将recordLists加入taskTable"<<endl;
    while(!recordLists.isEmpty()){
        this->taskTable.append(recordLists.takeFirst());
    }
}

void MainFriend::deleteFromTaskTable(qint32 clientID,qint32 token){
    QVector<mainRecord*>::iterator iter;
    QVector<blockInfo> tempBlocks;
    blockInfo *tempBlock;
    bool found=false;
    for(iter=this->taskTable.begin();iter!=taskTable.end();){
        if((*iter)->getClientId()==clientID && (*iter)->getToken()==token){
            //插入历史记录表
            historyRecord *hRecord=new historyRecord();
            hRecord->token=(*iter)->getToken();
            hRecord->recordID=(*iter)->getRecordID();
            hRecord->clientID=(*iter)->getClientId();
            tempBlocks=(*iter)->getBlockIds();
            for(int i=0;i<tempBlocks.size();i++){
                tempBlock=new blockInfo(tempBlocks[i]);
                hRecord->blockId.append(*tempBlock); //复制块信息到历史记录中保存
            }
            qDebug()<<"MainFriend::deleteFromTaskTablePartner  删除任务记录：token>>"<<hRecord->token
                   <<" | clientID>>"<<hRecord->clientID<<endl;
            iter=this->taskTable.erase(iter);//销毁记录，取下一个iter指针
            //入历史记录
            this->addToHistoryTable(*hRecord);
            found=true;break;
        }
        else{
            iter++;
        }
    }
    if(!found){
        qDebug()<<"MainFriend::deleteFromTaskTablePartner  删除任务记录失败，任务未找到：token>>"<<token
               <<" | clientID>>"<<clientID<<endl;
    }
}

void MainFriend::addToHistoryTable(historyRecord &hRecord){
    qDebug()<<"MainFriend::addToHistoryTable  记录转入historyTable"<<hRecord.token<<endl;
    this->historyTable.append(hRecord);
}

void MainFriend::assignTaskToPartner(qint32 partnerID,QVector<mainRecord*> recordLists){
    qDebug()<<"MainFriend::assignTaskToPartner  向伙伴机分配Task， partnerId>> "<<partnerID<<endl;
    QVector<mainRecord*>::iterator iter;
    QVector<blockInfo> tempBlocks;
    qint64 pos;
    qint32 len;
    for(iter=recordLists.begin();iter!=recordLists.end();iter++){
        //对每个record创建msg发送
        tempBlocks=(*iter)->getBlockIds();
        pos=tempBlocks.constFirst().index * this->blockSize;//下载起始地址
        if(tempBlocks.constLast().isEndBlock){
            //如果是最后的块
            len=this->myMission.filesize-pos;
        }
        else{
            len=tempBlocks.size()*this->blockSize;
        }
        CommMsg msg=this->msgUtil->createDownloadTaskMsg((*iter)->getToken(),pos,len);
        this->tcpSocketUtil->sendToPartner(partnerID,msg);
        qDebug()<<"MainFriend::assignTaskToPartner  发createDownloadTaskMsg. partnerId>> "<<partnerID<<endl;
    }
}

void MainFriend::adjustLocalTask(mainRecord *record, double progress){
    quint8 taskNums;
    QVector<blockInfo> blocks;
    if(progress<=0.5){
        //中止任务
        qDebug()<<"MainFriend::adjustLocalTask  本地主机进度："<<progress<<" 缓慢，删除任务，重新分配";
        if(this->downloadManager->abort()){
            //下载任务终止成功
            //1.占有blocks归还
            blocks=record->getBlockIds();
            for(int i=0;i<blocks.size();i++){
                this->blockQueue.push_back((blocks[i]));
            }
            //2.下载能力下调
            for(int i=0;i<this->workingClients.size();i++){
                if(this->workingClients[i]->getId()==LOCALID){
                    //找到主机
                    taskNums=this->workingClients[i]->getTaskNum();
                    if(taskNums>=2){
                        this->workingClients[i]->setTaskNum(taskNums/2);
                    }
                }
            }
            //3. 主机入空闲队列
            this->work2wait(LOCALID);
        }
        else{
            qDebug()<<"MainFriend::adjustLocalTask  ERRROR：主机下载临时文件删除失败！"<<this->downloadManager->getName()<<endl;
        }
    }
    else{
        qDebug()<<"MainFriend::adjustLocalTask  本地主机进度："<<progress<<" 重启计时器，等待任务完成";
        record->deleteTimer();record->createTimer(DDL,true);
        QObject::connect(record,SIGNAL(sendTimeOutToCtrl(qint32)),this,SLOT(checkTimeOutTask(qint32)));
        qDebug()<<"MainFriend::adjustLocalTask  connect::sendTimeOutToCtrl 连接计时器"<<endl;
    }
}

void MainFriend::checkTimeOutTask(qint32 token){
    mainRecord *record=this->mainctrlutil->findTaskRecord(token,this->taskTable);
    qDebug()<<"MainFriend::checkTimeOutTask  下载超时：token>>"<<record->getToken()<<" | clientID>>"<<record->getClientId()<<endl;
    //TODO:找找看local ID到底初始化成什么了
    if(record->getClientId()==LOCALID){
        //为朋友机（本地主机）
        this->adjustLocalTask(record,this->downloadManager->getProgress());
    }
    else{
        //为伙伴机，发信号询问进度
        qDebug()<<"MainFriend::checkTimeOutTask 发信号询问伙伴机进度："<<record->getClientId()<<endl;
        CommMsg msg=this->msgUtil->createAreYouAliveMsg();
        this->tcpSocketUtil->sendToPartner(record->getClientId(),msg);
        //置nullptr，便于按照clientId进行无token的record查找
        record->deleteTimer();
    }
}

void MainFriend::recPartnerSlice(qint32 partnerId, qint32 token, qint32 index){
    //收到slice，发送THANKYOURHELP
    qDebug()<<"MainFriend::recPartnerSlice 发送THANKYOURHELP, partner>> "<<partnerId<<" | token>> "<<token<<endl;
    CommMsg msg=this->msgUtil->createThankYourHelpMsg(token,index+1);//期待收的下一个slice index
    this->tcpSocketUtil->sendToPartner(partnerId,msg);
}

void MainFriend::recPartnerProgress(qint32 partnerId,double progress){
    mainRecord *record=nullptr;
    QVector<blockInfo> blocks;
    quint8 taskNums;
    bool found=false;
    //寻找对应record
    for(int i=0;i<this->taskTable.size();i++){
        //trick：通过nullptr来判断响应的是partner具体哪个token的任务
        //超时任务的timer已置nullptr
        if(taskTable[i]->getClientId()==partnerId&&taskTable[i]->isNullTimer()){
            record=taskTable[i];
            break;
        }
    }
    if(record==nullptr){
        qDebug()<<"MainFriend::recPartnerProgress  ERROR！未在taskTable找到partner 的任务，partner："<<partnerId<<endl;
        return;
    }
    qDebug()<<"MainFriend::recPartnerProgress  partner>> "<<partnerId<<" | token>> "<<record->getToken();
    if(progress<=0.5){
        //中止任务
        qDebug()<<"MainFriend::recPartnerProgress  进度："<<progress<<" 缓慢，删除任务，重新分配";
        //TODO: 发消息让伙伴端中断任务，清理已下载文件
        //CommMsg msg AbortDownload Task

        //1.占有blocks归还
        blocks=record->getBlockIds();
        for(int i=0;i<blocks.size();i++){
            this->blockQueue.push_back((blocks[i]));
        }
        //2.下载能力下调
        for(int i=0;i<this->workingClients.size();i++){
            if(this->workingClients[i]->getId()==partnerId){
                //找到主机
                taskNums=this->workingClients[i]->getTaskNum();
                found=true;
                if(taskNums>=2){
                    this->workingClients[i]->setTaskNum(taskNums/2);
                }
            }
        }
        if(!found){
            qDebug()<<"MainFriend::recPartnerProgress  ERROR！partner not found in workingClients>> "<<partnerId<<endl;
            return;
        }
        //3. 主机入空闲队列
        this->work2wait(partnerId);

    }
    else{
        qDebug()<<"MainFriend::recPartnerProgress  本地主机进度："<<progress<<" 重启计时器，等待任务完成";
        record->deleteTimer();record->createTimer(DDL,true);
        QObject::connect(record,SIGNAL(sendTimeOutToCtrl(qint32)),this,SLOT(checkTimeOutTask(qint32)));
        qDebug()<<"MainFriend::recPartnerProgress  connect::sendTimeOutToCtrl 连接计时器"<<endl;
    }

}

void MainFriend::work2wait(qint32 clientId){
    bool found=false;
    for(int i=0;i<this->workingClients.size();i++){
        if(clientId==workingClients[i]->getId()){
            qDebug()<<"MainFriend::work2wait  clientc move from working to waiting>> "<<clientId<<endl;
            this->waitingClients.enqueue(workingClients.takeAt(i));
            found=true;
            break;
        }
    }
    if(!found){
        qDebug()<<"MainFriend::work2wait  ERROR! client not found in workinglist>> "<<clientId<<endl;
    }
}

void MainFriend::taskEndAsLocal(){
    mainRecord *record=this->localRecordLists.takeFirst();
    qDebug()<<"MainFriend::taskEndAsLocal  local完成任务"<<record->getToken();
    this->taskEndConfig(record->getClientId(),record->getToken());//任务表中删除记录
    //唤起本地下载任务分配，检查是否还有下载任务
    emit(this->callAssignTaskToLocal());
    delete record;
}

void MainFriend::taskEndConfig(qint32 clientId,qint32 token){
    //NOTE：完整性检查，出错重发
    bool found=false;
    //任务状态更新
    this->deleteFromTaskTable(clientId,token);
    //伙伴状态更新
    for(int i=0;i<this->taskTable.size();i++){
        if(this->taskTable[i]->getClientId()==clientId){
            //仍有下载任务
            qDebug()<<"MainFriend::taskEndConfig  仍有下载任务，client不空闲。 client>> "<<clientId<<endl;
            found=true;break;
        }
    }
    if(!found){
        //client全部下载任务完成
        qDebug()<<"MainFriend::taskEndConfig  client任务全部完成，移至waiting队列，等待新任务分配。 client>> "<<clientId<<endl;
        this->work2wait(clientId);
    }
}

void MainFriend::recMissionValidation(bool success){
    //TODO:缺少后续处理
    if(!success){
        qDebug()<<"MainFriend::recMissionValidation 文件拼接失败，完整性缺失"<<endl;
    }
    else{
        qDebug()<<"MainFriend::recMissionValidation "<<this->myMission.name<<" 完成"<<endl;
    }
}


