package server;

import server.api.ChatWorker;
import server.proto.MessageBody;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.concurrent.BlockingDeque;
import java.util.concurrent.LinkedBlockingDeque;


public class CommandThread implements Runnable {
    private static final CommandThread instance = new CommandThread();
    private BlockingDeque<Command> commands = new LinkedBlockingDeque<>();

    private CommandThread () {}

    public static CommandThread getInstance() {
        return instance;
    }

    @Override
    public void run() {
        while (true) {
            try {
                if (!commands.isEmpty()) {
                    Command command = commands.take();
                    request(command.getWorker(), getRequestText(command.getCommand()));
                }
            } catch (InterruptedException e) {
                LOG.error("Error when take command from deque", e);
                Thread.currentThread().interrupt();
            }
        }
    }

    public void addCommand(Command command) {
        commands.add(command);
    }

    private void request(ChatWorker worker, String text) {
        MessageBody.Message msg = MessageBody.Message.newBuilder()
                .setSender("Server").setText(text).build();
        worker.write(msg.toByteArray());
    }

    private String getRequestText(String command) {
        try {
            Runtime runtime = Runtime.getRuntime();
            Process process = runtime.exec(command);
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                StringBuilder builder = new StringBuilder();
                while ((line = reader.readLine()) != null) {
                    builder.append(line);
                }
                return builder.toString();
            }
        } catch (IOException e) {
            return "command '" + command + "' failed";
        }
    }
}
