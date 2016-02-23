package client.api;

import java.io.IOException;
import java.nio.channels.AsynchronousSocketChannel;


public interface Client {
    AsynchronousSocketChannel getChannel();
    void connect() throws IOException;
    void disconnect() throws IOException;
    void read();
    void write(byte[] message);
    void registerListener(ClientListener listener);
    void unregisteredListener(ClientListener listener);
}
