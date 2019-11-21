#ifndef MAINCTRLUTIL_H
#define MAINCTRLUTIL_H
#include<qstring.h>
#include"mainctrlmacro.h"

class mainCtrlUtil
{
private:
    qint32 startId;
    qint32 nowId;
    qint32 startTokenId;
    qint32 nowTokenId;
    qint32 startRecordId;
    qint32 nowRecordId;
public:
    mainCtrlUtil();
    //创建clientID
    qint32 createId();
    //创建recordID
    qint32 createRecordId();
    //创建token
    qint32 createTokenId();
    //指定路径下创建文件夹，指定路径不存在导致创建失败return false
    static bool createDirectory(QString dirName,QString savePath);
    //指定路径下创建空文件
    static void createEmptyFile(QString fileName,QString savePath);
    //清空mission结构体
    static void clearMissionStruct(mission &m);
    //检查任务合法性，包括url可访问性，存储路径
    //TODO:实现
    static bool isValidMission(mission m);
};

#endif // MAINCTRLUTIL_H
