#-------------------------------------------------
#
# Project created by QtCreator 2014-04-08T13:33:31
#
#-------------------------------------------------

QT       += core
QT       -= gui
CONFIG   += c++11
CONFIG   += console
CONFIG   += static

TARGET = EOGfilter

target.path = /sync/sofia-eog/bin
INSTALLS += target

SOURCES += main.cpp \
    sumfilter.cpp

HEADERS += \
    sumfilter.h
