QT += core
QT -= gui

TARGET = mnv_1
CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    Image.cpp \
    Utils.cpp \
    colormanagement.cpp


unix:!macx|win32: LIBS += -L$$PWD/../../../../../tbb2017_20161128oss/lib/intel64/vc12/ -ltbb -ltbbmalloc -ltbbmalloc_proxy -ltbbproxy

INCLUDEPATH += $$PWD/../../../../../tbb2017_20161128oss/include
DEPENDPATH += $$PWD/../../../../../tbb2017_20161128oss/include

HEADERS += \
    Image.h \
    Utils.h \
    colormanagement.h
