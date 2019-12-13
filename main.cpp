#include "mainwindow.h"
#include"test1.h"
#include <QApplication>

#include "downloadmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
 //   MainWindow w;
//    w.show();
//    test1 *t=new test1();
//    t->runTest();
    DownloadManager *manager = new DownloadManager(
                QUrl("http://mirror.bit.edu.cn/qtproject/official_"
                     "releases/qtcreator/4.5/4.5.0/qt-creator-open"
                     "source-windows-x86_64-4.5.0.exe"));
    manager->start();

    return a.exec();
}
