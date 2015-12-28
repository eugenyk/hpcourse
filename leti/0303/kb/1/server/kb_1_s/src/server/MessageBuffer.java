package server;

import server.proto.MessageBody;

import java.util.ArrayDeque;
import java.util.Queue;

/**
 * Created by lightwave on 24.11.15.
 */
public class MessageBuffer {

    byte[] buffer;
    int length;
    int curLength;
    Queue<byte[]> parts;
    boolean isClean = true;

    public MessageBuffer(byte[] buffer, int length) {


        create(buffer, length);

    }

    public MessageBuffer() {

    }

    synchronized public void append(byte[] toAppend){
            parts.add(toAppend);
            curLength += toAppend.length;


    }

    public byte[] makeMessage() {
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


    public void clean() {
        length = 0;
        buffer = null;
        isClean = true;
        parts = new ArrayDeque<byte[]>();

    }

    public void create(byte[] buffer, int length) {
        parts = new ArrayDeque<byte[]>();
        curLength = buffer.length;
        parts.add(buffer);
        this.length = length;

        isClean = false;
    }


    public boolean isClean() {
        return isClean;
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
