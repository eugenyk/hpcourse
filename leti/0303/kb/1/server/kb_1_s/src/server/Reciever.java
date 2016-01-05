package server;

import server.proto.MessageBody;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * Created by lightwave on 19.12.15.
 */
public class Reciever implements CompletionHandler<Integer, Void> {

    private MessageBuffer buffer = null;
    private Map<String, AsynchronousSocketChannel> connections;
    private ConsoleExecutor executor;
    private AsynchronousSocketChannel client;
    ByteBuffer data;

    public Reciever(Map<String, AsynchronousSocketChannel> connections, AsynchronousSocketChannel client, ConsoleExecutor executor) {
        this.connections = connections;
        this.client = client;
        this.executor = executor;
        data = ByteBuffer.allocate(MessageServer.BUFFERSIZE);
        buffer = new MessageBuffer();
    }

    public void start() {
        data.position(0);
        client.read(data, null, this);

    }


    @Override
    public void completed(Integer result, Void aVoid) {
        System.out.println("Something has been recieved. Length:" + result);
        if(result == -1) {
            try {
                connections.remove(client.getRemoteAddress().toString());
            }
            catch (Exception e) {}
            return;
        }

        List<MessageBody.Message> messages = null;

        try {
            messages = buffer.process(data, result);
        } catch (Exception e) {
            System.out.println("ByteBuffer process gone wrong:" + e.toString());
        }
        if(messages!= null && messages.size() > 0) {
            Iterator<MessageBody.Message> iterator = messages.iterator();
            while(iterator.hasNext()) {
                MessageBody.Message message = iterator.next();
                Responder responder = new Responder(connections, client, message.toByteArray());
                responder.start();
            }
        }
        start();
    }

    @Override
    public void failed(Throwable throwable, Void aVoid) {

    }

    public void setBuffer(MessageBuffer buffer) {
        this.buffer = buffer;
    }
}
