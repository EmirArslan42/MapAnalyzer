include("QXlsx\QXlsx\QXlsx.pri")

QT += core gui widgets charts
QT += charts

TARGET = MapAnalyzer
TEMPLATE = app

SOURCES += main.cpp \
           MemoryDetailDialog.cpp \
           mainwindow.cpp \
           MapParser.cpp

HEADERS += mainwindow.h \
           MapParser.h \
           MemoryDetailDialog.h \
           clickablelabel.h

RESOURCES += \
    assets.qrc

FORMS +=

RC_ICONS += TEI_logo2.ico

