#include "mainwindow.h"
#include"test1.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    test1 *t=new test1();
//    t->runTest();
    return a.exec();
}
