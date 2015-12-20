package chat.chatserver.handlers;

import chat.chatserver.Client;
import chat.chatserver.ChatServer;
import chat.chatserver.CommandManager;
import chat.chatserver.Message.Msg;
import com.google.protobuf.InvalidProtocolBufferException;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.CompletionHandler;
import java.util.Arrays;
import java.util.regex.Pattern;
import org.apache.log4j.Logger;


public class ReadHandler implements CompletionHandler<Integer, ByteArrayOutputStream> {

    private static final Pattern commandPattern = Pattern.compile("/c (.*)");
            
    private final Logger logger = Logger.getLogger(ReadHandler.class);
    
    private final Client client;
    private final ByteBuffer buffer = ByteBuffer.allocate(2048);
    private int messageLength = 0;
    
    public ReadHandler(Client client) {
        this.client = client;
    }
    
    public ReadHandler read() {
        client.getChannel().read(buffer, new ByteArrayOutputStream(), this);
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
                logger.info("Stopping listening to client..");
                
                ChatServer.getInstance().removeClient(client);
                client.getChannel().close();

                return;
            }
            
            messageStream.write(toBytesArray(buffer));
             
            while (true) {
                if ((messageStream.size() >= 4) && (messageLength == 0)) {
                    messageLength = ByteBuffer.wrap(messageStream.toByteArray(), 0, 4).getInt();
                }

                if ((messageStream.size() >= messageLength) && (messageLength > 0)) {
                    byte[] message = Arrays.copyOfRange(messageStream.toByteArray(), 0, messageLength + 4);
                    byte[] remaining = Arrays.copyOfRange(messageStream.toByteArray(), messageLength + 4, messageStream.size());
                    messageStream.reset();
                    messageStream.write(remaining);
                    messageLength = 0;
                    
                    handleMessage(message);
                } else {
                    break;
                }

            }

            client.getChannel().read(buffer, messageStream, this);
        } catch (IOException ex) {
            ChatServer.getInstance().removeClient(client);
            logger.error("Error while reading data from client!", ex);
        }
    }

    @Override
    public void failed(Throwable ex, ByteArrayOutputStream attachment) {
        ChatServer.getInstance().removeClient(client);
        logger.error("Failed to read data from client!", ex);
    }
    
    private void handleMessage(byte[] message) {
        try {
            Msg source = Msg.parseFrom(Arrays.copyOfRange(message, 4, message.length));
            if (commandPattern.matcher(source.getText()).matches()) {
                CommandManager.getInstance().processCommand(client, source.getText());
                return;
            }
        } catch (InvalidProtocolBufferException ex) {
            logger.error(ex);
        }
        
        ChatServer.getInstance().sendToAll(client, message);
    }
    
}
