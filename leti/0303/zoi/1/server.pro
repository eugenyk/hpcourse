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
    src/server/command_thread.cpp \
    src/server/my_threadpool.cpp \
    src/server/main.cpp \
    src/server/my_socket.cpp

HEADERS += \
    common/proto/Message.pb.h \
    common/socket/socket.h \
    inc/server/command_thread.h \
    inc/server/my_threadpool.h \
    inc/server/server.h \
    inc/server/my_socket.h

CONFIG += c++11
LIBS += -pthread -I/usr/local/include -pthread -L/usr/local/lib -lprotobuf -lpthread
