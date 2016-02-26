package Konstantinova.Server;

import server.proto.MessageOuterClass;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;
import java.util.Queue;

/**
 * Created by User on 09.01.2016.
 */
public class MessageParser {
    private ByteBuffer msg;
    Queue<byte[]> msgQueue;
    int bufLen, tail, msgLen;
    ArrayList<MessageOuterClass.Message> parsedArr;

    public MessageParser() {
        this.msgQueue = new ArrayDeque<>();
        tail = 0;
        msgLen = 0;
    }

    public ArrayList<MessageOuterClass.Message> parseBuffer (ByteBuffer buff, int len){
        bufLen = len;
        msg = buff;
        parsedArr = new ArrayList<>();
        msg.position(0);
        while (bufLen > 0) {
            if (tail == 0) {
                msgLen = msg.getInt();
                tail = msgLen;
                bufLen -= 4;
            }
            else if (tail <= bufLen) {
                byte[] rawMsg = new byte[tail];
                MessageOuterClass.Message message = null;
                msg.get(rawMsg);
                bufLen -= tail;
                tail = 0;
                if (msgQueue.isEmpty()) {
                    try {
                        message = MessageOuterClass.Message.parseFrom(rawMsg);
                    } catch (Exception e) {}
                }
                else {
                    msgQueue.add(rawMsg);
                    byte[] allMsg = new byte[msgLen];
                    byte[] temp;
                    int i = 0;
                    while (!msgQueue.isEmpty()) {
                        temp = msgQueue.poll();
                        for (int j = 0; j < temp.length; j++){
                            allMsg[i] = temp[j];
                            i++;
                        }
                    }
                    try {
                        message = MessageOuterClass.Message.parseFrom(allMsg);
                    } catch (Exception e) {
                        System.out.println("Long message parsing error: " + e.toString());
                    }
                }
                parsedArr.add(message);
            }
            else if (tail > bufLen) {
                byte[] rawMsg = new byte[bufLen];
                msg.get(rawMsg);
                tail -= bufLen;
                bufLen = 0;
                msgQueue.add(rawMsg);
            }
        }
        return parsedArr;
    }
}
