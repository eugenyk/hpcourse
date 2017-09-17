QT += core
QT -= gui

CONFIG += c++11

TARGET = posix
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += \
    value.h
