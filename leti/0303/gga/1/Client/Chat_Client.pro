#-------------------------------------------------
#
# Project created by QtCreator 2015-12-14T16:57:10
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Chat_Client
TEMPLATE = app
LIBS += -pthread -I/usr/local/include  -pthread -L/usr/local/lib -lprotobuf -lpthread

SOURCES += main.cpp\
    controller.cpp \
    ../common/proto/Message.pb.cc \
    ../common/socket/socket_handler.cpp \
    dialogchat.cpp \
    dialogconnect.cpp


HEADERS  += \
    controller.h \
    ../common/proto/Message.pb.h \
    ../common/socket/socket_handler.h \
    dialogchat.h \
    dialogconnect.h

INCLUDEPATH += $$PWD/../common/socket
INCLUDEPATH += $$PWD/../common/proto

FORMS += \
    dialogchat.ui \
    dialogconnect.ui
