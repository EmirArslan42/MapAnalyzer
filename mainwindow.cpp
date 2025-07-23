#include "mainwindow.h"
#include <QMimeData>
#include <QUrl>
#include <QDebug>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "xlsxdocument.h" // QXlsx
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),chartRow(nullptr) {

    setAcceptDrops(true);
    resize(1000, 600);
    setWindowTitle("Map Analyzer");

    QMenuBar *menuBar = new QMenuBar(this);

    setMenuBar(menuBar);
    QToolBar *toolBar = addToolBar("Araçlar");
      toolBar->addAction(QIcon(":/icons/open.png"), "Dosya Aç", this, &MainWindow::openFileDialog);
      toolBar->addAction(QIcon(":/icons/excel.png"), "Excel'e Kaydet", this, &MainWindow::exportToExcel);
      toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QWidget *central = new QWidget(this);
    mainLayout = new QVBoxLayout(central);

    dropLabel = new ClickableLabel(this);
    dropLabel->setText("📁 Buraya .map dosyasını sürükleyebilirsiniz");
    dropLabel->setAlignment(Qt::AlignCenter);
    dropLabel->setStyleSheet("QLabel { border: 2px dashed #aaa; font-size: 16px; padding: 20px; color: #555; }");
    dropLabel->setFixedHeight(80);
    dropLabel->setAcceptDrops(true);
    mainLayout->addWidget(dropLabel);

    connect(dropLabel, &ClickableLabel::clicked, this, &MainWindow::openFileDialog);

    memoryTable = new QTableWidget(this);
    memoryTable->setColumnCount(5);
    memoryTable->setHorizontalHeaderLabels({"Bellek Türü", "Toplam (KB)", "Kullanılan (KB)", "Boş (KB)", "Kullanım %"});
    memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memoryTable->setStyleSheet("QTableWidget { background-color: #f8f9fa; border: 1px solid #ddd; }"
                               "QHeaderView::section { background-color: #3498db; color: white; padding: 5px; }");
    memoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    memoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    memoryTable->setSelectionMode(QAbstractItemView::SingleSelection);

    initializeMemoryTable();

    QPushButton *showChartsButton = new QPushButton("Grafikleri Göster", this);
    showChartsButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px; border-radius: 4px; }"
                                   "QPushButton:hover { background-color: #2980b9; }");
    connect(showChartsButton, &QPushButton::clicked, this, &MainWindow::showCharts);

    chartRow = new QHBoxLayout();
    stackChartView = new QtCharts::QChartView();
    flashChartView = new QtCharts::QChartView();
    ramChartView = new QtCharts::QChartView();
    chartRow->addWidget(stackChartView);
    chartRow->addWidget(flashChartView);
    chartRow->addWidget(ramChartView);

    stackChartView->setVisible(false);
    flashChartView->setVisible(false);
    ramChartView->setVisible(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(showChartsButton);
    buttonLayout->addStretch();

    mainLayout->addWidget(memoryTable);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(chartRow);
    setCentralWidget(central);
}

