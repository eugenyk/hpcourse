package chat.chatserver;

import chat.chatserver.handlers.ConnectionHandler;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.Executors;
import org.apache.log4j.Logger;


public class ChatServer implements Runnable {
    
    private final Logger logger = Logger.getLogger(ChatServer.class);
    private final Configuration conf;
    private final Set<Client> clientsSet = new CopyOnWriteArraySet<>();
    private ConnectionHandler conHandler = null;
    
    private static class ServerHolder {
        public static ChatServer instance = new ChatServer();
    }
    
    private ChatServer() {
        this.conf = new Configuration();
        logger.info(
                String.format("Server started. Host: %s, port: %d, threads count: %d", 
                        conf.getHost(), conf.getPort(), conf.getThreadsCount())
        );
    }
    
    @Override
    public void run() {
        try {
            AsynchronousChannelGroup threadGroup =
                    AsynchronousChannelGroup.withFixedThreadPool(
                            conf.getThreadsCount(), 
                            Executors.defaultThreadFactory()
                    );
            
            AsynchronousServerSocketChannel socketListener = AsynchronousServerSocketChannel.open(threadGroup)
                    .bind(new InetSocketAddress(conf.getHost(), conf.getPort()));
            
            logger.info(
                    String.format("Server is listening at %s : %d", conf.getHost(), conf.getPort())
            );
            
            conHandler = new ConnectionHandler(socketListener).accept();
        } catch (IOException ex) {
            logger.error("Initialization of server socket listener failed!", ex);
        }
    }
    
    public void addClient(Client client) {
        clientsSet.add(client);
    }
            
    public void removeClient(Client client) {
        clientsSet.remove(client);
    }
    
    public void sendToAll(Client source, byte[] message) {
        for (Client client : clientsSet){
            if (client.equals(source)) {
                continue;
            }
            
            client.sendMessage(ByteBuffer.wrap(message));
        }
    }
    
    public static ChatServer getInstance() {
        return ServerHolder.instance;
    }
    
    public static void main(String[] args) throws InterruptedException, IOException {
        new Thread(ChatServer.getInstance()).start();
        Thread.currentThread().join();
    }
    
}
