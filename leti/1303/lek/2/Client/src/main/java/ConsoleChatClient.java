import protobuf.Message;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Created by Елена on 08.01.2017.
 */
public class ConsoleChatClient implements Client {
    private boolean clientWork = true;
    ChatClient client = null;

    public static void main(String[] args) {
        new ConsoleChatClient().start();
    }

    public void start() {
        System.out.print("Please, introduce yourself: ");

        BufferedReader consoleReader = new BufferedReader(
                new InputStreamReader(System.in));
        try {
            String name = consoleReader.readLine();
            client = new ChatClient(name, this);
            if (!client.start()) {
                System.out.println("It's impossible to set connection!");
                return;
            }

            while (clientWork) {
                System.out.print("You : ");
                String message = consoleReader.readLine();
                if (message != null) {
                    client.send(message);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void disconnect() {
        clientWork = false;
        client.disconnect();
    }

    public void showMessage(Message.Msg message) {
        System.out.println();
        System.out.print(message.getSender());
        System.out.print(new StringBuffer(" [").append(message.getDateTime()).append("] :"));
        System.out.println(new StringBuffer(" ").append(message.getText()));
    }

    public void connectionLost() {
        System.out.println();
        System.out.println("Connection with server was lost!");
        disconnect();
    }
}