void MainWindow::initializeMemoryTable() {
    memoryTable->setRowCount(0);
    lastStats = {};
}
void MainWindow::updateMemoryTable() {
    if (lastStats.stackTotal == 0 && lastStats.flashTotal == 0 && lastStats.ramTotal == 0) {
        memoryTable->setRowCount(0);
        return;
    }

    memoryTable->setRowCount(3);

    auto addRow = [&](int row, const QString &type, double used, double total) {
        double free = total - used;
        double percent = (total > 0) ? (used * 100.0 / total) : 0.0;

        memoryTable->setItem(row, 0, new QTableWidgetItem(type));
        memoryTable->setItem(row, 1, new QTableWidgetItem(QString::number(total, 'f', 2)));
        memoryTable->setItem(row, 2, new QTableWidgetItem(QString::number(used, 'f', 2)));
        memoryTable->setItem(row, 3, new QTableWidgetItem(QString::number(free, 'f', 2)));
        memoryTable->setItem(row, 4, new QTableWidgetItem(QString("%1%").arg(QString::number(percent, 'f', 2))));

        // Kullanım yüzdesine göre renklendirme(kullanıcıdan al)
        QTableWidgetItem *percentItem = memoryTable->item(row, 4);
        if (percent > 80) {
            percentItem->setBackground(QColor("#ff6b6b"));
        } else if (percent > 60) {
            percentItem->setBackground(QColor("#ffd166"));
        } else {
            percentItem->setBackground(QColor("#06d6a0"));
        }
    };

    addRow(0, "STACK", lastStats.stackUsed, lastStats.stackTotal);
    addRow(1, "FLASH", lastStats.flashUsed, lastStats.flashTotal);
    addRow(2, "RAM", lastStats.ramUsed, lastStats.ramTotal);
}void MainWindow::showCharts()
{
    bool visible = !stackChartView->isVisible();

    stackChartView->setVisible(visible);
    flashChartView->setVisible(visible);
    ramChartView->setVisible(visible);

    if (visible) {
        setupCharts(); // Boyutları yeniden ayarla

        // Grafikleri oluştur
        showPieChart(stackChartView, "STACK", lastStats.stackUsed, lastStats.stackTotal);
        showPieChart(flashChartView, "FLASH", lastStats.flashUsed, lastStats.flashTotal);
        showPieChart(ramChartView, "RAM", lastStats.ramUsed, lastStats.ramTotal);

        // Layout'u güncelle
        layout()->update();
    }
}
void MainWindow::setupCharts()
{
    // 1. SizePolicy ayarları
    QSizePolicy chartSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartSizePolicy.setHorizontalStretch(1);

    // 2. Boyut ayarları
    const int chartSize = 350;
    QSize chartDimensions(chartSize, chartSize);

    // 3. Tüm chart view'ları ayarla
    auto setupChartView = [&](QtCharts::QChartView* view) {
        view->setSizePolicy(chartSizePolicy);
        view->setMinimumSize(chartDimensions);
        view->setRenderHint(QPainter::Antialiasing);
    };

    setupChartView(stackChartView);
    setupChartView(flashChartView);
    setupChartView(ramChartView);

    // 4. Layout stretch ayarları
    chartRow->setStretch(0, 1); // stackChartView
    chartRow->setStretch(1, 1); // flashChartView
    chartRow->setStretch(2, 1); // ramChartView
}
MainWindow::~MainWindow() {}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().toLocalFile().endsWith(".map")) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        if (filePath.endsWith(".map")) {
            openFile(filePath);
        }
    }
}

void MainWindow::openFileDialog() {
    QString filePath = QFileDialog::getOpenFileName(this, "Map Dosyası Seç", "", "Map Files (*.map);;All Files (*)");
    if (!filePath.isEmpty()) {
        openFile(filePath);
    }
}

void MainWindow::openFile(const QString &filePath) {
    // İstatistikleri sıfırla
    lastStats = {};

    // MapParser'ı kullanarak dosyayı parse et
    if (!parseMapFile(filePath, lastStats)) {
        QMessageBox::warning(this, "Hata", "Dosya işlenirken bir hata oluştu.");
        return;
    }

    lastStats.stackUsed /= 1024.0;
    lastStats.stackTotal /= 1024.0;
    lastStats.flashUsed /= 1024.0;
    lastStats.flashTotal /= 1024.0;
    lastStats.ramUsed /= 1024.0;
    lastStats.ramTotal /= 1024.0;

    updateMemoryTable();

    if (stackChartView->isVisible()) {
        showPieChart(stackChartView, "STACK", lastStats.stackUsed, lastStats.stackTotal);
        showPieChart(flashChartView, "FLASH", lastStats.flashUsed, lastStats.flashTotal);
        showPieChart(ramChartView, "RAM", lastStats.ramUsed, lastStats.ramTotal);
    }

    // Dosya adını pencerenin başlığına ekle
    QFileInfo fileInfo(filePath);
    setWindowTitle("Map Analyzer - " + fileInfo.fileName());
    QMessageBox::information(this, "Başarılı",
            QString("Dosya başarıyla yüklendi:\n%1").arg(fileInfo.fileName()));
}
void MainWindow::updateCharts(const QVector<QString> &lines) {
    // Bu fonksiyon örnek olarak basit verilerle doldurulmuştur.

    lastStats.stackUsed = 120.0;
    lastStats.stackTotal = 200.0;
    lastStats.flashUsed = 150.0;
    lastStats.flashTotal = 300.0;
    lastStats.ramUsed = 180.0;
    lastStats.ramTotal = 256.0;

    updateMemoryTable();

    if (stackChartView->isVisible()) {
        showPieChart(stackChartView, "STACK", lastStats.stackUsed, lastStats.stackTotal);
        showPieChart(flashChartView, "FLASH", lastStats.flashUsed, lastStats.flashTotal);
        showPieChart(ramChartView, "RAM", lastStats.ramUsed, lastStats.ramTotal);
    }
}

