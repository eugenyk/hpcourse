#include "ChatServerHandler.h"
#include "Message.pb.h"

ChatServerHandler::ChatServerHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor):
_socket(socket),
_reactor(reactor)
{
    Poco::Util::Application& app = Poco::Util::Application::instance();
    app.logger().information("Connection from " + socket.peerAddress().toString());
    
    _reactor.addEventHandler(_socket, Poco::NObserver<ChatServerHandler, Poco::Net::ReadableNotification>(*this, &ChatServerHandler::onReadable));
    _reactor.addEventHandler(_socket, Poco::NObserver<ChatServerHandler, Poco::Net::ShutdownNotification>(*this, &ChatServerHandler::onShutdown));
    
    sendBroadcastMesage("[system]", _socket.peerAddress().toString() + " connected.\n");
    
    clients.push_back(socket);
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
    server::proto::Message message;
    
    char _pBuffer[1000];// = (void*)::operator new(BUFFER_SIZE);
    int n = _socket.receiveBytes(&_pBuffer, BUFFER_SIZE);
    
    std::cout << n << ":" << _pBuffer << std::endl;

    if (n > 0){
        if (n > 3) {
            message.ParsePartialFromArray(_pBuffer + 4, 996);
            std::cout << "message: " << message.text() << std::endl;
            sendBroadcastMesage(message.sender(), message.text());
            
            if (_pBuffer[0] == '/' && _pBuffer[1] == 'c') {
                queue.enqueueNotification(new CommandNotification(_pBuffer, _socket));
            }
        }
    }
    else
        delete this;
}

void ChatServerHandler::onShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf)
{
    server::proto::Message message;
    message.set_sender("[system]");
    message.set_text("Server shutdown");
    std::string buffer;
    
    message.SerializeToString(&buffer);
    _socket.sendBytes(buffer.c_str(), (int) buffer.length());
}

void ChatServerHandler::sendBroadcastMesage(const std::string sender, const std::string message) {
    server::proto::Message _message;
    _message.set_sender(sender);
    _message.set_text(message);
    
    std::string buffer;
    
    _message.SerializeToString(&buffer);
    
    union {
        uint32_t size;
        char buff[1004];
    };
    
    for (int o = 0; o < 1004; o++ ){
        buff[o] = 0;
    }
    
    size = (uint32_t)buffer.length();
    _message.SerializeToArray(buff + 4, size);

    for (int i = 0; i < clients.size(); i++) {
        clients.at(i).sendBytes(buff, size + 4);
    }
}
