// MemoryTableWidget.h
#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QSpinBox>

class MemoryTableWidget : public QWidget {
    Q_OBJECT

public:
    explicit MemoryTableWidget(QWidget *parent = nullptr);

private:
    QTableWidget *table;
    int threshold;

    void updateTable();
};

