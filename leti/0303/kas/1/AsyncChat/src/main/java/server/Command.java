package server;

import server.api.ChatWorker;


public class Command {
    private ChatWorker worker;
    private String command;

    public Command(ChatWorker worker, String command) {
        this.worker = worker;
        this.command = command;
    }

    public ChatWorker getWorker() {
        return worker;
    }

    public String getCommand() {
        return command;
    }
}
