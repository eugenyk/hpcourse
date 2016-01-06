#include "dialogconnect.h"

#include <QApplication>

#include <google/protobuf/stubs/common.h>

int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    QApplication a(argc, argv);

    DialogConnect d_con;
    d_con.show();

    return a.exec();

    google::protobuf::ShutdownProtobufLibrary();
}
