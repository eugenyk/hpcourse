#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
#error This project requires Qt 5.2.0 or newer
#endif

#include <QtWidgets/QApplication>
#include <QtNetwork/QTcpSocket>

#include <google/protobuf/stubs/common.h>

#include "connectdialog.h"
#include "chat.h"

int main(int argc, char** argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    QApplication app(argc, argv);

    QTcpSocket* socket = new QTcpSocket;

    {
        QScopedPointer<ConnectDialog> dialog(new ConnectDialog(socket));
        if(dialog->exec() != QDialog::Accepted)
            return EXIT_FAILURE;
    }

    Chat chat(socket);

    QObject::connect(&app, &QApplication::aboutToQuit, &chat, &Chat::close);
    QObject::connect(&app, &QApplication::aboutToQuit, socket, &QTcpSocket::deleteLater);

    chat.show();
    return app.exec();
}
