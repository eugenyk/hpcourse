package edu.etu.client.cmd;

import edu.etu.protobuf.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;


public class Client {
    private final AsynchronousSocketChannel socketChannel;
    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    private ByteArrayOutputStream data;
    private int msgLength;
    private ChatClient chatClient;

    public Client(final AsynchronousSocketChannel socketChannel, ChatClient guiChatClient) {
        this.socketChannel = socketChannel;
        data = new ByteArrayOutputStream();
        this.chatClient = guiChatClient;
        new ReadHandler();
    }

    public void write(byte[] message) {
        ByteBuffer byteBuffer = ByteBuffer.allocate(4 + message.length);
        byteBuffer.putInt(message.length);
        byteBuffer.put(message);
        byteBuffer.rewind();
        socketChannel.write(byteBuffer);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Client)) return false;

        Client client = (Client) o;

        if (socketChannel != null ? !socketChannel.equals(client.socketChannel) : client.socketChannel != null)
            return false;
        return !(data != null ? !data.equals(client.data) : client.data != null);

    }

    @Override
    public int hashCode() {
        int result = socketChannel != null ? socketChannel.hashCode() : 0;
        result = 31 * result + (data != null ? data.hashCode() : 0);
        return result;
    }

    private class ReadHandler implements CompletionHandler<Integer, ByteArrayOutputStream> {

        private ByteBuffer buffer;

        public ReadHandler() {
            buffer = ByteBuffer.allocate(2048);
            socketChannel.read(buffer, data, this);
        }

        private byte[] getBytes(ByteBuffer buffer) {
            buffer.flip();
            int limit = buffer.limit();
            byte[] received = new byte[limit];
            buffer.get(received, 0, limit).clear();
            return received;
        }

        @Override
        public void completed(Integer result, ByteArrayOutputStream msg) {
            try {
                if (result == -1) {
                    chatClient.disconnect();
                    return;
                }

                msg.write(getBytes(buffer));
                while (true) {
                    if (msg.size() >= 4 && msgLength == 0) {
                        msgLength = ByteBuffer.wrap(msg.toByteArray(), 0, 4).getInt();
                    }

                    if (msg.size() >= msgLength + 4 && msgLength > 0) {
                        Message.Msg from = Message.Msg.parseFrom(Arrays.copyOfRange(msg.toByteArray(), 4, msgLength + 4));
                        logger.info("{}: {}", from.getSender(), from.getText());

                        String message = from.getSender() + ": " + from.getText() + "\n";

                        chatClient.getNotificationsListener().processMessage(message);

                        byte[] remaining = Arrays.copyOfRange(msg.toByteArray(), 4 + msgLength, msg.size());
                        msg.reset();
                        msg.write(remaining);
                        msgLength = 0;
                    } else {
                        break;
                    }

                }

                socketChannel.read(buffer, data, this);

            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void failed(Throwable e, ByteArrayOutputStream buffer) {
            e.printStackTrace();
        }
    }

    public AsynchronousSocketChannel getSocketChannel() {
        return socketChannel;
    }
}
