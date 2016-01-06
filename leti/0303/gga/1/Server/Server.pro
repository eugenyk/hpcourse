QT       += core network
CONFIG += c++11

LIBS +=  -pthread -I/usr/local/include  -pthread -L/usr/local/lib -lprotobuf -lpthread

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Chat_Server_Git
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server.cpp \
    socketdecorator.cpp \
    ../common/proto/Message.pb.cc \
    ../common/socket/socket_handler.cpp \
    waitingthread.cpp

HEADERS += \
    server.h \
    socketdecorator.h \
    ../common/proto/Message.pb.h \
    ../common/socket/socket_handler.h \
    waitingthread.h

INCLUDEPATH += $$PWD/../common/socket
INCLUDEPATH += $$PWD/../common/proto
