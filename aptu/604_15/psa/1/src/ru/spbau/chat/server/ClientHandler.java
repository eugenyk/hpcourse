package ru.spbau.chat.server;

import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;

class ClientHandler {

    private static final int MESSAGE_LIMIT = 4096;
    private static final int QUEUE_LIMIT = 100;

    private final Server server;
    private final AsynchronousSocketChannel channel;

    private final Reader reader;
    private final Writer writer;


    public ClientHandler(final Server server, final AsynchronousSocketChannel channel) {
        this.server = server;
        this.channel = channel;

        reader = new Reader();
        writer = new Writer();

        reader.run();
    }

    public SocketAddress getRemoteAddress() throws IOException {
        return channel.getRemoteAddress();
    }

    public void send(final ChatProtocol.Message message) {
        writer.send(message);
    }

    public void close() throws IOException {
        channel.close();
    }

    private class Reader implements CompletionHandler<Integer, Object> {

        private final ByteBuffer buffer = ByteBuffer.allocate(MESSAGE_LIMIT);

        public void run() {
            channel.read(buffer, null, this);
        }

        @Override
        public void completed(final Integer result, final Object attachment) {
            if (result != -1) {
                try {
                    final ChatProtocol.Message message = ChatProtocol.Message.parseDelimitedFrom(
                            new ByteArrayInputStream(buffer.array())
                    );

                    resetAndRun();

                    server.onNewMessage(ClientHandler.this, message);
                } catch (final IOException ignore) {
                    if (!buffer.hasRemaining() && server.onIncomingOverload(ClientHandler.this)) {
                        resetAndRun();
                    }
                }
            } else {
                server.onReadEnd(ClientHandler.this);
            }
        }

        @Override
        public void failed(final Throwable t, final Object attachment) {
            if (server.onReadFailed(ClientHandler.this, t)) {
                run();
            }
        }

        private void resetAndRun() {
            buffer.position(0);

            run();
        }
    }

    private class Writer implements CompletionHandler<Integer, Object> {

        private final ConcurrentLinkedQueue<ChatProtocol.Message> queue = new ConcurrentLinkedQueue<>();
        ;
        private final AtomicBoolean isBusy = new AtomicBoolean();

        private ByteBuffer currentBuffer = null;

        public void send(final ChatProtocol.Message message) {
            if (queue.offer(message) && queue.size() > QUEUE_LIMIT) {
                if (!server.onOutcomingOverload(ClientHandler.this)) {
                    queue.poll();
                }
            }

            if (isBusy.compareAndSet(false, true)) {
                processQueue();
            }
        }

        @Override
        public void completed(final Integer result, final Object attachment) {
            if (currentBuffer.hasRemaining()) {
                channel.write(currentBuffer, null, this);
            } else {
                processQueue();
            }
        }

        @Override
        public void failed(final Throwable t, final Object attachment) {
            if (server.onWriteFailed(ClientHandler.this, t)) {
                processQueue();
            } else {
                isBusy.set(false);
            }
        }

        private void processQueue() {
            final ChatProtocol.Message message = queue.poll();

            if (message != null) {
                final ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

                try {
                    message.writeDelimitedTo(outputStream);
                } catch (final IOException e) {
                    isBusy.set(false);

                    server.onSerializeFailed(ClientHandler.this, message, e);

                    return;
                }

                currentBuffer = ByteBuffer.wrap(outputStream.toByteArray());

                channel.write(currentBuffer, null, this);
            } else {
                isBusy.set(false);
            }
        }
    }
}
