package chat.chatclient;

import chat.chatclient.handlers.ReadHandler;
import chat.chatclient.handlers.WriteHandler;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import org.apache.log4j.Logger;


public class ChatClient {
 
    private final Logger logger = Logger.getLogger(ChatClient.class);
    
    private String host;
    private int port;
    private String senderName;
    ChatEventsHandler eventsHandler;
    
    private AsynchronousSocketChannel socketChannel = null;
    private WriteHandler writeHandler = null;
   
    public ChatClient(ChatEventsHandler eventsHandler) {
        this.eventsHandler = eventsHandler;
        this.host = "127.0.0.1";
        this.port = 7676;
        this.senderName = "default";
    }
    
    public String getHost() {
        return host;
    }
    
    public int getPort() {
        return port;
    }
    
    public String getSenderName() {
        return senderName;
    }
    
    public ChatEventsHandler getEventsHandler() {
        return eventsHandler;
    }
    
    public void connect(String host, int port, String senderName) {
        this.host = host;
        this.port = port;
        this.senderName = senderName;
        
        connect();
    }
    
    public void connect() {
        try {
            logger.info(
                    String.format("Connecting to host: %s, port: %d", host, port)
            );
            
            socketChannel = AsynchronousSocketChannel.open();
            SocketAddress serverAddress = new InetSocketAddress(host, port);
            Future<Void> result = socketChannel.connect(serverAddress);
            
            result.get();
            eventsHandler.onConnect();
            
            new ReadHandler(this, socketChannel).read();
        } catch (IOException | InterruptedException | ExecutionException ex) {
            logger.error("Problems with connecting to server..", ex);
        }
    }
    
    public void disconnect() {
        try {
            socketChannel.close();
            
            writeHandler = null;
            eventsHandler.onDisconnect();
        } catch (IOException ex) {
            logger.error("Problems with disconnecting..", ex);
        }
    }
    
    public void send(String message) {
        if (writeHandler == null) {
            writeHandler = new WriteHandler(socketChannel, eventsHandler);
        }
        
        writeHandler.write(senderName, message);
    }
}
