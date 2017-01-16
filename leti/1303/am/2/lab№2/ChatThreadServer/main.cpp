#include <QCoreApplication>
#include "chat_server.h"
#include "QThread"
int main(int argc, char *argv[])
{

   QCoreApplication a(argc, argv);
   ChatServer chatServer(4);
   return a.exec();


}

