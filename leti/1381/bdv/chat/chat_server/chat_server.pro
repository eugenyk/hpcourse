QT       += core network
QT -= gui

CONFIG += console
CONFIG -= app_bundle

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chat_server
TEMPLATE = app

LIBS += -lprotobuf

SOURCES += main.cpp\
    tcpserver.cpp \
    readandhandle.cpp \
    ../protobuff/chatmessage.pb.cc

HEADERS  += \
    tcpserver.h \
    readandhandle.h \
    ../protobuff/chatmessage.pb.h
