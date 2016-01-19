package ru.spbau.chat.server;

import ru.spbau.chat.commons.protocol.ChatProtocol;

import java.io.File;
import java.util.Arrays;

class LsExecutor implements Runnable {

    private final ClientHandler clientHandler;

    public LsExecutor(final ClientHandler clientHandler) {
        this.clientHandler = clientHandler;
    }

    @Override
    public void run() {
        final ChatProtocol.Message message = ChatProtocol.Message.newBuilder()
                .setType(ChatProtocol.Message.Type.MESSAGE)
                .addAllText(Arrays.asList(new File("." ).list()))
                .setAuthor(Server.SERVER_NAME)
                .build();

        clientHandler.send(message);
    }
}
