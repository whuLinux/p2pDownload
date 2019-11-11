#ifndef HOST_H
#define HOST_H
#include<qstring.h>

class host
{
    //主控模块中空闲主机队列的主机
    //记录主机信息
public:
    host();

private:
    qint32 hostID;//主机id。空闲主机队列中以0标记请求发起机，其他id由与服务器的连接进行分配
    quint8 taskNum;//本轮分配的下载任务数量

};

#endif // HOST_H
