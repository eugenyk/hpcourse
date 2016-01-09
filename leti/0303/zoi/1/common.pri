!contains(QT_VERSION, ^5\\.[2-9]\\..*) {
    error(This project requires Qt 5.2.0 or newer but Qt $$QT_VERSION was detected)
}

!win32:CONFIG += c++11 thread
mac:CONFIG -= app_bundle

win32 {
    contains(QT_ARCH, x86_64) {
        ARCH = x64
    } else {
        ARCH = x86
    }

    INCLUDEPATH += $$PWD/third-party
    LIBS += -L$$PWD/third-party/google/$$ARCH

    CONFIG(release, debug|release): LIBS += -llibprotobuf
    else:CONFIG(debug, debug|release): LIBS += -llibprotobufd
} else {
    LIBS += -lprotobuf
}

INCLUDEPATH += $$PWD/common/socket
INCLUDEPATH += $$PWD/common/proto
