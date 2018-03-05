package ChatServer2;
import ChatServer2.Message;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;


public class ConvertMessage {
    public static final int BUFFER_SIZE = 1024; // размер буфера

    private ConvertMessage() {

    }

    public static ByteBuffer createDefaultBuffer() {
        return ByteBuffer.allocate(BUFFER_SIZE);
    }

    public static Message.Msg receivingMessageFromByte(ByteBuffer buffer) throws IOException {
        return Message.Msg.parseDelimitedFrom(new ByteArrayInputStream(buffer.array()));
    }

    public static ByteBuffer receivingBufferFormMessage(Message.Msg msg) {
        try {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            msg.writeDelimitedTo(outputStream);
            return ByteBuffer.wrap(outputStream.toByteArray());
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
