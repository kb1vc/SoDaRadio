#include "logtable.h"
#include <iostream>

LogTable::LogTable(QWidget *parent) :
    QTableWidget(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    setShowGrid(true);

    connect(this, &LogTable::cellChanged,
            [this](int row, int col) {
    emit entryUpdated(row, this->current_headers.at(col).toStdString(), this->item(row, col)->text().toStdString()); });
}

void LogTable::setKeys(QStringList headers)
{
    current_headers = headers; // save the header list -- they'll be used for keys.
    setColumnCount(headers.size());
    setHorizontalHeaderLabels(headers);
}

LogTable::~LogTable()
{
}
