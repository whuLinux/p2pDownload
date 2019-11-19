#include "mainctrlutil.h"
#include <QDir>
#include <QFile>
#include <QDebug>

mainCtrlUtil::mainCtrlUtil()
{

}

void mainCtrlUtil::createEmptyFile(QString fileName,QString savePath)
{
    QDir tempDir;
    QString currentDir;
    QFile *tempFile = new QFile;
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
}
