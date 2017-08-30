#include "mainwindow.hpp"
#include <QApplication>
#include <iostream>
#include <QMessageBox>
#include "../common/GuiParams.hxx"

/**
 * @brief simple conversion from std::string to QString... 
 *
 * @param s std::string 
 * @return a QString corresponding to the input parameter
 */
static QString ss2QS(const std::string & st)
{
  return QString::fromStdString(st);
}

static void alertAndExit(const QString & err_msg) 
{
  QMessageBox mbox(QMessageBox::Critical, 
		   "Fatal Error", 
		   err_msg,
		   QMessageBox::Ok, NULL);
  mbox.exec();
  exit(-1);
}

/**
 * @brief start the radio server process
 */
static void startupServer(SoDa::GuiParams & p)
{
  // start the radio server  
  QString server_name;

  if(p.getServerName() != "") {
    server_name = ss2QS(p.getServerName());
  }
  else {
    QString app_dir = QCoreApplication::applicationDirPath();
    server_name = app_dir + "/SoDaServer";     
  }

  if(!QFile::exists(server_name)) {
    alertAndExit(QString("%1 did not find the SoDa Radio server program.\n"
			   "It looked in\n[%2]\n"
			   "Please press OK button to quit.\n\n"
			   "(Though this is -not- OK. "
			 "Send a note when you get a chance to kb1vc@kb1vc.org)").arg(qApp->applicationDisplayName()).arg(server_name));
  }

  // now build the command
  QString server_command;
  // fix a problem with the UBUNTU menu proxy, whatever that is.
  char ub_fix[] = "UBUNTU_MENUPROXY=";
  putenv(ub_fix);
  
  server_command = QString("%1 --uds_name %2 ").arg(server_name).arg(ss2QS(p.getServerSocketBasename()));

  // now add the uhd args
  QString uhd_args = ss2QS(p.getUHDArgs());
  if(uhd_args != "") server_command += QString("--uhdargs %1 --debug %2").arg(uhd_args).arg(p.getDebugLevel());
  
  qDebug() << QString("Starting process with command [%1]").arg(server_command);
  QProcess::startDetached(server_command); 

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
