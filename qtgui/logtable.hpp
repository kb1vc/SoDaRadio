#ifndef LOGTABLE_H
#define LOGTABLE_H

#include <Qt>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QTableWidget>

namespace Ui {
class LogTable;
}

class LogTable : public QTableWidget
{
    Q_OBJECT

public:
    explicit LogTable(QWidget *parent = Q_NULLPTR);

    ~LogTable();

signals:
    void entryUpdated(int row, std::string key, std::string val);

public slots:
    void setKeys(QStringList headers);

protected:
    QStringList current_headers;
};

#endif // LOGTABLE_H
