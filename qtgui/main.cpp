#include "mainwindow.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QObject::connect(&a, SIGNAL(lastWindowClosed()),
                     &w, SLOT(closeRadio()));

    return a.exec();
}
