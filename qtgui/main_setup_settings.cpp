#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"

void MainWindow::setupSettings()
{
  connect(ui->CWSpeed_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setCWSpeed(int)));
  connect(ui->CWSpeed_sli, &QSlider::valueChanged,
	  [=](int s) {
	    ui->CWSpeed_lbl->setText(QString("%1").arg(s, 2));
	  }); 
  connect(ui->Sidetone_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setSidetoneVolume(int)));
  connect(ui->TXPower_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setTXPower(int)));

  connect(ui->FromGrid_le, SIGNAL(textChanged(const QString &)),
	  ui->FromGrid_lab, SLOT(setText(const QString &)));


  connect(ui->FullScreen_tgl, &QRadioButton::toggled, 
	  [=]() 
	  { if(ui->FullScreen_tgl->isChecked()) {
	      // go to full screen
	      //this->setWindowState(Qt::WindowFullScreen);
	      this->showFullScreen();
	    }
	    else {
	      // ungo to full screen
	      this->showMaximized();
	    }
	  });
	      
}
