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
#include <QTextEdit>
#include <QSpinBox>
#include "MemoryDetailDialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool kontrol =false;

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
    QHBoxLayout *chartRow;
    QTextEdit *mapContentView;
    QSpinBox *greenMinSpin;
    QSpinBox *yellowMinSpin;
    QSpinBox *thresholdSpin;

    void analyzeFile(const QString &filePath);
    void showPieChart(QtCharts::QChartView *view, const QString &title, double used, double total);
    void initializeMemoryTable();
    void updateMemoryTable();
    void showCharts();
    void openUserGuide();
    void openMapFullScreen();

    QLabel* teiLogoLabel; // TEI logosu için QLabel


    MemoryStats lastStats;
    ClickableLabel *dropLabel;

    void openFile(const QString &filePath);
    void updateCharts(const QVector<QString> &lines);

private slots:
    void openFileDialog();
    void exportToExcel();
};
