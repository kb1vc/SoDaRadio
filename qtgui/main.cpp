#include "mainwindow.hpp"
#include <QApplication>
#include <iostream>

#include "../common/GuiParams.hxx"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SoDa::GuiParams p(argc, argv);    
    MainWindow w(0, p);
    w.show();


    
    for(int i = 0; i < argc; i++) {
      std::cout << boost::format("Argv[%d] = [%s]\n") % i % argv[i]; 
    }
    QObject::connect(&a, SIGNAL(lastWindowClosed()),
                     &w, SLOT(closeRadio()));

    return a.exec();
}
