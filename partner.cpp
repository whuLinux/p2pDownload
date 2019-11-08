#include "partner.h"

Partner::Partner()
{

}

Partner::Partner(qint32 id, QString name, QString ip, quint16 port, quint16 filePort) : id(id), name(name), ip(ip), port(port), filePort(filePort)
{

}

Partner::~Partner()
{

}

//bool Partner::attributeTask(bool hasTask, int pos, int len)
//{
//    this->hasTask = hasTask;

//    if (hasTask && pos >= 0 && len > 0) {
//        this->pos = pos;
//        this->len = len;

//        qDebug() << "Partner::attributeTask " << "name " << this->name << " ip " << this->ip << " port " << this->port << " 成功分配任务" << " pos " << this->pos << " len " << this->len << endl;
//        return true;
//    }

//    qDebug() << "Partner::attributeTask " << "name " << this->name << " ip " << this->ip << " port " << this->port << " 分配任务失败" << endl;
//    return false;
//}
