#-------------------------------------------------
#
# Project created by QtCreator 2014-05-05T02:04:10
#
#-------------------------------------------------

QT       += core gui multimedia printsupport
CONFIG += c++11

TARGET = sofia-eog_desktop

target.path = /sync/sofia-eog/bin
INSTALLS += target

TEMPLATE = app

SOURCES += main.cpp\
        eogwindow.cpp \
    ../eoglibrary.cpp \
    ../eogfilter.cpp \
    ../petko10q.cpp \
    eogfilterwidget.cpp \
    ../../../QCustomPlot/qcustomplot.cpp

HEADERS  += eogwindow.h \
    ../eoglibrary.h \
    ../eogfilter.h \
    ../petko10q.h \
    eogfilterwidget.h \
    ../../../QCustomPlot/qcustomplot.h

FORMS    += eogwindow.ui \
    eogfilterwidget.ui

OTHER_FILES += \
    ../filter_configurations/raw_input.filter

RESOURCES += \
    ../resources.qrc

DISTFILES += \
    ../LOG.txt
