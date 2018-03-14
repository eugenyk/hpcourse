import chat.Message;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.concurrent.Future;

public class ChatClient implements Runnable {
    private Client client;
    private String host;
    private int port;
    private NotificationsListener notificationsListener;
    private String name;

    public ChatClient(String name, NotificationsListener notificationsListener)
    {
        host = "localhost";
        port = 8989;
        this.notificationsListener = notificationsListener;
        this.name = name;
    }

    public ChatClient(String host, int port, String name, NotificationsListener notificationsListener)
    {
        this.host = host;
        this.port = port;
        this.notificationsListener = notificationsListener;
        this.name = name;
    }

    public void run()
    {
        try
        {
            AsynchronousSocketChannel socketChannel = AsynchronousSocketChannel.open();
            SocketAddress serverAddr = new InetSocketAddress(host, port);
            Future<Void> result = socketChannel.connect(serverAddr);
            result.get();
            client = new Client(socketChannel, this);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public void send(String message)
    {
        Message.Msg pMsg = Message.Msg.newBuilder()
                .setText(message.trim())
                .setSender(name)
                .build();
        client.write(pMsg.toByteArray());
    }

    public void disconnect()
    {
        try
        {
            client.getSocketChannel().shutdownOutput();
            notificationsListener.processDisconnect();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

    }

    public NotificationsListener getNotificationsListener()
    {
        return notificationsListener;
    }
}
