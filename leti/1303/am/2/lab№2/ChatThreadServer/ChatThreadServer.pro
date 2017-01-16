QT       += core network
QT -= gui

TARGET = ChatThreadServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    chat_server.cpp \
    chat_session.cpp \
    chat_room.cpp \
    chat_participant.cpp \
    message/message.pb.cc \
    message/encode_chat_message.cpp \
    runnable/send_all_messages.cpp \
    runnable/send_message_to_clients.cpp

HEADERS += \
    chat_server.h \
    chat_session.h \
    chat_room.h \
    chat_participant.h \
    message/message.pb.h \
    message/encode_chat_message.h \
    runnable/send_all_messages.h \
    runnable/send_message_to_clients.h

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
