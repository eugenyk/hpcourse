win32:message(You have to add the path to the google protobuf headers to the PATH variable and compile Message.proto file by protoc.exe)
else:message(You have to install libprotoc-dev package to build this project and protobuf-compiler to compile Message.proto file)

TEMPLATE = subdirs

SUBDIRS += \
    server.pro \
    client.pro
