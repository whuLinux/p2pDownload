#include "test1.h"
#include"mainctrlmacro.h"
test1::test1()
{

}

void test1::runTest(){
    t.signalsConnect();
    Client * client = new Client(11,"bar","192.168.43.111",DEFAULTPORT,DEFAULTFILEPORT);
    CommMsg msg=t.partner->msgUtil->createP2PPunchMsg();
//    t.partner->tcpSocketUtil->addClient(client, 8809, 8810);
    while(true){
        t.partner->tcpSocketUtil->sendToFriend(11,msg);
    }
//    t.local->addClientToExist(client);
//    t.local->createMission("http://www.baidu.com/index.html");
//    t.local->initWaitingClients();
//    t.local->creatDownloadReq();
//    t.local->downLoadSchedule();
//    t.getLocal()
//    t.getLocal()->regLocalClients();
}
