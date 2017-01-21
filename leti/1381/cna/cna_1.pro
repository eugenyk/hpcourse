QT += core
QT -= gui

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

TARGET = cna_1
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    image.cpp \
    comporator.cpp

HEADERS += \
    image.h \
    agroritms.h \
    comporator.h

INCLUDEPATH += C:/lib_src/tbb2017_20161128oss/include
DEPENDPATH += C:/lib_src/tbb2017_20161128oss/include
LIBS += -LC:/lib_src/tbb2017_20161128oss/build/windows_ia32_gcc_mingw4.8.2_release
LIBS += -ltbb -ltbbmalloc -ltbbmalloc_proxy
