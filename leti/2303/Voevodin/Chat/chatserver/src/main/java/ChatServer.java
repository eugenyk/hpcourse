import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.AsynchronousChannelGroup;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.Executors;

public class ChatServer implements Runnable {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    private AsynchronousServerSocketChannel serverSocketListener;
    private String host;
    private int port;
    private int nThreads;
    private CopyOnWriteArraySet<Client> clients;

    public ChatServer(ServerConfiguration configuration)
    {
        this.nThreads = configuration.getThreads();
        this.host = configuration.getHost();
        this.port = configuration.getPort();
        clients = new CopyOnWriteArraySet<>();

        logger.info("Server start with configuration: \n\t Host: {} \n\t Port: {} \n\t Number Threads: {}",
                host, port, nThreads);
    }

    @Override
    public void run()
    {
        try
        {
            AsynchronousChannelGroup threadGroup = AsynchronousChannelGroup.withFixedThreadPool(nThreads, Executors.defaultThreadFactory());
            serverSocketListener = AsynchronousServerSocketChannel.open(threadGroup);
            serverSocketListener.bind(new InetSocketAddress(host, port));
            logger.info("Server is listening at {}:{}", host, port);
            serverSocketListener.accept(null, new ConnectionHandler());
        }
        catch (IOException e)
        {
            logger.error("Couldn't start server", e);
        }
    }

    public void registerClient(Client client)
    {
        clients.add(client);
    }

    public void unregisterClient(Client client)
    {
        clients.remove(client);
    }

    public void broadcast(Client who, byte[] message) throws IOException
    {
        for (Client client : clients)
        {
            if (!who.equals(client))
            {
                client.write(message);
            }
        }
    }


    private class ConnectionHandler implements CompletionHandler<AsynchronousSocketChannel, Void>
    {
        @Override
        public void completed(AsynchronousSocketChannel socketChannel, Void server)
        {
            try
            {
                logger.info("Accepted a connection from {}", socketChannel.getRemoteAddress());
                serverSocketListener.accept(null, this);
                Client client = new Client(socketChannel, ChatServer.this);
                registerClient(client);
                logger.info("Client {} registered", socketChannel.getRemoteAddress());
                client.listen();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }

        @Override
        public void failed(Throwable e, Void attach)
        {
            logger.error("Failed to accept a  connection");
            e.printStackTrace();
        }
    }
}
