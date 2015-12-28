package server;

import java.nio.channels.AsynchronousSocketChannel;

/**
 * Created by lightwave on 14.12.15.
 */
public class QueueElement {
    private AsynchronousSocketChannel client;
    private String request;

    public QueueElement(AsynchronousSocketChannel client, String request) {
        this.client = client;
        this.request = request;
    }

    public AsynchronousSocketChannel getClient() {
        return client;
    }

    public void setClient(AsynchronousSocketChannel client) {
        this.client = client;
    }

    public String getRequest() {
        return request;
    }

    public void setRequest(String request) {
        this.request = request;
    }
}
