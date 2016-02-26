package Konstantinova.Server;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.nio.channels.WritePendingException;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Created by User on 20.12.2015.
 */
public class MessageSender {
    List<AsynchronousSocketChannel> connections;
    byte[] message;
    AsynchronousSocketChannel author;

    public MessageSender(byte[] message, List<AsynchronousSocketChannel> connections, AsynchronousSocketChannel author) {
        this.message = message;
        this.connections = connections;
        this.author = author;
    }

    public void sendMessage(){
        Iterator<AsynchronousSocketChannel> iterator = connections.iterator();
        while (iterator.hasNext()){
            AsynchronousSocketChannel currReciever = iterator.next();
            if (currReciever != author) {
                ByteBuffer buffer = ByteBuffer.allocate(message.length + 4);
                buffer.putInt(message.length);
                buffer.put(message);
                SendHandler handler = new SendHandler(currReciever);
                boolean written = false;
                while (!written) {
                    buffer.position(0);
                    try {
                        currReciever.write(buffer, null, handler);
                        written = true;
                    } catch (WritePendingException e) {}
                }
            }
        }
    }

    class SendHandler implements CompletionHandler<Integer, Void> {

        AsynchronousSocketChannel reciever;

        public SendHandler(AsynchronousSocketChannel reciever) {
            this.reciever = reciever;
        }

        @Override
        public void completed(Integer result, Void attachment) {
            System.out.println("Message has been sended!");
        }

        @Override
        public void failed(Throwable exc, Void attachment) {
            System.out.println("Message hasn't been sended: " + exc.toString());
        }
    }
}
