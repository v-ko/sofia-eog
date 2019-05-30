#-------------------------------------------------
#
# Project created by QtCreator 2014-06-28T20:07:38
#
#-------------------------------------------------

QT       += core
QT       -= gui
CONFIG   += c++11
CONFIG   += console
CONFIG   += static

TARGET = sofia-eog_cli

target.path = /sync/sofia-eog/bin
INSTALLS += target

SOURCES += main.cpp \
    ../eogfilter.cpp \
    ../eoglibrary.cpp \
    ../petko10q.cpp

HEADERS += \
    ../eogfilter.h \
    ../eoglibrary.h \
    ../petko10q.h

RESOURCES += \
    ../resources.qrc
