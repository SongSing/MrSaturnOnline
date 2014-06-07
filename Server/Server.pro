#-------------------------------------------------
#
# Project created by QtCreator 2014-06-05T21:22:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Server
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    channel.cpp \
    client.cpp \
    ../lib/packet.cpp

HEADERS  += mainwindow.h \
    server.h \
    channel.h \
    client.h \
    ../lib/packet.h

FORMS    += mainwindow.ui
