package edu.etu.server;

import com.google.protobuf.InvalidProtocolBufferException;
import edu.etu.protobuf.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;
import java.util.concurrent.Future;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class Client {
    private final AsynchronousSocketChannel socketChannel;
    private final Logger logger = LoggerFactory.getLogger(Client.class);
    private ChatServer server;
    private ByteArrayOutputStream data;
    private int msgLength;

    public Client(final AsynchronousSocketChannel socketChannel, final ChatServer server) {
        this.socketChannel = socketChannel;
        this.server = server;
        data = new ByteArrayOutputStream();
    }

    public void listen() {
        new ReadHandler();
    }

    public void write(byte[] message) {
        Future<Integer> writeMessageFuture = socketChannel.write(ByteBuffer.wrap(message));
        while (!writeMessageFuture.isDone()) ;
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
            buffer = ByteBuffer.allocate(ServerConfiguration.BUFFER_CAPACITY);
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
                    socketChannel.close();
                    server.unregisterClient(Client.this);
                    logger.info("Stopped listening to the {}", Client.this);
                    return;
                }

                msg.write(getBytes(buffer));

                while (true) {
                    if (msg.size() >= 4 && msgLength == 0) {
                        msgLength = ByteBuffer.wrap(msg.toByteArray(), 0, 4).getInt();
                    }


                    if (msg.size() >= msgLength && msgLength > 0) {
                        byte[] receivedMsg = Arrays.copyOfRange(msg.toByteArray(), 0, msgLength + 4);
                        byte[] remaining = Arrays.copyOfRange(msg.toByteArray(), msgLength + 4, msg.size());
                        msg.reset();
                        msg.write(remaining);
                        msgLength = 0;

                        if (!checkAndRun(receivedMsg)) {
                            server.broadcast(Client.this, receivedMsg);
                        }

                    } else {
                        break;
                    }

                }

                socketChannel.read(buffer, data, this);

            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        private boolean checkAndRun(byte[] receivedMsg) {
            try {
                Message.Msg from = Message.Msg.parseFrom(Arrays.copyOfRange(receivedMsg, 4, receivedMsg.length));
                if (isCommand(from.getText())) {
                    CommandExecutor.INSTANCE.addTask(from.getText(), Client.this);
                    return true;
                }
            } catch (InvalidProtocolBufferException e) {
                e.printStackTrace();
            }
            return false;
        }

        private boolean isCommand(String cmd) {
            Pattern pattern = Pattern.compile("/c (.*)");
            Matcher matcher = pattern.matcher(cmd);
            return matcher.matches();
        }

        @Override
        public void failed(Throwable e, ByteArrayOutputStream buffer) {
            e.printStackTrace();
        }

    }

}
