package ru.spb.leti.garadagly.chat.client.core;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.concurrent.ExecutionException;

@Deprecated
public class StartClient {
    public static void main(String[] args) throws ExecutionException, InterruptedException, IOException {
        NotificationsListener listener = new NotificationsListener() {
            @Override
            public void processMessage(String msg) {
                System.out.println("Принято сообщение " + msg);
            }

            @Override
            public void processDisconnect() {
                System.out.println("Отсоединён от сервера");
            }

            @Override
            public void processConnect() {

            }
        };

        FacadeChatClient client = new FacadeChatClient(listener);
        client.connect("localhost", 8989, "test");
        BufferedReader consoleReader = new BufferedReader(
                new InputStreamReader(System.in));
        while (true) {
            String msg = consoleReader.readLine();
            System.out.print(">");
            client.send(msg);
        }
    }
}
