package ru.leti.chat.utils.protoMessage;

import ru.leti.chat.utils.protoMessage.ProtoMessage;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * <p>Вспомогательный класс для конвертирования входящего буфера байт в proto-сообщение и обратно</p>
 */
public class ConvertMessage {
    public static final int BUFFER_SIZE = 1024; // размер буфера

    private ConvertMessage() {

    }

    /**
     * <p>Создает буфер определенного размера</p>
     *
     * @return ByteBuffer размера, определенного в BUFFER_SIZE
     */
    public static ByteBuffer createDefaultBuffer() {
        return ByteBuffer.allocate(BUFFER_SIZE);
    }

    /**
     * <p>Формирование объекта proto-сообщения из буфера</p>
     *
     * @param buffer буфер
     * @return ProtoMessage.ChatMessage объект proto-сообщения
     * @throws IOException
     */
    public static ProtoMessage.ChatMessage receivingMessageFromByte(ByteBuffer buffer) throws IOException {
        return ProtoMessage.ChatMessage.parseDelimitedFrom(new ByteArrayInputStream(buffer.array()));
    }

    /**
     * <p>Формирование буфера с известным размером из proto-сообщения</p>
     *
     * @param msg объект proto-сообщения
     * @return байтовый буфер
     * @throws IOException
     */
    public static ByteBuffer receivingBufferFormMessage(ProtoMessage.ChatMessage msg) {
        try {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            msg.writeDelimitedTo(outputStream);
            return ByteBuffer.wrap(outputStream.toByteArray());
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
