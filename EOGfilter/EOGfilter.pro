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

target.path = /home/pi/sofia-eog/bin
target.files = /home/p10/Dropbox/Pepi/C++/EOGfilter/build-EOGfilter-RPi_5_3_3_static-Release/EOGfilter
INSTALLS += target

SOURCES += main.cpp \
    sumfilter.cpp

HEADERS += \
    sumfilter.h
