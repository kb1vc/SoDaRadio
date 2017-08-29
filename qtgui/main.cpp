#include "mainwindow.hpp"
#include <QApplication>
#include <iostream>
#include <QMessageBox>
#include "../common/GuiParams.hxx"

void startupServer(SoDa::GuiParams & p)
{

  // start the radio server  

  QString server_name;

  if(p.getServerName() != "") {
    server_name = QString::fromStdString(p.getServerName());
  }
  else {
    QString app_dir = QCoreApplication::applicationDirPath();
    server_name = app_dir + "/SoDaServer";     
  }

  qDebug() << QString("In startup server -- testing for file [%1]").arg(server_name);
  if(!QFile::exists(server_name)) {
    QMessageBox mbox(QMessageBox::Critical, 
		     "Fatal Error", 
		     QString("%1 can not find the SoDa Radio server program.\n"
			     "It looked in\n[%2]\n"
			     "Please press OK button to quit.\n\n"
			     "(Though this is -not- OK. "
			     "Send a note when you get a chance to kb1vc@kb1vc.org)").arg(qApp->applicationDisplayName()).arg(server_name),
		     QMessageBox::Ok, NULL);
    mbox.exec();
    exit(-1);
  }
  else {
    qDebug() << QString("In startup server -- found file [%1]").arg(server_name);    
    qDebug() << QString("Starting process in [%1]").arg(server_name);
    QProcess::startDetached(server_name); 
  }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SoDa::GuiParams p(argc, argv);    

    startupServer(p); 

    MainWindow w(0, p);
    w.show();
    return a.exec();
}
