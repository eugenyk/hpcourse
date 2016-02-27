public class ServerExecutorRequest {

    private ServerClient client;
    private String request;

    public ServerExecutorRequest(ServerClient client, String request) {
        this.client = client;
        this.request = request;
    }

    public ServerClient getClient() {
        return client;
    }

    public String getRequest() {
        return request;
    }

}
