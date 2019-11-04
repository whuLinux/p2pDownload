#ifndef UNIFORMLABEL_H
#define UNIFORMLABEL_H

#include <QString>

/**
 * @brief The UDPCtrlMsgType enum
 * LOGIN                客户端信息注册到服务器
 * LOGOUT               服务器上客户端信息清空
 * OBTAINALLPARTNERS    客户端申请获取伙伴客户端信息
 * RETURNALLPARTNERS    服务器返回伙伴客户端信息
 * P2PTRANS             请求服务器“打洞”
 * P2PHOLEPACKAGE       服务器向客户端发送，要求此客户端发送UDP打洞包
 */
enum class UDPCtrlMsgType : qint8 { LOGIN, LOGOUT, OBTAINALLPARTNERS, RETURNALLPARTNERS, P2PTRANS, P2PNEEDHOLE };

/**
 * @brief The TCPCtrlMsgType enum
 * 朋友：主动要求下载的客户端，伙伴：被动协助下载的客户端
 * 每台客户端既可以是朋友也可以是伙伴
 * P2PPUNCH             朋友客户端发送的打洞包，接收端应忽略此消息
 * ASKFORHELP           朋友客户端请求伙伴客户端协助下载
 * AGREETOHELP          伙伴客户端同意协助下载
 * REFUSETOHELP         伙伴客户端拒绝协助下载
 * DOWNLOADTASK         下载任务信息
 * TASKFINISH           伙伴客户端通知朋友客户端下载任务执行完成
 * TASKEXECUING         伙伴客户端通知朋友客户端下载任务正在执行
 * TASKFAILURE          伙伴客户端通知朋友客户端下载任务执行失败
 * THANKYOURHELP        朋友客户端通知伙伴客户端已经收到传送的文件
 * ENDYOURHELP          朋友客户端通知伙伴客户端已终止
 */
enum class TCPCtrlMsgType : qint8 { P2PPUNCH, ASKFORHELP, AGREETOHELP, REFUSETOHELP, DOWNLOADTASK, TASKFINISH, TASKEXECUING, TASKFAILURE, THANKYOURHELP, ENDYOURHELP };

/**
 * @brief The clientNode struct
 * 服务器记录客户端信息的结构体
 */
struct clientNode {
    QString name;
    QString ip;
    quint16 port;
};

typedef struct clientNode ClientNode;

/**
 * @brief CommMsg CtrlMsg FileMsg
 * 各类消息json格式传送时的key值
 */
const QString MSGTYPE = "msgType";
const QString ID = "id";
const QString DOWNLOADADDRESS = "downloadAddress";
const QString LENMAX = "lenMax";
const QString TOKEN = "token";
const QString INDEX = "index";
const QString POS = "pos";
const QString LEN = "len";
const QString HOSTNAME = "hostName";
const QString PARTNERNAME = "partnerName";
const QString PARTNERVECTOR = "partnerVector";
const QString IP = "ip";
const QString PORT = "port";
const QString FRIEND = "friend";

#endif // UNIFORMLABEL_H
