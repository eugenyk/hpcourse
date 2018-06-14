package ru.leti.chat.utils.handlersIO;

import ru.leti.chat.utils.protoMessage.ProtoMessage;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.function.Consumer;

import static ru.leti.chat.utils.protoMessage.ConvertMessage.createDefaultBuffer;
import static ru.leti.chat.utils.protoMessage.ConvertMessage.receivingMessageFromByte;

/**
 * @author Ivan Motovilov
 *
 * <p>Реализация обработчика завершения для асинхронного чтения</p>
 */
public class CompletionHandlerInput extends CompletionHandlerIO {
    private final ByteBuffer byteBuffer;
    private final Consumer<ProtoMessage.ChatMessage> messageGenaration;

    public CompletionHandlerInput(ByteBuffer byteBuffer,
                                  Consumer<ProtoMessage.ChatMessage> messageGenaration,
                                  Consumer<Throwable> errorGeneration) {
        super(errorGeneration);
        this.byteBuffer = byteBuffer;
        this.messageGenaration = messageGenaration;
    }

    @Override
    protected void onFulfilled(AsynchronousSocketChannel socketChannel) throws IOException {
        try {
            messageGenaration.accept(receivingMessageFromByte(this.byteBuffer));
        } finally {
            submitInputTask(socketChannel, messageGenaration, errorGeneration);
        }
    }

    /**
     * <p>Задача на чтение</p>
     * @param socketChannel канал сокета
     * @param messageGenaration функциональный интерфейс для обаботки сообщений
     * @param errorGeneration функциональный интерфейс для обработки ошибок
     */
    public static void submitInputTask(AsynchronousSocketChannel socketChannel,
                                       Consumer<ProtoMessage.ChatMessage> messageGenaration,
                                       Consumer<Throwable> errorGeneration) {
        ByteBuffer buffer = createDefaultBuffer();
        socketChannel.read(buffer,
                socketChannel,
                new CompletionHandlerInput(buffer, messageGenaration, errorGeneration));
    }
}
