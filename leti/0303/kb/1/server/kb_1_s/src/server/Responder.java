package server;

import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Map;

/**
 * Created by lightwave on 19.12.15.
 */
public class Responder {
    private Map <String,AsynchronousSocketChannel> acceptors = null;
    private AsynchronousSocketChannel sender;
    private byte[] message;

    public Responder(Map<String,AsynchronousSocketChannel> acceptors, AsynchronousSocketChannel sender, byte[] message) {
        this.sender = sender;
        this.acceptors = acceptors;
        this.message = message;
    }

    public Responder(AsynchronousSocketChannel sender, byte[] message) {
        this.sender = sender;
        this.message = message;
    }

    public void start() {
        if(acceptors == null) {
            System.out.println("Directly");
            sendDirectly();
        } else {
            System.out.println("Broadcast");
            sendBroadcast();
        }
    }

    protected void sendDirectly() {
        Carrier carrier = new Carrier(sender, message);
        carrier.send();
    }

    protected void sendBroadcast() {
        Iterator<AsynchronousSocketChannel> iterator = acceptors.values().iterator();
        while(iterator.hasNext()) {
            AsynchronousSocketChannel curChannel = iterator.next();
            try {
                System.out.println("Channel:" + curChannel.getRemoteAddress() + " open:" + curChannel.isOpen());
            } catch (Exception e) {

            }
            if(curChannel != sender && curChannel.isOpen()) {
                Carrier carrier = new Carrier(curChannel, message);
                carrier.send();
            }
        }
    }


    class Carrier implements CompletionHandler<Integer, Void> {
        AsynchronousSocketChannel user;
        String key;
        byte[] message;

        public Carrier(AsynchronousSocketChannel user, byte[] message) {
            this.message = message;
            this.user = user;
        }

        public void send() {
            try {
                System.out.println("Sending to:" + (key = user.getRemoteAddress().toString()));
            } catch (Exception e) {

            }
            ByteBuffer buffer = ByteBuffer.allocate(message.length + 4);
            buffer.putInt(message.length);
            buffer.put(message);
            buffer.position(0);
            user.write(buffer, null, this);
        }


        @Override
        public void completed(Integer integer, Void aVoid) {

        }

        @Override
        public void failed(Throwable throwable, Void aVoid) {
            System.out.println("Failed to send:" + key + " : " + throwable.toString() );
        }
    }
}
