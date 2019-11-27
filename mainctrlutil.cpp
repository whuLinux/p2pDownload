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

partnerTask* mainCtrlUtil::findParnterTask(qint32 token, QVector<partnerTask> sliceSchedule){
    partnerTask *task=nullptr;
    for(int i=0;i<sliceSchedule.size();i++){
        if(sliceSchedule[i].token==token){
            task=&sliceSchedule[i];
            break;
        }
    }
    return task;
}


bool mainCtrlUtil::isValidMission(mission m){
    //TODO:任务校验
    return true;
}

