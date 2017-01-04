QT += core
QT -= gui

CONFIG += c++11

TARGET = labr1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    image.h \
    oper.h \
    pixel.h \
    graph_types.h

SOURCES += tbb.cpp \
    oper.cpp

INCLUDEPATH += /home/administrator/IntelTBB/ltbb/include


LIBS += -L$$PWD/../../IntelTBB/ltbb/lib/intel64/gcc4.7/ -ltbb
INCLUDEPATH += $$PWD/../../IntelTBB/ltbb/lib/intel64/gcc4.7
DEPENDPATH += $$PWD/../../IntelTBB/ltbb/lib/intel64/gcc4.7
