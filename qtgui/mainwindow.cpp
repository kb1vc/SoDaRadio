#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>

#include <QString>

#include "soda_comboboxes.h"
#include "soda_listener.h"
#include "../common/GuiParams.hxx"

MainWindow::MainWindow(QWidget *parent, SoDa::GuiParams & params) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // setup the listener. 
    listener = new SoDaListener(this, QString::fromStdString(params.getServerSocketBasename())); 

    setupSpectrum();
    setupWaterFall();
    
    setupTopControls();
    setupMidControls();
    setupLogGPS();

    setupSettings();
    setupBandConfig();
    setupLogEditor();

    QStringList headers;
    headers << "Date" << "Time" << "From Call" << "From Grid" << "To Call" << "To Grid"
            << "Mode" << "Comment";
    ui->LogView->setKeys(headers);

    connect(ui->LogView, &LogTable::entryUpdated,
            [](int row, std::string key, std::string val) {
        std::cerr << boost::format("row = %d key = [%s] val = [%s]\n") % row % key % val;
    });

    connect(this, SIGNAL(closeRadio()), listener, SLOT(closeRadio()));

    connect(listener, SIGNAL(repHWMBVersion(const QString &)), 
	    this, SLOT(setWindowTitle(const QString &)));

    listener->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

