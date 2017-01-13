#-------------------------------------------------
#
# Project created by QtCreator 2017-01-03T14:58:28
#
#-------------------------------------------------

QT       += core gui
QT       += core network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChatClient
TEMPLATE = app


SOURCES += main.cpp\
        main_window.cpp \
    chat_client.cpp \
    area_message_form.cpp \
    login_form.cpp \
    message/message.pb.cc

HEADERS  += main_window.h \
    chat_client.h \
    area_message_form.h \
    login_form.h \
    message/message.pb.h

FORMS    += main_window.ui \
    area_message_form.ui \
    login_form.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib/release/ -lprotobuf
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib/debug/ -lprotobuf
else:unix: LIBS += -L$$PWD/../../../../usr/local/lib/ -lprotobuf

INCLUDEPATH += $$PWD/../../../../usr/local/include/google/protobuf
DEPENDPATH += $$PWD/../../../../usr/local/include/google/protobuf

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/release/libprotobuf.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/debug/libprotobuf.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/release/protobuf.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/debug/protobuf.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/libprotobuf.a
