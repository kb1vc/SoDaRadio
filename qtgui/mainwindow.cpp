#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.h"
#include "soda_listener.h"

MainWindow::MainWindow(QWidget *parent, QString socket_basename) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // setup the listener. 
    listener = new SoDaListener(this, socket_basename); 

    connect(ui->RXFreq_lab, &FreqLabel::newFreq,
            this, &MainWindow::newFreq);

    setupSpectrum();
    setupWaterFall();

    QStringList headers;
    headers << "Date" << "Time" << "From Call" << "From Grid" << "To Call" << "To Grid"
            << "Mode" << "Comment";
    ui->LogView->setKeys(headers);

    connect(ui->LogView, &LogTable::entryUpdated,
            [](int row, std::string key, std::string val) {
        std::cerr << boost::format("row = %d key = [%s] val = [%s]\n") % row % key % val;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setupSpectrum()
{
  std::cerr << "in setupSpectrum\n";
  ui->spectrum_plt->setRefLevel(10);
  ui->spectrum_plt->setDynamicRange(100.0);
  ui->spectrum_plt->setFreqCenter(144.2e6);
  connect(ui->spectrum_plt, SIGNAL(xClick(double)), this, SLOT(newFreq(double)));
  connect(ui->sp_moveRight_btn, SIGNAL(clicked(bool)), 
	  ui->spectrum_plt, SLOT(scrollRight(bool)));
  connect(ui->sp_moveLeft_btn, SIGNAL(clicked(bool)), 
	  ui->spectrum_plt, SLOT(scrollLeft(bool)));
  connect(ui->sp_dynRange_cb, SIGNAL(valueChanged(double)), 
	  ui->spectrum_plt, SLOT(setDynamicRange(double)));
  connect(ui->sp_freqSpan_cb, SIGNAL(valueChanged(double)), 
	  ui->spectrum_plt, SLOT(setFreqSpanKHz(double)));
  connect(ui->sp_ceilLevel_spb, SIGNAL(valueChanged(int)), 
	  ui->spectrum_plt, SLOT(setRefLevel(int)));

  ui->spectrum_plt->setMarkerOffset(0,1e3);
  ui->spectrum_plt->configureSpectrum(144.2e6, 400e3, 16384);
}

void MainWindow::setupWaterFall()
{
    std::cerr << "in setupSpectrum\n";
    connect(ui->waterfall_plt,SIGNAL(xClick(double)), this, SLOT(newFreq(double)));
    connect(ui->waterfall_plt, &SoDaWFall::xClick, listener, &SoDaListener::setRXFreq);
    connect(ui->wf_moveRight_btn, SIGNAL(clicked(bool)), 
	    ui->waterfall_plt, SLOT(scrollRight(bool)));
    connect(ui->wf_moveLeft_btn, SIGNAL(clicked(bool)), 
	    ui->waterfall_plt, SLOT(scrollLeft(bool)));
    connect(ui->wf_dynRange_cb, SIGNAL(valueChanged(double)), 
	    ui->waterfall_plt, SLOT(setDynamicRange(double)));
    connect(ui->wf_ceilLevel_spb, SIGNAL(valueChanged(int)), 
	    ui->waterfall_plt, SLOT(setRefLevel(int)));
    connect(ui->wf_freqSpan_cb, SIGNAL(valueChanged(double)), 
	    ui->waterfall_plt, SLOT(setFreqSpanKHz(double)));

    ui->waterfall_plt->configureSpectrum(10368.2e6, 200e3, 16384);
    ui->waterfall_plt->setMarkerOffset(-100, 1e3);

    ui->wf_dynRange_cb->setCurrentIndex(0);
    ui->wf_freqSpan_cb->setCurrentIndex(0);
}


void MainWindow::newFreq(double freq)
{
  std::cerr << boost::format("Got new frequency: %15.6lf MHz\n") % (freq * 1e-6) ;
}

void MainWindow::closeRadio()
{
    std::cerr << "Closing the radio." << std::endl;
}