void MainWindow::showPieChart(QtCharts::QChartView *view, const QString &title, double used, double total) {
    using namespace QtCharts;

    QPieSeries *series = new QPieSeries();
    double free = total - used;

    // Renk paleti iyileştirmeleri
    QPieSlice *usedSlice = series->append("Used", used);
    QPieSlice *freeSlice = series->append("Free", free);

    // Modern gradient renkler
    QLinearGradient usedGradient(0, 0, 1, 1);
    usedGradient.setColorAt(0, QColor("#3498db"));
    usedGradient.setColorAt(1, QColor("#2980b9"));
    usedSlice->setBrush(usedGradient);

    QLinearGradient freeGradient(0, 0, 1, 1);
    freeGradient.setColorAt(0, QColor("#bdc3c7"));
    freeGradient.setColorAt(1, QColor("#95a5a6"));
    freeSlice->setBrush(freeGradient);

    // Kenar çizgileri
    QPen pen(Qt::white);
    pen.setWidth(2);
    usedSlice->setPen(pen);
    freeSlice->setPen(pen);

    // 3D efekti için gölgelendirme
    series->setPieSize(0.7);  // Daha küçük boyut
    series->setHorizontalPosition(0.5);
    series->setVerticalPosition(0.5);

    // Etiketler için geliştirmeler
    series->setLabelsPosition(QPieSlice::LabelOutside);
    series->setLabelsVisible(true);

    for (auto slice : series->slices()) {
        slice->setLabelVisible(true);
        double percent = slice->percentage() * 100.0;
        slice->setLabel(QString("%1\n%2% (%3 KB)")
                        .arg(slice->label())
                        .arg(percent, 0, 'f', 1)
                        .arg(slice->value(), 0, 'f', 2));

        QFont font = slice->labelFont();
        font.setFamily("Segoe UI");
        font.setPointSize(10);
        font.setBold(true);
        slice->setLabelFont(font);
        slice->setLabelColor(QColor("#333333"));

        // Etiket bağlantı çizgileri - DÜZELTİLMİŞ KISIM
        slice->setLabelArmLengthFactor(0.1);
        slice->setLabelPosition(QPieSlice::LabelOutside); // Enum değeri kullanılıyor
    }

    // Hover efekti iyileştirmeleri
    connect(series, &QPieSeries::hovered, [=](QPieSlice *slice, bool state){
        QPropertyAnimation *explodeAnim = new QPropertyAnimation(slice, "explodeDistanceFactor");
        explodeAnim->setDuration(300);
        explodeAnim->setEasingCurve(QEasingCurve::OutBack);

        if(state) {
            explodeAnim->setStartValue(slice->explodeDistanceFactor());
            explodeAnim->setEndValue(0.15);  // Patlama miktarı
            slice->setLabelFont(QFont("Segoe UI", 11, QFont::Bold));
        } else {
            explodeAnim->setStartValue(slice->explodeDistanceFactor());
            explodeAnim->setEndValue(0);    // Orijinal pozisyon
            slice->setLabelFont(QFont("Segoe UI", 10, QFont::Bold));
        }

        explodeAnim->start(QPropertyAnimation::DeleteWhenStopped);
    });

    // Chart ayarları
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("<b>%1 Memory Usage</b><br><span style='font-size:10pt; color:#555'>Total: %2 KB</span>")
                    .arg(title)
                    .arg(total, 0, 'f', 2));

    // Başlık formatı
    chart->setTitleBrush(QColor("#333333"));
    QFont titleFont("Segoe UI", 12, QFont::Bold);
    titleFont.setWeight(QFont::DemiBold);
    chart->setTitleFont(titleFont);

    // Animasyonlar
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setAnimationDuration(1200);

    // Arkaplan ve tema
    chart->setBackgroundBrush(QColor("#f8f9fa"));
    chart->setBackgroundRoundness(10);
    chart->setMargins(QMargins(15, 15, 15, 15));
    chart->setContentsMargins(-10, -10, -10, -10);

    // Legend (açıklama) ayarları
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI", 9));
    chart->legend()->setLabelColor(QColor("#555555"));
    chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

    // ChartView ayarları
    view->setRenderHint(QPainter::Antialiasing, true);
    view->setRenderHint(QPainter::TextAntialiasing, true);
    view->setRenderHint(QPainter::SmoothPixmapTransform, true);
    view->setBackgroundBrush(QColor("#f8f9fa"));
    view->setChart(chart);

    // Gölge efekti
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 0, 0, 160));
    shadowEffect->setOffset(3, 3);
    view->setGraphicsEffect(shadowEffect);
}
void MainWindow::exportToExcel() {
    // Eğer hiç veri yoksa uyarı ver
    if (lastStats.stackTotal == 0 && lastStats.flashTotal == 0 && lastStats.ramTotal == 0) {
        QMessageBox::warning(this, "Uyarı", "Dışa aktarılacak veri bulunamadı!");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this,
        "Excel Dosyasını Kaydet",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/memory_stats.xlsx",
        "Excel Files (*.xlsx)");

    if (path.isEmpty()) return;

    QXlsx::Document xlsx;

    // Stil ayarları
    QXlsx::Format headerFormat;
    headerFormat.setFontBold(true);
    headerFormat.setFontSize(12);
    headerFormat.setFillPattern(QXlsx::Format::PatternSolid);
    headerFormat.setPatternBackgroundColor(QColor("#3498db"));
    headerFormat.setFontColor(Qt::white);
    headerFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);

    QXlsx::Format dataFormat;
    dataFormat.setFontSize(11);
    dataFormat.setHorizontalAlignment(QXlsx::Format::AlignRight);

    // Kolon başlıkları
    xlsx.write(1, 1, "Bellek Türü", headerFormat);
    xlsx.write(1, 2, "Toplam (KB)", headerFormat);
    xlsx.write(1, 3, "Kullanılan (KB)", headerFormat);
    xlsx.write(1, 4, "Boş (KB)", headerFormat);
    xlsx.write(1, 5, "Kullanım %", headerFormat);

    // Kolon genişlikleri
    xlsx.setColumnWidth(1, 20);
    xlsx.setColumnWidth(2, 15);
    xlsx.setColumnWidth(3, 15);
    xlsx.setColumnWidth(4, 15);
    xlsx.setColumnWidth(5, 15);

    // Veri yazan fonksiyon
    auto writeRow = [&](int row, const QString &type, double used, double total) {
        double free = total - used;
        double percent = (total > 0) ? (used * 100.0 / total) : 0.0;

        // Kullanım yüzdesine göre renk formatı
        QXlsx::Format percentFormat = dataFormat;
        if (percent > 80) {
            percentFormat.setPatternBackgroundColor(QColor("#ff6b6b"));
        } else if (percent > 60) {
            percentFormat.setPatternBackgroundColor(QColor("#ffd166"));
        } else {
            percentFormat.setPatternBackgroundColor(QColor("#06d6a0"));
        }

        xlsx.write(row, 1, type, dataFormat);
        xlsx.write(row, 2, total, dataFormat);
        xlsx.write(row, 3, used, dataFormat);
        xlsx.write(row, 4, free, dataFormat);
        xlsx.write(row, 5, QString("%1%").arg(percent, 0, 'f', 2), percentFormat);
    };

    // Bellek türlerini yaz
    writeRow(2, "STACK", lastStats.stackUsed, lastStats.stackTotal);
    writeRow(3, "FLASH", lastStats.flashUsed, lastStats.flashTotal);
    writeRow(4, "RAM",   lastStats.ramUsed,   lastStats.ramTotal);

    if (xlsx.saveAs(path)) {
          // Windows'ta dosyayı aç
          #ifdef Q_OS_WIN
              QDesktopServices::openUrl(QUrl::fromLocalFile(path));
          #endif

          QMessageBox::information(this, "Başarılı",
              QString("Excel dosyası başarıyla kaydedildi ve açılıyor:\n%1").arg(path));
      } else {
          QMessageBox::warning(this, "Hata", "Excel dosyası kaydedilemedi!");
      }
}
