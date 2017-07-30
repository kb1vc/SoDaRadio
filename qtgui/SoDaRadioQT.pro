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

HEADERS  += mainwindow.hpp \
    freqlabel.hpp \
    logtable.hpp \
    soda_wfall.hpp \
    soda_wfall_picker.hpp \
    soda_wfall_data.hpp \
    soda_list_spinbox.hpp \
    soda_freq_scale_draw.hpp \
    soda_comboboxes.hpp \
    soda_plot_picker.hpp \
    soda_spect.hpp
    

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig qwt debug

