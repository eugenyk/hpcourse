package server.api;

import java.nio.channels.AsynchronousSocketChannel;


public interface ChatWorker {
    AsynchronousSocketChannel getChannel();
    void read();
    void write(byte[] message);
}
