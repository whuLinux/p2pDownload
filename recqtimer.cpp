#include "Recqtimer.h"

RecQTimer::RecQTimer()
{
    connect(this,SIGNAL(timeout()),this,SLOT(acceptTimeOut()));
}

void RecQTimer::acceptTimeOut()
{
    emit recordTimeOut(this->token);
}


