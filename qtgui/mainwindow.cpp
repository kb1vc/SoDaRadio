#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->RXFreq_lab, &FreqLabel::newFreq,
            this, &MainWindow::newRXFreq);

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

void MainWindow::newRXFreq(double freq)
{
    std::cerr << boost::format("Got new RX frequency: [%15.6lf]\n") % freq;
}

void MainWindow::closeRadio()
{
    std::cerr << "Closing the radio." << std::endl;
}
