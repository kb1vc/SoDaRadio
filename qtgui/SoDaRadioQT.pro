#-------------------------------------------------
#
# Project created by QtCreator 2017-06-12T20:57:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SoDaRadioQT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    freqlabel.cpp \
    logtable.cpp

HEADERS  += mainwindow.h \
    freqlabel.h \
    logtable.h

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig

