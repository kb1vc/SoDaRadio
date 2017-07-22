#-------------------------------------------------
#
# Project created by QtCreator 2017-06-12T20:57:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SoDaRadioQT
TEMPLATE = app

LIBS += -lqwt-qt5

SOURCES += main.cpp\
        mainwindow.cpp \
        freqlabel.cpp \
        logtable.cpp \
        soda_wfall.cpp soda_wfall_data.cpp soda_spect.cpp \
    soda_comboboxes.cpp

HEADERS  += mainwindow.h \
    freqlabel.h \
    logtable.h \
    soda_wfall.h \
    soda_wfall_picker.h \
    soda_wfall_data.h \
    soda_list_spinbox.h \
    soda_freq_scale_draw.h \
    soda_comboboxes.h \
    soda_plot_picker.h \
    soda_spect.h
    

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig qwt debug

