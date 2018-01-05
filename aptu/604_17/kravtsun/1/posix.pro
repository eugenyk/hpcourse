TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

QMAKE_CXXFLAGS += -Wall -Wpedantic
QMAKE_CXXFLAGS += -lpthread

LIBS += -lpthread

DISTFILES += \
    readme.txt
