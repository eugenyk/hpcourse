package chat.chatserver;

import chat.chatserver.handlers.ReadHandler;
import chat.chatserver.handlers.WriteHandler;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.Objects;


public class Client {
    
    private final AsynchronousSocketChannel channel;
    private ReadHandler readHandler;
    private WriteHandler writeHandler = null;
    
    public Client(AsynchronousSocketChannel channel) {
        this.channel = channel;
    }
    
    public void listen() {
        readHandler = new ReadHandler(this).read();
    }
    
    public AsynchronousSocketChannel getChannel() {
        return channel;
    }
    
    public void sendMessage(ByteBuffer messageBuff) {
        if (writeHandler == null) {
            writeHandler = new WriteHandler(channel);
        }
        
        channel.write(messageBuff, messageBuff, writeHandler);
    }
    
    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (!(obj instanceof Client)) return false;

        Client other = (Client) obj;

        return (Objects.equals(channel, other.channel));
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 17 * hash + Objects.hashCode(channel);
        return hash;
    }
}
