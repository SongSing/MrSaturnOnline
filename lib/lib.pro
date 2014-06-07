#-------------------------------------------------
#
# Project created by QtCreator 2014-06-05T21:22:46
#
#-------------------------------------------------

QT       += widgets

TARGET = lib
TEMPLATE = lib

DEFINES += LIB_LIBRARY

CONFIG += c++11

SOURCES += \
    enums.cpp \
    packet.cpp

HEADERS += \
    enums.h \
    packet.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
