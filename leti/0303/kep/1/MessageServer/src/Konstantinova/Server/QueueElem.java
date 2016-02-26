package Konstantinova.Server;

import java.nio.channels.AsynchronousSocketChannel;

/**
 * Created by User on 20.12.2015.
 */
public class QueueElem {
    String command;
    AsynchronousSocketChannel author;

    public QueueElem(String command, AsynchronousSocketChannel author) {
        this.command = command;
        this.author = author;
    }

    public String getCommand() {
        return command;
    }

    public void setCommand(String command) {
        this.command = command;
    }

    public AsynchronousSocketChannel getAuthor() {
        return author;
    }

    public void setAuthor(AsynchronousSocketChannel author) {
        this.author = author;
    }
}
