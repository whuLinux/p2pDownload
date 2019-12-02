#ifndef MSGUTIL_H
#define MSGUTIL_H

#include "ctrlmsg.h"
#include "commmsg.h"
#include "filemsg.h"
#include "uniformlabel.h"

class MsgUtil
{
public:
    MsgUtil();

    /**
     * @brief createCtrlMsg
     */
    CtrlMsg createLoginMsg(QString hostName, QString pwd, quint16 port, quint16 filePort);
    CtrlMsg createLogoutMsg(QString hostName, QString pwd);
    CtrlMsg createObtainAllPartners(QString hostName, QString pwd);
    CtrlMsg createP2PTrans(QString hostName, QString pwd, QString partnerName);

    /**
     * @brief createCommonMsg
     */
    CommMsg createP2PPunchMsg();
    CommMsg createAreYouAliveMsg();
    CommMsg createIsAliveMsg(double rate);
    CommMsg createAskForHelpMsg(QString downloadAddress, qint32 lenMax);
    CommMsg creteAgreeToHelpMsg();
    CommMsg creteRefuseToHelpMsg();
    CommMsg createDownloadTaskMsg(qint32 token, qint64 pos, qint32 len);
    CommMsg createTaskFinishMsg(qint32 token);
    CommMsg creteTaskFailureMsg(qint32 token);
    CommMsg createThankYourHelpMsg(qint32 token, qint32 index);
    CommMsg createEndYourHelpMsg();

    /**
     * @brief createFileMsg
     */
    FileMsg createTaskExecuingMsg(qint32 token, qint32 index, qint8 lastOne, QByteArray & msg);
};

#endif // MSGUTIL_H
