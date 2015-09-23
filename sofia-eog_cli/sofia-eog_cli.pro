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
target.path = /home/pi/sofia-eog/bin
target.files = /home/p10/Dropbox/Pepi/C++/sofia-eog/build-sofia-eog_cli-RPi-Release/sofia-eog_cli
INSTALLS += target
#TEMPLATE = app


SOURCES += main.cpp \
    ../eogfilter.cpp \
    ../eoglibrary.cpp \
    ../../../petko10q.cpp

HEADERS += \
    ../eogfilter.h \
    ../eoglibrary.h \
    ../../../petko10q.h

RESOURCES += \
    ../resources.qrc
