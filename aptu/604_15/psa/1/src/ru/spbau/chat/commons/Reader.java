package ru.spbau.chat.commons;

import com.google.protobuf.CodedOutputStream;
import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

public class Reader implements CompletionHandler<Integer, Object> {

    private final Dispatcher dispatcher;
    private final AsynchronousSocketChannel channel;

    private byte[] prevBuffer;
    private ByteBuffer currentBuffer;

    public Reader(final Dispatcher dispatcher,
                  final AsynchronousSocketChannel channel,
                  final ByteBuffer buffer) {
        this.dispatcher = dispatcher;
        this.channel = channel;
        this.prevBuffer = new byte[0];
        this.currentBuffer = buffer;
    }

    @Override
    public void completed(final Integer result, final Object attachment) {
        if (result == -1) {
            return;
        }

        final byte[] prevAndCurData = concatPrevAndCurData(result);

        try {
            final ChatProtocol.Message message = ChatProtocol.Message.parseDelimitedFrom(new ByteArrayInputStream(prevAndCurData));

            updateBuffersAndRunAsyncRead(calculateNewPrevBuffer(prevAndCurData, message));

            dispatcher.onNewMessage(channel, message);
        } catch (final IOException e) {
            e.printStackTrace();
            System.err.println(e.getMessage());

            updateBuffersAndRunAsyncRead(prevAndCurData);
        }
    }

    @Override
    public void failed(final Throwable exc, final Object attachment) {
        exc.printStackTrace();
        System.err.println(exc.getMessage());
    }

    private byte[] concatPrevAndCurData(final int curBufferLength) {
        final byte[] result = new byte[prevBuffer.length + curBufferLength];

        System.arraycopy(prevBuffer, 0, result, 0, prevBuffer.length);
        System.arraycopy(currentBuffer.array(), 0, result, prevBuffer.length, curBufferLength);

        return result;
    }

    private void updateBuffersAndRunAsyncRead(final byte[] newPrevBuffer) {
        prevBuffer = newPrevBuffer;
        currentBuffer = ByteBuffer.allocate(dispatcher.getDefaultBufferSize());

        channel.read(currentBuffer, null, this);
    }

    private byte[] calculateNewPrevBuffer(final byte[] prevAndCurData, final ChatProtocol.Message message) {
        final int messageSize = message.getSerializedSize();
        final int lengthSize = CodedOutputStream.computeRawVarint32Size(messageSize);

        final int newPrevBufferLength = prevAndCurData.length - messageSize - lengthSize;

        final byte[] newPrevBuffer = new byte[newPrevBufferLength];
        System.arraycopy(prevAndCurData, messageSize + lengthSize, newPrevBuffer, 0, newPrevBufferLength);

        return newPrevBuffer;
    }

    public interface Dispatcher {

        int getDefaultBufferSize();

        void onNewMessage(final AsynchronousSocketChannel channel, final ChatProtocol.Message message);
    }
}
