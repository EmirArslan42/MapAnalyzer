#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "MapParser.h"
#include <QLabel>
#include "clickablelabel.h"
#include <QTableWidget>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QVBoxLayout *mainLayout;
    QtCharts::QChartView *stackChartView, *flashChartView, *ramChartView;
    QTableWidget *memoryTable;
    QPushButton *showChartsButton;
    void setupCharts();
    QHBoxLayout *chartRow;  // Bu satırı ekleyin


    void analyzeFile(const QString &filePath);
    void showPieChart(QtCharts::QChartView *view, const QString &title, double used, double total);
    void initializeMemoryTable();
    void updateMemoryTable();
    void showCharts();

    MemoryStats lastStats; // analiz edilen son veriyi tut
    ClickableLabel *dropLabel;

    void openFile(const QString &filePath);
    void updateCharts(const QVector<QString> &lines);

private slots:
    void openFileDialog();
    void exportToExcel();
};
