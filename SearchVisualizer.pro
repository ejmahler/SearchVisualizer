#-------------------------------------------------
#
# Project created by QtCreator 2014-05-25T19:29:32
#
#-------------------------------------------------

QT       += core gui widgets opengl concurrent

TARGET = SearchVisualizer
TEMPLATE = app

CONFIG += c++11


SOURCES += \
    graphicswidget.cpp \
    main.cpp \
    mainwindow.cpp \
    hexgrid/gridsearchevent.cpp \
    hexgrid/hexgrid.cpp \
    hexgrid/gridsearcher.cpp \
    hexgrid/gridpainter.cpp

HEADERS  += \
    graphicswidget.h \
    mainwindow.h \
    hexgrid/gridsearchevent.h \
    hexgrid/hexgrid.h \
    hexgrid/gridpainter.h \
    hexgrid/gridsearcher.h \
    utils/channel.h \
    algorithms/searchalgorithms.h

FORMS    += \
    mainwindow.ui
