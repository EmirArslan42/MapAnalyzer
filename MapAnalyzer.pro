include("QXlsx\QXlsx\QXlsx.pri")

QT += core gui widgets charts
QT += charts

TARGET = MapAnalyzer
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           MapParser.cpp

HEADERS += mainwindow.h \
           MapParser.h \
           clickablelabel.h
