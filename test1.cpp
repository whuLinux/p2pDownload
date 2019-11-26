#include "test1.h"

test1::test1()
{

}

void test1::runTest(){
    t.signalsConnect();
    t.regLocalClients();
}
