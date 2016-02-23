package server;

import org.apache.log4j.Logger;
import server.api.ChatHandler;
import server.api.ChatWorker;

import java.nio.channels.AsynchronousSocketChannel;
import java.util.Collection;
import java.util.concurrent.CopyOnWriteArraySet;


public class ChatWebSocketHandler implements ChatHandler {
    private static final Logger LOG = Logger.getLogger(ChatWebSocketHandler.class);
    private Collection<ChatWorker> workers;

    public ChatWebSocketHandler() {
        workers = new CopyOnWriteArraySet<>();
    }

    @Override
    public Collection<ChatWorker> getWorkers() {
        return workers;
    }

    @Override
    public void addWorker(AsynchronousSocketChannel channel) {
        LOG.info("Worker: " + channel + " connected");
        ChatWorker worker = new ChatWebSocketWorker(channel) {
            @Override
            protected void disconnect() {
                removeWorker(this);
            }

            @Override
            protected void sendMessage(byte[] message) {
                broadcast(this, message);
            }
        };

        worker.read();
        workers.add(worker);
    }

    @Override
    public void removeWorker(ChatWorker worker) {
        LOG.info("Worker: " + worker.getChannel() + " disconnected");
        workers.remove(worker);
    }

    @Override
    public void broadcast(ChatWorker sender, byte[] message) {
        workers.stream().filter(worker -> !worker.equals(sender)).forEach(worker -> worker.write(message));
    }
}
