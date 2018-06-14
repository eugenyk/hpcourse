package ru.leti.chat.utils.handlersIO;

import ru.leti.chat.utils.protoMessage.ProtoMessage;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.function.Consumer;

import static ru.leti.chat.utils.protoMessage.ConvertMessage.receivingBufferFormMessage;


/**
 * @author Ivan Motovilov
 *
 * Реализация обработчика завершения для асинхронной записи
 */
public class CompletionHandlerOutput extends CompletionHandlerIO {
    private CompletionHandlerOutput(Consumer<Throwable> errorGeneration) {
        super(errorGeneration);
    }

    @Override
    protected void onFulfilled(AsynchronousSocketChannel socketChannel) throws IOException {
    }

    /**
     * <p>Задача на запись</p>
     * @param socketChannel канал сокета
     * @param byteBuffer буфер сообщения
     * @param errorGeneration функциональный интерфейс для обработки ошибок
     */
    public static void submitOutputTask(AsynchronousSocketChannel socketChannel,
                                        ByteBuffer byteBuffer,
                                        Consumer<Throwable> errorGeneration) {
        socketChannel.write(byteBuffer, socketChannel, new CompletionHandlerOutput(errorGeneration));
    }

    /**
     *     /**
     * <p>Формирвание задачи на запись</p>
     * @param socketChannel канал сокета
     * @param chatMessage объект proto-сообщения
     * @param errorGeneration функциональный интерфейс для обработки ошибок
     * @throws IOException
     */
    public static void submitOutputTask(AsynchronousSocketChannel socketChannel,
                                        ProtoMessage.ChatMessage chatMessage,
                                        Consumer<Throwable> errorGeneration) throws IOException {
        submitOutputTask(socketChannel, receivingBufferFormMessage(chatMessage), errorGeneration);

    }
}
