#include "MemoryDetailDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>

MemoryDetailDialog::MemoryDetailDialog(const QString &type, double used, double total, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(type + " Detayları");
    setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *infoLabel = new QLabel(QString("%1 Bellek\nKullanılan: %2 KB\nToplam: %3 KB")
                                   .arg(type).arg(used, 0, 'f', 2).arg(total, 0, 'f', 2), this);
    layout->addWidget(infoLabel);

    QtCharts::QChartView *chartView = new QtCharts::QChartView(this);
    layout->addWidget(chartView);

    QtCharts::QPieSeries *series = new QtCharts::QPieSeries();
    series->append("Kullanılan", used);
    series->append("Boş", total - used);

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle(type + " Kullanım Dağılımı");
    chart->legend()->setAlignment(Qt::AlignRight);

    chartView->setChart(chart);
}
