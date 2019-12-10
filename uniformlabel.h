#ifndef UNIFORMLABEL_H
#define UNIFORMLABEL_H

#include <QString>

/**
 * @brief The uniformlable classes
 * @authors 余宗宪、薛国潼
 * P2P通信所使用到的一些枚举类型和常量
 */

/**
 * @brief The UDPCtrlMsgType enum
 * LOGIN                客户端信息注册到服务器
 * LOGOUT               服务器上客户端信息清空
 * RENAME               客户端设置的主机名不允许重复
 * LOGINSUCCESS         客户端登陆成功
 * LOGINFAILURE         客户端登陆失败
 * LOGOUTSUCCESS        客户端登出成功
 * LOGOUTFAILURE        客户端登出失败
 * OBTAINALLPARTNERS    客户端申请获取伙伴客户端信息
 * OBTAINSUCCESS        客户端申请获取伙伴客户端信息成功
 * OBTAINFAILURE        客户端申请获取伙伴客户端信息失败
 * RETURNALLPARTNERS    服务器返回伙伴客户端信息
 * P2PTRANS             请求服务器“打洞”
 * P2PHOLEPACKAGE       服务器向客户端发送，要求此客户端发送UDP打洞包
 */
enum class UDPCtrlMsgType : qint8 { LOGIN, LOGOUT, RENAME, LOGINSUCCESS, LOGINFAILURE, LOGOUTSUCCESS, LOGOUTFAILURE, OBTAINALLPARTNERS, OBTAINSUCCESS, OBTAINFAILURE, RETURNALLPARTNERS, P2PTRANS, P2PNEEDHOLE };



/**
 * @brief The TCPCtrlMsgType enum
 * 朋友 主动要求下载的客户端
 * 伙伴 被动协助下载的客户端
 * 每台客户端既可以是朋友也可以是伙伴
 * P2PPUNCH             伙伴客户端发送的打洞包，接收端应忽略此消息
 * AREYOUALIVE          朋友客户端询问伙伴客户端是否存活
 * ISALIVE              伙伴客户端确认存活
 * ASKFORHELP           朋友客户端请求伙伴客户端协助下载
 * AGREETOHELP          伙伴客户端同意协助下载
 * REFUSETOHELP         伙伴客户端拒绝协助下载
 * DOWNLOADTASK         下载任务信息
 * TASKFINISH           伙伴客户端通知朋友客户端下载任务执行完成
 * TASKEXECUING         伙伴客户端通知朋友客户端下载任务正在执行
 * TASKFAILURE          伙伴客户端通知朋友客户端下载任务执行失败
 * ABORTTASK            朋友客户端通知伙伴客户端终止并清除当前下载任务
 * THANKYOURHELP        朋友客户端通知伙伴客户端已经收到传送的文件
 * ENDYOURHELP          朋友客户端通知伙伴客户端已终止
 */
enum class TCPCtrlMsgType : qint8 { P2PPUNCH, AREYOUALIVE, ISALIVE, ASKFORHELP, AGREETOHELP, REFUSETOHELP, DOWNLOADTASK, TASKFINISH, TASKEXECUING, TASKFAILURE, ABORTTASK, THANKYOURHELP, ENDYOURHELP };



/**
 * @brief The clientNode struct
 * 服务器返回客户端信息的结构体
 */
struct clientNode {
    QString name;
    QString ip;
    quint16 port;
    quint16 udpPort;
    quint16 filePort;
};

typedef struct clientNode ClientNode;



/**
 * @brief CommMsg CtrlMsg FileMsg
 * 各类消息json格式传送时的key值
 */
const QString MSGTYPE = "msgType";
const QString ID = "id";
const QString RATE = "rate";
const QString DOWNLOADADDRESS = "downloadAddress";
const QString LENMAX = "lenMax";
const QString TOKEN = "token";
const QString INDEX = "index";
const QString POS = "pos";
const QString LEN = "len";
const QString HOSTNAME = "hostName";
const QString PARTNERNAME = "partnerName";
const QString PARTNERVECTOR = "partnerVector";
const QString PWD = "pwd";
const QString IP = "ip";
const QString PORT = "port";
const QString UDPPORT = "udpPort";
const QString FILEPORT = "filePort";
const QString FRIEND = "friend";



/**
 * @brief 约定好的命名
 * Client 客户机
 * Server 服务器
 * 朋友 主动要求下载的客户端
 * 伙伴 被动协助下载的客户端
 * Host 主动监听端口，接收多个伙伴客户端访问的的TcpServer对象
 * Guest，向某一个朋友客户端发送信息的TcpSocket对象
 *
 * Mission 本次要下载的文件
 * Task client被分配到的任务，每个Task为n倍Block Size
 * Block 主控模块划分任务的最小单位
 * Slice 伙伴机向朋友机发送Task时，受TCP限制而进行的文件分片
 * token 任务令牌，任务的唯一标识
 * index 任务等待P2P传送的文件分块后，块的唯一标识
 *
 * @brief 临时文件命名
 * token.tmp
 */

#endif // UNIFORMLABEL_H
