package chat.chatclient.handlers;

import chat.chatclient.ChatEventsHandler;
import chat.chatclient.Message.Msg;
import chat.chatclient.handlers.WriteHandler.MessageInfoHolder;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import org.apache.log4j.Logger;


public class WriteHandler implements CompletionHandler<Integer, MessageInfoHolder> {

    private final Logger logger = Logger.getLogger(WriteHandler.class);
    private final AsynchronousSocketChannel channel;
    private final ChatEventsHandler handler;
    
    public WriteHandler(AsynchronousSocketChannel channel, ChatEventsHandler handler) {
        this.channel = channel;
        this.handler = handler;
    }
    
    public WriteHandler write(String sender, String message) {
        Msg newMsg = Msg.newBuilder()
                .setText(message.trim())
                .setSender(sender)
                .build();
        
        byte[] newMsgBytes = newMsg.toByteArray();
        ByteBuffer buffer = ByteBuffer.allocate(4 + newMsgBytes.length)
                .putInt(newMsgBytes.length)
                .put(newMsgBytes);
        
        buffer.rewind();
        
        channel.write(buffer, new MessageInfoHolder(buffer, sender, message), this);
        return this;
    }
    
    @Override
    public void completed(Integer result, MessageInfoHolder holder) {
        if (holder.buffer.hasRemaining()) {
            channel.write(holder.buffer, holder, this);
        } else {
            handler.onMessage(holder.sender, holder.message);
        }
    }

    @Override
    public void failed(Throwable ex, MessageInfoHolder attachment) {
        logger.error("Unable to write message!", ex);
    }
    
    public static class MessageInfoHolder {
        
        public final ByteBuffer buffer;
        public final String sender;
        public final String message;
        
        public MessageInfoHolder(
                final ByteBuffer buffer,
                final String sender,
                final String message) {
            this.buffer = buffer;
            this.sender = sender;
            this.message = message;
        }
    }
}
