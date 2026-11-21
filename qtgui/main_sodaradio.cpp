/*
Copyright (c) 2017, 2025 Matthew H. Reilly (kb1vc)
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
#include <QDir>
#include <QTemporaryDir>

#include "../common/GuiParams.hxx"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <QLoggingCategory>
#include <QtGlobal>

// Qt's PulseAudio backend probes device formats on its own thread and can
// print "No sample format supported found for device ..." (e.g. for NoMachine's
// nx_remapped_out remap-sink) asynchronously — long after MainWindow ctor
// returns and any stderr redirect is gone. Filter it via a message handler.
static QtMessageHandler g_prev_msg_handler = nullptr;
static void sodaMessageFilter(QtMsgType type, const QMessageLogContext & ctx, const QString & msg) {
  if (msg.contains(QLatin1String("No sample format supported"))) return;
  if (g_prev_msg_handler) g_prev_msg_handler(type, ctx, msg);
}
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
 * @param p set of command line parameters, some of which are
 * passed to the server process.
 */
static QProcess * startupServer(QObject * parent, SoDa::GuiParams & p)
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

  QStringList server_args;

  server_args << "--uds_name" << ss2QS(p.getServerSocketBasename());

  // radio type is a required positional argument to SoDaServer
  QString radio_name = ss2QS(p.getRadioName());
  if(radio_name != "") {
    server_args << radio_name.toUpper();
  }

  // now add the uhd args
  QString uhd_args = ss2QS(p.getUHDArgs());
  if(uhd_args != "") {
    server_args << "--uhdargs" << uhd_args;
  }
  if(p.getDebugLevel() > 0) {
    server_args << "--debug" << QString("%1").arg(p.getDebugLevel());
  }
  
  QString server_extra_args = ss2QS(p.getServerArgs());
  if(server_extra_args != "") {
    for(const QString & arg : server_extra_args.split(' ', Qt::SkipEmptyParts))
      server_args << arg;
  }

  QProcess * process = new QProcess(parent);
  process->setProcessChannelMode(QProcess::ForwardedChannels);
  // Ask the kernel to send SIGTERM to SoDaServer if this GUI process dies
  // for any reason (crash, SIGKILL, etc.), so it never gets orphaned.
  process->setChildProcessModifier([]() {
    prctl(PR_SET_PDEATHSIG, SIGTERM);
  });
  process->start(server_name, server_args);


  return process;
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

/**
 * @brief Start the SoDaRadio GUI app and launch the server process
 * 
 * @param argc count of command line args
 * @param argv vector of command line tokens
 */
int main(int argc, char *argv[])
{
    // Must be set before QApplication so Qt's multimedia plugins see it on load.
    setenv("LIBVA_MESSAGING_LEVEL", "0", 0);

    // Install before QApplication so plugin-load warnings are also filtered.
    g_prev_msg_handler = qInstallMessageHandler(sodaMessageFilter);

    QApplication a(argc, argv);

    SoDa::GuiParams p(argc, argv);

    // Qt logging rules.
    //
    //  - The named-category rules silence background noise from Qt's own
    //    subsystems on every build (FFmpeg backend info line, QPA services,
    //    Wayland text input probes).
    //
    //  - "*.debug=false" silences plain qDebug() in normal operation so
    //    stray dev-trace prints don't reach the console on distros whose
    //    Qt build (e.g. Ubuntu's) doesn't already do this.  We only apply
    //    it when the user has NOT asked for debug output -- "--debug N"
    //    for any N > 0 leaves qDebug enabled, giving an explicit runtime
    //    switch.  Setting QT_LOGGING_RULES at runtime also overrides this
    //    (Qt applies env-var rules after setFilterRules), so power users
    //    can still turn individual categories on and off from the shell.
    QString qt_rules = QStringLiteral(
        "qt.multimedia.ffmpeg=false\n"
        "qt.qpa.services=false\n"
        "qt.qpa.wayland.textinput=false\n");
    if (p.getDebugLevel() == 0) {
        qt_rules += QStringLiteral("*.debug=false\n");
    }
    QLoggingCategory::setFilterRules(qt_rules);

      
    if(p.getDebugLevel() > 0) {
      qInfo().noquote() << QString("parsed command line");
    }
    
    if(p.hadNoCommand()) {
      a.quit();
      return 0;
    }
    
    QTemporaryDir sockDir("/tmp/SoDaRadio-XXXXXX");
    QString ssbn;
    if(p.getServerSocketBasename() == std::string("")) {
      if(!sockDir.isValid()) {
        alertAndExit("Failed to create temporary directory for radio sockets.");
      }
      ssbn = sockDir.path() + "/SoDa_";
      p.setServerSocketBasename(ssbn.toStdString());
    }
    else {
      ssbn = QString::fromStdString(p.getServerSocketBasename());
    }

    if(p.getDebugLevel() > 0) {
      qInfo().noquote() << QString("starting server");
    }

    QProcess * server_proc = startupServer(nullptr, p);

    if(p.getDebugLevel() > 0) {
      qInfo().noquote() << QString("settingup UI");
    }

    setupLookNFeel();

    // libvdpau writes "Failed to open VDPAU backend" directly via fprintf(stderr)
    // when FFmpeg probes hardware acceleration; no env var suppresses it.
    int _saved_stderr = dup(STDERR_FILENO);
    { int _n = open("/dev/null", O_WRONLY); dup2(_n, STDERR_FILENO); close(_n); }
    MainWindow w(0, p);
    fflush(stderr);
    dup2(_saved_stderr, STDERR_FILENO);
    close(_saved_stderr);

    w.show();
    int ret = a.exec();
    // closeEvent already sent STOP via the command socket; give the server
    // time to exit cleanly before we tear down the socket directory.
    if (!server_proc->waitForFinished(3000)) {
      server_proc->terminate();
      if (!server_proc->waitForFinished(2000))
        server_proc->kill();
    }
    return ret;

}
