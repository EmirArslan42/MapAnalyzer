#ifndef MEMORYDETAILDIALOG_H
#define MEMORYDETAILDIALOG_H

#include <QDialog>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChartView>

class MemoryDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit MemoryDetailDialog(const QString &type, double used, double total, QWidget *parent = nullptr);
};

#endif // MEMORYDETAILDIALOG_H
