package client;

import java.net.URISyntaxException;

public class Main {
    public static void main(String[] args) {
        try {
            Client client = new Client();
            client.connect();
            client.execShell();
        } catch (URISyntaxException ignored) {

        }
    }
}
