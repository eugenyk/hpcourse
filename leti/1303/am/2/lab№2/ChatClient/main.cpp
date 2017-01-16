#include "main_window.h"
#include <QApplication>
#include "chat_client.h"
#include "message/message.pb.h"
#include "iostream"
#include <sys/types.h>
#include <netinet/in.h>
#include <stdint.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
using namespace std;
using namespace google::protobuf::io;
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
