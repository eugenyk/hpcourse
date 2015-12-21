package chat.chatclient.handlers;

import chat.chatclient.ChatClient;
import chat.chatclient.Message.Msg;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousCloseException;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;
import org.apache.log4j.Logger;


public class ReadHandler implements CompletionHandler<Integer, ByteArrayOutputStream> {

    private final Logger logger = Logger.getLogger(ReadHandler.class);
    
    private final ChatClient chatClient;
    private final AsynchronousSocketChannel socketChannel;
    private final ByteBuffer buffer = ByteBuffer.allocate(2048);
    private int messageLength = 0;
    
    public ReadHandler(ChatClient chatClient, AsynchronousSocketChannel socketChannel) {
        this.chatClient = chatClient;
        this.socketChannel = socketChannel;
    }
    
    public ReadHandler read() {
        socketChannel.read(buffer, new ByteArrayOutputStream(), this);
        return this;
    }
    
    private byte[] toBytesArray(ByteBuffer buffer) {
        buffer.flip();
        int limit = buffer.limit();
        byte[] received = new byte[limit];
        buffer.get(received, 0, limit).clear();
        return received;
    }
    
    @Override
    public void completed(Integer result, ByteArrayOutputStream messageStream) {
        try {
            if (result == -1) {
                logger.warn("Disconnecting from server..");
                chatClient.disconnect();
                return;
            }
            
            messageStream.write(toBytesArray(buffer));
            
            while (true) {
                if ((messageStream.size() >= 4) && (messageLength == 0)) {
                    messageLength = ByteBuffer.wrap(messageStream.toByteArray(), 0, 4).getInt();
                }

                if ((messageStream.size() >= messageLength + 4) && (messageLength > 0)) {
                    Msg source = Msg.parseFrom(Arrays.copyOfRange(messageStream.toByteArray(), 4, messageLength + 4));
                    logger.info(String.format("New message '%s: %s'", source.getSender(), source.getText()));

                    chatClient.getEventsHandler().onMessage(source.getSender(), source.getText());

                    byte[] remaining = Arrays.copyOfRange(messageStream.toByteArray(), 4 + messageLength, messageStream.size());
                    messageStream.reset();
                    messageStream.write(remaining);
                    messageLength = 0;
                } else {
                    break;
                }
            }
            
            socketChannel.read(buffer, messageStream, this);
        } catch (IOException ex) {
            logger.error("Error while reading data from client!", ex);
        }
    }

    @Override
    public void failed(Throwable ex, ByteArrayOutputStream attachment) {
        if (ex instanceof AsynchronousCloseException) {
            return;
        }
        
        logger.error("Failed to read data from server!", ex);
        chatClient.disconnect();
    }
    
}
