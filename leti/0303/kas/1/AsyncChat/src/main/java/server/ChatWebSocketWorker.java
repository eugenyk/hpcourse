package server;

import com.google.protobuf.InvalidProtocolBufferException;
import org.apache.log4j.Logger;
import server.api.ChatWorker;
import server.proto.MessageBody;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Optional;
import java.util.concurrent.*;
import java.util.concurrent.locks.ReentrantLock;


public abstract class ChatWebSocketWorker implements ChatWorker {
    private static final Logger LOG = Logger.getLogger(ChatWebSocketWorker.class);
    public static final int CAPACITY = 65536;
    public static final int LENGTH = 4;
    private final AsynchronousSocketChannel socketChannel;
    private BlockingDeque<byte[]> blocks;
    private ConcurrentLinkedQueue<ByteBuffer> queue;
    private ReentrantLock lock = new ReentrantLock();
    private ByteBuffer buffer;
    private int messageLength;
    private int appendix;

    public ChatWebSocketWorker(AsynchronousSocketChannel socketChannel) {
        this.socketChannel = socketChannel;
        blocks = new LinkedBlockingDeque<>();
        queue = new ConcurrentLinkedQueue<>();
        buffer = ByteBuffer.allocate(CAPACITY);
    }

    protected abstract void disconnect();
    protected abstract void sendMessage(byte[] message);

    @Override
    public AsynchronousSocketChannel getChannel() {
        return socketChannel;
    }

    @Override
    public void read() {
        buffer.position(0);
        socketChannel.read(buffer, null, new CompletionHandler<Integer, Void>() {
            @Override
            public void completed(Integer result, Void attachment) {
                try {
                    if (result == -1) {
                        socketChannel.close();
                        disconnect();
                    } else {
                        handleRead(result);
                    }
                } catch (Throwable ex) {
                    LOG.error("I/O operation failed", ex);
                }
            }

            @Override
            public void failed(Throwable ex, Void attachment) {
                disconnect();
            }
        });
    }

    @Override
    public void write(byte[] message) {
        ByteBuffer buffer = ByteBuffer.allocate(LENGTH + message.length);
        buffer.putInt(message.length);
        buffer.put(message);
        buffer.position(0);
        queue.offer(ByteBuffer.wrap(message));
        if (lock.tryLock()) {
            socketChannel.write(queue.poll(), null, new CompletionHandler<Integer, Void>() {
                @Override
                public void completed(Integer result, Void attachment) {
                    try {
                        LOG.info("Message write: " + socketChannel.getRemoteAddress());
                        if (!queue.isEmpty()) {
                            socketChannel.write(queue.poll(), null, this);
                        } else {
                            lock.unlock();
                        }
                    } catch (IOException e) {
                        LOG.error("Failed write message", e);
                        lock.unlock();
                    }
                }

                @Override
                public void failed(Throwable exc, Void attachment) {
                    LOG.error("Failed write message");
                    lock.unlock();
                }
            });
        }
    }

    private void handleRead(int result) {
        Optional<MessageBody.Message> message = formMessage(result);
        if (message.isPresent()) {
            String msgText = message.get().getText();
            // handle command
            if (msgText.startsWith("/c ")) {
                Command cmd = new Command(this, msgText.substring(3).trim());
                CommandThread.getInstance().addCommand(cmd);
            } else {
                sendMessage(message.get().toByteArray());
            }
        }
        read();
    }

    public Optional<MessageBody.Message> formMessage(int result) {
        buffer.position(0);
        if (blocks.size() == 0) {
            messageLength = appendix = buffer.getInt();
            result -= LENGTH;
        }

        byte[] block = new byte[result];
        buffer.get(block);
        blocks.add(block);
        appendix -= result;

        if (appendix == 0) {
            byte[] message = new byte[messageLength];
            int position = 0;
            while(!blocks.isEmpty()) {
                byte[] b = blocks.poll();
                int blockLength = b.length;
                System.arraycopy(b, 0, message, position, blockLength);
                position += blockLength;
            }

            try {
                return Optional.of(MessageBody.Message.parseFrom(message));
            } catch (InvalidProtocolBufferException e) {
                LOG.error("Failed parse message", e);
            }
        }

        return Optional.empty();
    }
}
