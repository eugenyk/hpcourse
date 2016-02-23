package server.api;

import java.nio.channels.AsynchronousSocketChannel;
import java.util.Collection;


public interface ChatHandler {
    Collection<ChatWorker> getWorkers();
    void addWorker(AsynchronousSocketChannel channel);
    void removeWorker(ChatWorker worker);
    void broadcast(ChatWorker sender, byte[] message);
}
