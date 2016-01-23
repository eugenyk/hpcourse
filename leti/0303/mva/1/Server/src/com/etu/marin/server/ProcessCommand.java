package com.etu.marin.server;

import common.Message;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousSocketChannel;

/**
 * Created by Vladislav on 18.01.2016.
 */
public class ProcessCommand implements Runnable {

    AsynchronousSocketChannel socketChannel;
    String command;

    public ProcessCommand(AsynchronousSocketChannel socketChannel, String command){
        this.socketChannel = socketChannel;
        this.command = command;
    }

    @Override
    public void run() {
        String res = executeCommand(command.substring(3));
        byte[] response = Message.Msg.newBuilder()
                .setSender("Server")
                .setText(res)
                .build().toByteArray();
        ByteBuffer byteBuffer = ByteBuffer.allocate(4 + response.length);
        byteBuffer.putInt(response.length);
        byteBuffer.put(response);
        byteBuffer.rewind();

        socketChannel.write(byteBuffer);
    }

    private String executeCommand(String command) {

        StringBuilder output = new StringBuilder();

        Process p;
        try {
            p = Runtime.getRuntime().exec(command);
            p.waitFor();
            BufferedReader reader =
                    new BufferedReader(new InputStreamReader(p.getInputStream()));

            String line = "";
            while ((line = reader.readLine())!= null) {
                output.append(line).append("\n");
            }

        } catch (Exception e) {
            return "Wrong cmd " + command;
        }

        return output.toString();

    }
}
