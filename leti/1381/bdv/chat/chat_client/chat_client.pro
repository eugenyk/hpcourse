QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chat_client
TEMPLATE = app

LIBS += -lprotobuf

SOURCES += main.cpp\
        mainwindow.cpp \
    tcpclient.cpp \
    ../protobuff/chatmessage.pb.cc

HEADERS  += mainwindow.h \
    tcpclient.h \
    ../protobuff/chatmessage.pb.h
