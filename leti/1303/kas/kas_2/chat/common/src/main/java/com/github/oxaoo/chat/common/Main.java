package com.github.oxaoo.chat.common;


import com.github.oxaoo.chat.common.proto.Message;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 11.01.2017
 */
public class Main {
    public static void main(String[] args) {
        Message.ChatMessage.Builder message = Message.ChatMessage.newBuilder();
        message.setText("Hello world").setSender("Ivan");
        System.out.println(message.toString());
    }
}
