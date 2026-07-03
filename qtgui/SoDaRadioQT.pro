#-------------------------------------------------
#
# Project created by QtCreator 2017-06-12T20:57:44
#
#-------------------------------------------------

QT       += core gui multimedia widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SoDaRadioQT
TEMPLATE = app

LIBS += -lqwt-qt6

INCLUDEPATH += ../gui_common

SOURCES += main.cpp\
        mainwindow.cpp \
        soda_spect.cpp \
        ../gui_common/FreqLabel.cpp \
        ../gui_common/soda_logtable.cpp \
        ../gui_common/soda_wfall.cpp \
        ../gui_common/soda_wfall_data.cpp \
        ../gui_common/soda_comboboxes.cpp

HEADERS  += mainwindow.hpp \
    ../gui_common/FreqLabel.hpp \
    ../gui_common/soda_logtable.hpp \
    ../gui_common/soda_wfall.hpp \
    ../gui_common/soda_wfall_picker.hpp \
    ../gui_common/soda_wfall_data.hpp \
    ../gui_common/soda_freq_scale_draw.hpp \
    ../gui_common/soda_comboboxes.hpp \
    ../gui_common/soda_plot_picker.hpp \
    ../gui_common/soda_spect.hpp

QT += widgets

FORMS    += ../gui_common/mainwindow.ui

unix: CONFIG += link_pkgconfig qwt debug
