#include "ChatServerHandler.h"

ChatServerHandler::ChatServerHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor):
_socket(socket),
_reactor(reactor)
{
    Poco::Util::Application& app = Poco::Util::Application::instance();
    app.logger().information("Connection from " + socket.peerAddress().toString());
    
    _reactor.addEventHandler(_socket, Poco::NObserver<ChatServerHandler, Poco::Net::ReadableNotification>(*this, &ChatServerHandler::onReadable));
    _reactor.addEventHandler(_socket, Poco::NObserver<ChatServerHandler, Poco::Net::ShutdownNotification>(*this, &ChatServerHandler::onShutdown));
    
    sendBroadcastMesage("[system]", _socket.peerAddress().toString() + " connected.\n");
    mymutex.lock();
    clients.push_back(socket);
    mymutex.unlock();
}

ChatServerHandler::~ChatServerHandler()
{
    Poco::Util::Application& app = Poco::Util::Application::instance();
    try
    {
        app.logger().information("Disconnecting " + _socket.peerAddress().toString());
        sendBroadcastMesage("[system]", _socket.peerAddress().toString() + " disconnected.\n");
    }
    catch (...)
    {
    }
    _reactor.removeEventHandler(_socket, Poco::NObserver<ChatServerHandler, Poco::Net::ReadableNotification>(*this, &ChatServerHandler::onReadable));
    _reactor.removeEventHandler(_socket, Poco::NObserver<ChatServerHandler, Poco::Net::ShutdownNotification>(*this, &ChatServerHandler::onShutdown));
}

void ChatServerHandler::onReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf)
{
    char _pBuffer[1024];// = (void*)::operator new(BUFFER_SIZE);
    /*int n = */
    
    int symbol = 0; while (1) { symbol += _socket.receiveBytes(&_pBuffer + symbol, 1024);
        if (_pBuffer[symbol - 1] == '\n') break; }
    
    sendBroadcastMesage(_socket.peerAddress().toString(), _pBuffer);
    
    if (_pBuffer[0] == '/' && _pBuffer[1] == 'c') {
        queue.enqueueNotification(new CommandNotification(_pBuffer, _socket));
    }
}

void ChatServerHandler::onShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf)
{

    std::string buffer = "Server shutdown";
    _socket.sendBytes(buffer.c_str(), (int) buffer.length());
}

void ChatServerHandler::sendBroadcastMesage(const std::string sender, const std::string message) {
    
    std::string buffer = sender + ": " + message;

    for (int i = 0; i < clients.size(); i++) {
        clients.at(i).sendBytes(buffer.c_str(), (int) buffer.length());
    }
}
