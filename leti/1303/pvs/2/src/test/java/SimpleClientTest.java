import com.github.parfenovvs.websocketchatserver.entity.Message;
import com.google.gson.Gson;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;
import org.junit.Test;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;

public class SimpleClientTest {
    Gson gson = new Gson();

    @Test
    public void testClients() throws URISyntaxException, InterruptedException {
        List<WebSocketClient> clients = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            WebSocketClient webSocketClient = new WebSocketClient(new URI("ws://127.0.0.1:8081/")) {
                @Override
                public void onOpen(ServerHandshake handshakedata) {

                }

                @Override
                public void onMessage(String message) {
                    System.out.println("Message for client " + finalI + ": " + message);
                }

                @Override
                public void onClose(int code, String reason, boolean remote) {

                }

                @Override
                public void onError(Exception ex) {

                }
            };
            clients.add(webSocketClient);
            webSocketClient.connectBlocking();
            webSocketClient.send(gson.toJson(new Message("Client " + i, "server", "auth")));
        }

        for (int i = 0; i < 100; i++) {
            int client = (int) (Math.random() * clients.size());
            int receiver;
            while ((receiver = (int) (Math.random() * clients.size())) == client);
            clients.get(client).send(gson.toJson(new Message("Client " + client, "Client " + receiver, "some message")));
        }

        for (WebSocketClient c : clients) {
            c.closeBlocking();
        }
    }

}
