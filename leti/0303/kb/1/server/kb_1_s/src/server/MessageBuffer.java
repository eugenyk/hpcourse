package server;

import server.proto.MessageBody;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;
import java.util.Queue;

/**
 * Created by lightwave on 24.11.15.
 */
public class MessageBuffer {

    byte[] buffer;
    int length;
    int curLength;
    int tailLength;
    Queue<byte[]> parts;
    boolean isClean = true;

    public MessageBuffer() {
        parts = new ArrayDeque<byte[]>();
    }


    public List<MessageBody.Message> process(ByteBuffer buffer, int amount) {
        buffer.position(0);
        MessageBody.Message temp;
        ArrayList<MessageBody.Message> messages = new ArrayList<MessageBody.Message>();
        if(parts.size() != 0) {
            temp = processOld(buffer, amount);
            if(temp != null) {
                messages.add(temp);
            } else {
                return messages;
            }
        }
        messages.addAll(processNew(buffer, amount));
        return messages;
    }

    private List<MessageBody.Message> processNew(ByteBuffer buffer, int amount) {
        parts = new ArrayDeque<byte[]>();
        int curAmount = amount - buffer.position();
        ArrayList<MessageBody.Message> messages = new ArrayList<MessageBody.Message>();
        while (curAmount > 0) {
            int mLength = buffer.getInt();
            curAmount -= 4;
            byte[] b = new byte[Math.min(curAmount, mLength)];
            buffer.get(b);
            curAmount -= b.length;
            try {
                if (b.length >= mLength) {
                    messages.add(MessageBody.Message.parseFrom(b));
                } else {
                    parts.add(b);
                    curLength = mLength;
                    tailLength = mLength - b.length;
                }
            } catch (Exception e) {
                System.out.println( "Parsing message error: " + e.toString());
            }
        }
        return messages;
    }

    private MessageBody.Message processOld(ByteBuffer buffer, int amount) {
        byte[] b = new byte[Math.min(amount, tailLength)];
        buffer.get(b);
        tailLength -= b.length;
        parts.add(b);
        if(tailLength == 0) {
            try {
                return MessageBody.Message.parseFrom(makeMessage());
            } catch (Exception e) {
                System.out.println( "Parsing big message error: " + e.toString());
                return null;
            }
        }
        return null;
    }

    private byte[] makeMessage() {
        byte[] newBuffer = new byte[curLength];
        int j = 0;
        while(!parts.isEmpty()) {
            byte[] part = parts.poll();
            for(int i=0; i<part.length; i++) {
                newBuffer[j] = part[i];
                j++;
            }
        }
        return  newBuffer;
    }


    public int getCurrentLength() {
        return curLength;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public void setBuffer(byte[] buffer) {
        this.buffer = buffer;
    }

    public int getLength() {
        return length;
    }

    public void setLength(int length) {
        this.length = length;
    }

}
