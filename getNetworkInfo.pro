#-------------------------------------------------
#
# Project created by QtCreator 2015-12-24T22:31:39
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = getNetworkInfo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    network_utils.cpp

HEADERS  += mainwindow.h \
    network_utils.h

FORMS    += mainwindow.ui

win32: LIBS += -lws2_32
