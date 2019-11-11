#ifndef MAINCTRLMACRO_H
#define MAINCTRLMACRO_H

#include<qvector.h>
#include<qstring.h>
#include<QTime>

/**
 * @brief The blockInfo struct
 * 文件分块的信息
 */
struct blockInfo
{
    quint8 index;
    bool isEndBlock;
};

//单个文件块大小上限
//TODO: 以什么为单位
const quint32 MAXBLOCKSIZE=1024;
//任务下载时长上限
//TODO: 单位确认
const quint32 DDL=102400;
//初始下载任务数量
const quint32 INITTASKNUM=1;

#endif // MAINCTRLMACRO_H
