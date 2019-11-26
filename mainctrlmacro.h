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
    qint8 index;
    bool isEndBlock;//长度末尾非blocksize整数倍，单独处理
};

/**
 * @brief The historyRecord struct 历史记录，用于文件校验后重新下载
 */
struct historyRecord{
    qint8 recordID; //单个记录标识
    qint32 clientID; //下载主机的标识
    qint32 token; //任务token
    QVector<blockInfo> blockId; //下载文件块的下标
};

struct mission{
    QString url;
    qint64 filesize;
    QString savePath;
    QString name;
};

struct partnerTask{
    qint32 token;
    qint32 index;
    qint32 sentLength;//已发送字节长度
    qint32 maxLength;
    QByteArray *downloadFile;//下载的文件流
};

//TODO: 待改为配置文件

//单个文件块大小上限
//TODO: 以什么为单位
const quint32 MAXBLOCKSIZE=1024;
//任务下载时长上限
//TODO: 单位确认
const quint32 DDL=102400;
//初始下载任务数量
const quint32 INITTASKNUM=1;

const quint16 DEFAULTPORT=10086;
const quint16 DEFAULTFILEPORT=10087;

//伪记录标识
const qint8 FAKERECORD=-3;
const QString LOGPATH="log.txt";

/**
 * @brief The ClientStatus enum
 * client状态
 * DEAD 不在线或异常
 * IDLING 闲置中，无任务
 * HELPING 作为伙伴机协助下载
 * DOWNLOADING 作为朋友机，正在进行下载与调度
 * CHECKING 下载已完成，检查中
 */
enum class ClientStatus:qint8{UNKNOWN,OFFLINE,IDLING,HELPING,CHECKING,DOWNLOADING};


#endif // MAINCTRLMACRO_H
