#include "test1.h"
#include"mainctrlmacro.h"
test1::test1()
{

}

void test1::runTest(){
    t.signalsConnect();
    Client client(11,"foo","192.168.43.117",DEFAULTPORT,DEFAULTFILEPORT);
    t.local->addClientToExist(client);
    t.local->createMission("http://www.baidu.com/index.html");
    t.local->initWaitingClients();
    t.local->creatDownloadReq();
    t.local->downLoadSchedule();
//    t.getLocal()
//    t.getLocal()->regLocalClients();
}
