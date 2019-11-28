#include "mainctrlutil.h"
#include <QDir>
#include <QFile>
#include <QDebug>

mainCtrlUtil::mainCtrlUtil()
{
    startId=nowId=3;
    startTokenId=nowTokenId=24;
    startRecordId=nowRecordId=47;
}

qint32 mainCtrlUtil::createId(){
    return this->nowId++;
}

qint32 mainCtrlUtil::createTokenId(){
    return this->nowTokenId++;
}

qint32 mainCtrlUtil::createRecordId(){
    return this->nowRecordId++;
}

bool mainCtrlUtil::createDirectory(QString dirName,QString savePath){
    QDir tempDir;
    QString oriPath=tempDir.currentPath();
    savePath=savePath.trimmed();
    //绝对路径 及 文件夹内相对路径检查
    //如果filePath路径不存在，创建它
    if(!tempDir.exists(savePath)){
        qDebug()<<"不存在该路径"<<endl;
        return false;
    }
    else{
        tempDir.setCurrent(savePath);
        if(tempDir.mkdir(dirName)){
            qDebug()<<dirName<<"创建成功";
        }
        else{
            qDebug()<<dirName<<"已经存在";
        }
        tempDir.setCurrent(oriPath);//恢复原默认路径
        return true;
    }
}

void mainCtrlUtil::createEmptyFile(QString fileName,QString savePath)
{
    QDir tempDir;
    QString currentDir;
    QString oriPath=tempDir.currentPath();
    QFile *tempFile = new QFile();
    savePath=savePath.trimmed();
    //设定存储路径为当前文件夹
    if(savePath=="./"){
        //临时保存程序当前路径
        currentDir = tempDir.currentPath();
    }
    else{
        //绝对路径 及 文件夹内相对路径检查
        //如果filePath路径不存在，创建它
        if(!tempDir.exists(savePath)){
            qDebug()<<"不存在该路径"<<endl;
            tempDir.mkpath(savePath);
        }
        tempDir.setCurrent(savePath);
    }
    if(!fileName.endsWith(".tmp")){
        fileName+=".tmp";
    }
    if(tempFile->exists(fileName)){
        qDebug()<<"文件已存在"<<fileName;
    }
    else{
        tempFile->setFileName(fileName);
        if(!tempFile->open(QIODevice::WriteOnly|QIODevice::Text))
        {
            qDebug()<<"打开失败";
        }
        tempFile->close();
    }

    tempDir.setCurrent(oriPath);//恢复原默认路径
}

bool mainCtrlUtil::isFileExist(QString fileNameWithPath){
    QFile *tempFile = new QFile();
    if(tempFile->exists(fileNameWithPath)){
        qDebug()<<"mainCtrlUtil::isFileExist 文件存在:"<<fileNameWithPath;
        delete tempFile;
        return true;
    }
    else{
        qDebug()<<"mainCtrlUtil::isFileExist 文件不存在:"<<fileNameWithPath;
        delete tempFile;
        return false;
    }
}

void mainCtrlUtil::clearMissionStruct(mission &m){
    m.url="";m.name="";m.filesize=0;m.savePath="";
}

bool mainCtrlUtil::deleteFile(QString path, QString name){
    QFile file;
    QString fileFullPath=path+name;
    if(mainCtrlUtil::isFileExist(fileFullPath)){
        file.setFileName(fileFullPath);
        if(file.remove()){
            qDebug()<<"mainCtrlUtil::deleteFile  删除文件"<<fileFullPath<<endl;return true;
        }
        else{
            qDebug()<<"mainCtrlUtil::deleteFile  文件删除失败"<<fileFullPath<<endl;return false;
        }
    }
    else{
        qDebug()<<"mainCtrlUtil::deleteFile  删除失败，文件不存在"<<fileFullPath<<endl;return false;
    }
}

partnerSlices* mainCtrlUtil::findParnterTaskSlices(qint32 token,QVector<partnerSlices> &sliceSchedule){
    partnerSlices *task=nullptr;
    for(int i=0;i<sliceSchedule.size();i++){
        if(sliceSchedule[i].token==token){
            task=&sliceSchedule[i];
            break;
        }
    }
    return task;
}

mainRecord* mainCtrlUtil::findTaskRecord(qint32 token, QVector<mainRecord *> &taskTable){
    mainRecord *record=nullptr;
    for(int i=0;i<taskTable.size();i++){
        if(taskTable[i]->getToken()==token){
            record=taskTable[i];
            break;
        }
    }
    return record;
}

bool mainCtrlUtil::isValidMission(mission m){
    //TODO:任务校验
    return true;
}

void mainCtrlUtil::mergeMissionFiles(QVector<historyRecord> historyTable,const QString missionName,const QString filePath){
    qDebug()<<"mainCtrlUtil::mergeMissionFiles  拼接文件"<<filePath<<missionName<<endl;
    QDir tempDir;
    QFile *readFile,*writeFile;
    QString taskFileName;
    QVector<historyRecord>::iterator iter;

    qStableSort(historyTable.begin(),historyTable.end());//按照下载块的下标排序


    if(!tempDir.setCurrent(filePath)){
        qDebug()<<"mainCtrlUtil::mergeMissionFiles  路径无效："<<filePath<<endl;
    }
    else{
        //已设定当前路径
        writeFile=new QFile(missionName);
        writeFile->open(QIODevice::WriteOnly);//创建目标文件
        for(iter=historyTable.begin();iter!=historyTable.end();iter++){
            taskFileName=QString::number(iter->token)+".tmp";
            if(this->isFileExist(taskFileName)){
                //task文件存在，开始写入
                qDebug()<<"mainCtrlUtil::mergeMissionFiles  task文件开始拼接："<<taskFileName<<endl;
                readFile=new QFile(taskFileName);
                readFile->open(QIODevice::ReadOnly);
                writeFile->write(readFile->readAll());
                readFile->close();
                delete readFile;
            }
            else{
                qDebug()<<"mainCtrlUtil::mergeMissionFiles  task文件未找到，拼接失败："<<taskFileName<<endl;
            }
        }
        qDebug()<<"mainCtrlUtil::mergeMissionFiles  mission拼接完成："<<missionName<<endl;
        writeFile->close();
    }

}

bool mainCtrlUtil::missionIntegrityCheck(const QVector<historyRecord> &historyTable,
                                         const QString missionName,const QString filePath,const qint32 fileSize){
    bool validation=false;
    this->mergeMissionFiles(historyTable,missionName,filePath);//合并文件
    //获取文件信息
    QFileInfo *temp=new QFileInfo(filePath+missionName);
    if(temp->size()==fileSize){
        validation=true;
    }
    qDebug()<<"mainCtrlUtil::missionIntegrityCheck  检查Mission长度是否匹配："<<validation<<endl;
    delete temp;
    return validation;
}

