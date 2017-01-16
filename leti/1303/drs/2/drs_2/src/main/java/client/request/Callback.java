package client.request;

@FunctionalInterface
public interface Callback {
    void exec(String jsonResponse);
}
