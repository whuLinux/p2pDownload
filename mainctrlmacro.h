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

    bool operator <(const historyRecord& latter) const{
        //重载使之能升序排序
        return blockId.front().index < latter.blockId.front().index;
    }
};

struct mission{
    QString url;
    qint64 filesize;
    QString savePath;
    QString name;
};

struct partnerSlices{
    //用于slice分发记录
    qint32 token;
    qint32 index;//上一个发送slice的index
    qint32 sentLength;//已发送字节的最大长度
    qint32 maxLength;//task长度
    QByteArray *downloadFile;//下载的文件流
};

struct partnerTask{
    //记录friend机发来的task 信息
    qint32 friendId;
    qint32 token;
    qint64 pos;
    qint32 len;
};


//TODO: 待改为配置文件
//单个文件块大小上限
//TODO: 以什么为单位
const quint32 MAXBLOCKSIZE=1024;
//任务下载时长上限,10 minute
const qint64 DDL=600000;
//连续两个任务指派时，DDL时间增量，1minute
const qint64 RECORDGAP=60000;
//初始下载任务数量
const quint32 INITTASKNUM=1;

//local clientID
const qint32 LOCALID=0;
//本机端口默认值
const quint16 DEFAULTPORT=20086;
const quint16 DEFAULTFILEPORT=20087;
const quint16 DEFAULTUDPPORT=20090;

//伪记录标识
const qint8 FAKERECORD=-3;
const QString LOGPATH="log.txt";

//服务器信息
const QString SERVERIP="106.54.30.157";
const quint16 SERVERPORT=8808;

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
