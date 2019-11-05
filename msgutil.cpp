#include "msgutil.h"

MsgUtil::MsgUtil()
{

}

CtrlMsg MsgUtil::createLoginMsg(QString hostName, QString pwd, QString ip, quint16 port)
{
    CtrlMsg loginMsg(UDPCtrlMsgType::LOGIN);
    loginMsg.setPwd(pwd);
    loginMsg.setIP(ip);
    loginMsg.setPort(port);

    return loginMsg;
}

CtrlMsg MsgUtil::createLogoutMsg(QString hostName, QString pwd)
{
    CtrlMsg logoutMsg(UDPCtrlMsgType::LOGOUT);
    logoutMsg.setHostName(hostName);
    logoutMsg.setPwd(pwd);

    return logoutMsg;
}

CtrlMsg MsgUtil::createObtainAllPartners()
{
    CtrlMsg obtainMsg(UDPCtrlMsgType::OBTAINALLPARTNERS);

    return obtainMsg;
}

CtrlMsg MsgUtil::createReturnAllPartners(ClientNode * clients, int clientNum)
{
    CtrlMsg returnMsg(UDPCtrlMsgType::RETURNALLPARTNERS);

    if (clients == nullptr || clientNum <= 0) {
        qDebug() << "伙伴客户端信息列表有误" << endl;
        return returnMsg;
    }

    for (int i = 0; i < clientNum; i++) {
        returnMsg.addClient(clients[i]);
    }

    returnMsg.setClientNum(clientNum);

    return returnMsg;
}

CtrlMsg MsgUtil::createP2PTrans(QString partnerName)
{
    CtrlMsg transMsg(UDPCtrlMsgType::P2PTRANS);
    transMsg.setPartnerName(partnerName);

    return transMsg;
}

CtrlMsg MsgUtil::createP2PHolePackage(ClientNode client)
{
    CtrlMsg holeMsg(UDPCtrlMsgType::P2PNEEDHOLE);
    holeMsg.addClient(client);

    return holeMsg;
}

CommMsg MsgUtil::createP2PPunchMsg()
{
    CommMsg punchMsg(TCPCtrlMsgType::P2PPUNCH);

    return punchMsg;
}

CommMsg MsgUtil::createAskForhelpMsg(QString downloadAddress, qint32 lenMax)
{
    CommMsg downloadMsg(TCPCtrlMsgType::ASKFORHELP, downloadAddress, lenMax);

    return downloadMsg;
}

CommMsg MsgUtil::creteAgreeToHelpMsg()
{
    CommMsg agreeMsg(TCPCtrlMsgType::AGREETOHELP);

    return agreeMsg;
}

CommMsg MsgUtil::creteRefuseToHelpMsg()
{
    CommMsg refuseMsg(TCPCtrlMsgType::REFUSETOHELP);

    return refuseMsg;
}

CommMsg MsgUtil::createDownloadTaskMsg(qint32 token, qint64 pos, qint32 len)
{
    CommMsg taskMsg(TCPCtrlMsgType::DOWNLOADTASK, token, pos, len);

    return taskMsg;
}

CommMsg MsgUtil::createTaskFinishMsg(qint32 token)
{
    CommMsg finishMsg(TCPCtrlMsgType::TASKFINISH);
    finishMsg.setToken(token);

    return finishMsg;
}

CommMsg MsgUtil::creteTaskFailureMsg(qint32 token)
{
    CommMsg failureMsg(TCPCtrlMsgType::TASKFAILURE);
    failureMsg.setToken(token);

    return failureMsg;
}

CommMsg MsgUtil::createThankYourHelpMsg(qint32 token, qint32 index)
{
    CommMsg thankMsg(TCPCtrlMsgType::THANKYOURHELP, token, index);

    return thankMsg;
}

CommMsg MsgUtil::createEndYourHelpMsg()
{
    CommMsg endMsg(TCPCtrlMsgType::ENDYOURHELP);
    return endMsg;
}

FileMsg createTaskExecuingMsg(qint32 token, qint32 index, QByteArray & msg)
{
    FileMsg execMsg(TCPCtrlMsgType::TASKEXECUING, token, index);
    execMsg.setMsg(msg);

    return execMsg;
}
