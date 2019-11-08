#include "mainwindow.h"
#include "http.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Http w;
    w.show();
    return a.exec();
}
