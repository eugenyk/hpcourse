QT += core
QT -= gui

CONFIG += c++11

TARGET = lr1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    image.h \
    arguments.h \
    pixel.h \
    graph_types.h

SOURCES += main.cpp \
    arguments.cpp

INCLUDEPATH += /home/administrator/IntelTBB/ltbb/include


LIBS += -L$$PWD/../../IntelTBB/ltbb/lib/intel64/gcc4.7/ -ltbb
INCLUDEPATH += $$PWD/../../IntelTBB/ltbb/lib/intel64/gcc4.7
DEPENDPATH += $$PWD/../../IntelTBB/ltbb/lib/intel64/gcc4.7

