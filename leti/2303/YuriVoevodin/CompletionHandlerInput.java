package ChatServer2;


//package ru.leti.chat.utils.handlersIO;

import ChatServer2.Message;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.util.function.Consumer;

import static ChatServer2.ConvertMessage.createDefaultBuffer;
import static ChatServer2.ConvertMessage.receivingMessageFromByte;

public class CompletionHandlerInput extends CompletionHandlerIO {
    private final ByteBuffer byteBuffer;
    private final Consumer<Message.Msg> messageGenaration;

    public CompletionHandlerInput(ByteBuffer byteBuffer,
                                  Consumer<Message.Msg> messageGenaration,
                                  Consumer<Throwable> errorGeneration) {
        super(errorGeneration);
        this.byteBuffer = byteBuffer;
        this.messageGenaration = messageGenaration;
    }
    protected void onFulfilled(AsynchronousSocketChannel socketChannel) throws IOException {
        try {
            messageGenaration.accept(receivingMessageFromByte(this.byteBuffer));
        } finally {
            submitInputTask(socketChannel, messageGenaration, errorGeneration);
        }
    }

    public static void submitInputTask(AsynchronousSocketChannel socketChannel,
                                       Consumer<Message.Msg> messageGenaration,
                                       Consumer<Throwable> errorGeneration) {
        ByteBuffer buffer = createDefaultBuffer();
        socketChannel.read(buffer,
                socketChannel,
                new CompletionHandlerInput(buffer, messageGenaration, errorGeneration));
    }
}
