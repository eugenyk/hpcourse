package com.github.oxaoo.chat.client;

import com.github.oxaoo.chat.common.proto.Message;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.concurrent.ExecutionException;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 14.01.2017
 */
public class Main {
    private static final int DEFAULT_SERVER_PORT = 5432;
    private static final String DEFAULT_SERVER_HOST = "localhost";

    private static final Logger LOG = LoggerFactory.getLogger(Main.class);

    public static void main(String[] args) {
        final String serverHost = getHost(args);
        final int serverPort = getServerPort(args);

        final Client client;
        try {
            client = new Client(serverHost, serverPort);
        } catch (IOException e) {
            LOG.error("Error while create connection. Cause: {}", e.toString());
            return;
        } catch (InterruptedException | ExecutionException e) {
            return;
        }

        String[] texts = {"Message 1", "Message 2", "Message 3"};
        String[] senders = {"Ivan", "Petr", "Alex"};
        for (int i = 0; i < 3; i++) {
            Message.ChatMessage.Builder message = Message.ChatMessage.newBuilder();
            message.setText(texts[i]).setSender(senders[i]);

            try {
                client.sendMessage(message.build());
            } catch (IOException e) {
                LOG.error("Error while send message to server: {}", e.toString());
            }

            try {
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        try {
            client.closeConnection();
        } catch (IOException e) {
            LOG.error("Error close server connection. Cause: {}", e.toString());
        }
    }

    private static int getServerPort(String[] args) {
        int port = DEFAULT_SERVER_PORT;
        if (args.length > 1) {
            try {
                port = Integer.parseInt(args[1]);
            } catch (NumberFormatException ignore) {}
        }
        if (port < 1024 || port > 65535) port = DEFAULT_SERVER_PORT;
        return port;
    }

    private static String getHost(String[] args) {
        String host = DEFAULT_SERVER_HOST;
        if (args.length > 0) {
            try {
                host = args[0];
            } catch (NumberFormatException ignore) {}
        }
        if (host.isEmpty()) host = DEFAULT_SERVER_HOST;
        return host;
    }
}
