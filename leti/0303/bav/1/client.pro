include(common.pri)

QT += core gui widgets network

TARGET = chat
TEMPLATE = app

INCLUDEPATH += $$PWD/inc/client

OBJECTS_DIR = $$OUT_PWD/.client

SOURCES += common/proto/Message.pb.cc \
    src/client/chat.cpp \
    src/client/connectdialog.cpp \
    common/socket/socket.cpp \
    src/client/main.cpp

HEADERS  += \
    common/proto/Message.pb.h \
    inc/client/chat.h \
    inc/client/connectdialog.h \
    common/socket/socket.h
