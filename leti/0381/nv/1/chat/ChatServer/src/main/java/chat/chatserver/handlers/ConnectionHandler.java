package chat.chatserver.handlers;

import chat.chatserver.Client;
import chat.chatserver.ChatServer;
import java.io.IOException;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import org.apache.log4j.Logger;


public class ConnectionHandler implements CompletionHandler<AsynchronousSocketChannel, Void> {
    
    private final Logger logger = Logger.getLogger(ConnectionHandler.class);
    AsynchronousServerSocketChannel socketListener;
    
    public ConnectionHandler(AsynchronousServerSocketChannel socketListener) {
        this.socketListener = socketListener;
    }

    public ConnectionHandler accept() {
        socketListener.accept(null, this);
        return this;
    }
    
    @Override
    public void completed(AsynchronousSocketChannel channel, Void attachment) {
        try {
            logger.info(
                    String.format("Connection from %s accepted", channel.getRemoteAddress())
            );

            socketListener.accept(null, this);
            Client client = new Client(channel);
            ChatServer.getInstance().addClient(client);
            
            client.listen();
        } catch (IOException ex) {
            logger.error("Error while accepting connection!", ex);
        }
    }

    @Override
    public void failed(Throwable ex, Void attachment) {
        logger.error("Failed to accept connection!", ex);
    }
}
