package com.example.lightwave.messageclient.Connection;

/**
 * Created by lightwave on 25.11.15.
 */
/**
 * Created by lightwave on 24.11.15.
 */
public class MessageBuffer {

    byte[] buffer;
    int length;

    public MessageBuffer(byte[] buffer, int length) {
        this.buffer = buffer;
        this.length = length;
    }

    public void append(byte[] toAppend){
        int newLength = buffer.length + toAppend.length;
        int offset = buffer.length;
        byte[] newBuffer = new byte[newLength];
        for(int i=0; i<buffer.length; i++)
            newBuffer[i] = buffer[i];
        for(int i=0; i<toAppend.length; i++)
            newBuffer[i + offset] = toAppend[i];
        buffer = newBuffer;
    }

    public int getCurrentLength() {
        return buffer.length;
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

