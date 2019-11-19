#include "mainwindow.h"
#include "downloadmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DownloadManager w;
    w.show();
    return a.exec();
}
