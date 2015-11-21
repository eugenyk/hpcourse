#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
#error This project requires Qt 5.2.0 or newer
#endif

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtCore/QVector>

#include "execthread.h"
#include "socket.h"
#include "server.h"

int main(int argc, char** argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    QCoreApplication app(argc, argv);
    app.setApplicationName("Multithreaded asynchronous message server");
    app.setApplicationVersion("2.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Author: Aleksey Blokhin");
    parser.addHelpOption();

    QCommandLineOption threadCountOption(QStringList() << "t" << "thread-count"
                                         , QObject::tr("the maximum number of threads used to process messages")
                                         , QObject::tr("count")
                                         , QString::number(QThread::idealThreadCount()));
    QCommandLineOption portOption(QStringList() << "p" << "port"
                                  , QObject::tr("the port to start the server")
                                  , QObject::tr("port"));

    QString bigEndianDefault, littleEndianDefault;
    if(QSysInfo::ByteOrder == QSysInfo::BigEndian)
        bigEndianDefault.append(' ').append(QObject::tr("[DEFAULT]"));
    else
        littleEndianDefault.append(' ').append(QObject::tr("[DEFAULT]"));

    QCommandLineOption bigEndianOption(QStringList() << "b" << "big-endian"
                                  , QObject::tr("use big-endian") + bigEndianDefault);
    QCommandLineOption littleEndianOption(QStringList() << "l" << "little-endian"
                                  , QObject::tr("use little-endian") + littleEndianDefault);

    parser.addOption(littleEndianOption);
    parser.addOption(bigEndianOption);

    parser.addOption(threadCountOption);
    parser.addOption(portOption);

    parser.process(app);

    QString threadCountStr = parser.value(threadCountOption);
    QString portStr = parser.value(portOption);
    QSysInfo::Endian endian = QSysInfo::ByteOrder;

    int threadCount = QThread::idealThreadCount();
    int port = 10000;

    if(portStr.isEmpty())
    {
        printf("Missing '-p' option.\n");
        parser.showHelp();
    }
    else
    {
        bool ok = false;
        port = portStr.toInt(&ok);
        if(!ok || port < 1 || port > 65535)
        {
            printf("Invalid port value.\n");
            parser.showHelp();
        }
        else if(!threadCountStr.isEmpty())
        {
            threadCount = threadCountStr.toInt(&ok);
            if(!ok || threadCount < 1)
            {
                printf("Invalid thread count value.\n");
                parser.showHelp();
            }
            else
            {
                if(parser.isSet(bigEndianOption))
                {
                    if(parser.isSet(littleEndianOption))
                    {
                        printf("You could use either -b nor -l.\n");
                        parser.showHelp();
                    }

                    IOSocket::setEndian(QSysInfo::BigEndian);
                }
                else if(parser.isSet(littleEndianOption))
                {
                    if(parser.isSet(bigEndianOption))
                    {
                        printf("You could use either -b nor -l.\n");
                        parser.showHelp();
                    }

                    IOSocket::setEndian(QSysInfo::LittleEndian);
                }
            }
        }
    }

    Server::instance()->setThreadPoolSize(threadCount);
    ExecutionThread::instance()->start();

    return Server::instance()->start(port) ? app.exec() : EXIT_FAILURE;
}
