include(common.pri)

QT += core network
QT -= gui

TARGET = server
TEMPLATE = app

CONFIG += console

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

INCLUDEPATH += $$PWD/inc/server

OBJECTS_DIR = $$OUT_PWD/.server

SOURCES += common/proto/Message.pb.cc \
    common/socket/socket.cpp \
    src/server/server.cpp \
    src/server/execthread.cpp \
    src/server/threadpool.cpp \
    src/server/main.cpp \
    src/server/socketdecorator.cpp

HEADERS += \
    common/proto/Message.pb.h \
    common/socket/socket.h \
    inc/server/execthread.h \
    inc/server/threadpool.h \
    inc/server/server.h \
    inc/server/socketdecorator.h
