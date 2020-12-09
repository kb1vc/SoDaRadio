/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "mainwindow.hpp"
#include <QApplication>
#include <iostream>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTextStream>
#include <QDir>

#include "../common/GuiParams.hxx"
#include <stdlib.h>
/**
 * @brief simple conversion from std::string to QString... 
 *
 * @param st std::string 
 * @return a QString corresponding to the input parameter
 */
static QString ss2QS(const std::string & st)
{
  return QString::fromStdString(st);
}

/**
 * @brief pop up a notification box and then bail out.
 *
 * @param err_msg a descriptive message explaining how we got here. 
 */
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
 * 
 * @param lock_file_name name of exclusive-access lock file to help us detect zombie server processes. 
 * @param p set of command line parameters, some of which are 
 * passed to the server process.
 */
static void startupServer(const QString & lock_file_name, SoDa::GuiParams & p)
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
  
  server_command = QString("%1 --uds_name \"%2\" ").arg(server_name).arg(ss2QS(p.getServerSocketBasename()));

  // now add the uhd args
  QString uhd_args = ss2QS(p.getUHDArgs());
  if(uhd_args != "") server_command += QString("--uhdargs %1 ").arg(uhd_args);
  if(p.getDebugLevel() > 0) server_command += QString("--debug %1 ").arg(p.getDebugLevel());
  QString server_args = ss2QS(p.getServerArgs());
  if(server_args != "") server_command += QString("%1 ").arg(server_args);
  
  
  server_command += QString(" --lockfile %1 ").arg(lock_file_name); 

  QProcess::startDetached(server_command); 

}

/**
 * @brief initialize colors and shades for GUI elements to present
 * a look and feel based on the fusion style, but with dark background. 
 * 
 * code borrowed from https://gist.github.com/skyrpex
 * under his "do whatever you want" license
 * 
 */
void setupLookNFeel()
{
  qApp->setStyle(QStyleFactory::create("fusion"));

  QPalette palette;
  palette.setColor(QPalette::Window, QColor(53,53,53));
  palette.setColor(QPalette::WindowText, Qt::white);
  palette.setColor(QPalette::Base, QColor(15,15,15));
  palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
  palette.setColor(QPalette::ToolTipBase, Qt::lightGray);
  palette.setColor(QPalette::ToolTipText, Qt::blue);
  palette.setColor(QPalette::Text, Qt::white);
  palette.setColor(QPalette::Button, QColor(53,53,53));
  palette.setColor(QPalette::ButtonText, Qt::white);
  palette.setColor(QPalette::BrightText, Qt::red);

  palette.setColor(QPalette::Highlight, QColor(61,79,201).lighter());
  palette.setColor(QPalette::HighlightedText, Qt::black);

  palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);

  qApp->setPalette(palette);
}

bool checkForZombies(const QString & server_lock_filename, const QString & server_socket_base) 
{
  // does the server lock filename exist? 
  if(QFileInfo::exists(server_lock_filename)) {
    QString error_message = QString("%1 exists. \
This is an indication that a zombiefied \
SoDaServer instance is still running.\
<ol>\
<li>Kill the zombie SoDaServer process.</li>\
<li>Delete files that look like this: <p>%2_cmd</p> and <p>%2_wfall</p></li>	\
<li>Delete this file: <p>%1</p></li>					\
<li>Try again.</li>\
</ol>").arg(server_lock_filename).arg(server_socket_base);
    QMessageBox mbox(QMessageBox::Critical, 
		     "Fatal Error", 
		     error_message,
		     QMessageBox::Ok, NULL);
    //  mbox.setDetailedText(err_string); 
    mbox.exec();

    QTextStream qw(stdout);
    
    qw << QString("%1 exists.\nThis is an indication that a zombiefied\n\
SoDaServer instance is still running.\n\n\
1. Kill the zombie SoDaServer process.\n\
2. Delete files that look like this: %2_cmd  and  %2_wfall\n\
3. Delete this file: %1\n\
4. Try again.\n").arg(server_lock_filename).arg(server_socket_base);

    // we should not continue.
    return true; 
  }

  return false; 
}


/**
 * @brief Start the SoDaRadio GUI app and launch the server process
 * 
 * @param argc count of command line args
 * @param argv vector of command line tokens
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SoDa::GuiParams p(argc, argv);    

    if(p.hadNoCommand()) {
      a.quit();
      return 0;
    }
    
    QString apdir =  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);  

    if(!QDir(apdir).exists()) {
      QDir().mkdir(apdir);
    }

    QString uhdargs = QString::fromStdString(p.getUHDArgs());
    QString server_lock_filename = QString("%1/sodaserver_args%2.lock")
      .arg(apdir)
      .arg(uhdargs);
    
    QString ssbn; 
    if(p.getServerSocketBasename() == std::string("")) {
      ssbn = QString("%1/SoDa_%2_").arg(apdir).arg(uhdargs);
      std::string nbn = ssbn.toStdString();
      p.setServerSocketBasename(nbn);
    }
    else {
      ssbn = QString::fromStdString(p.getServerSocketBasename());
    }

    
    if(!checkForZombies(server_lock_filename, ssbn)) {
      startupServer(server_lock_filename, p); 
      
      setupLookNFeel();
    
      MainWindow w(0, p);
      w.show();
      return a.exec();
    }
}
