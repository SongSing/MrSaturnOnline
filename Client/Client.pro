#-------------------------------------------------
#
# Project created by QtCreator 2014-06-05T21:22:02
#
#-------------------------------------------------

QT       += core gui network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Client
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    channel.cpp \
    user.cpp \
    ../lib/packet.cpp

HEADERS  += mainwindow.h \
    channel.h \
    user.h \
    ../lib/packet.h

FORMS    += mainwindow.ui

RESOURCES +=
