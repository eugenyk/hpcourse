package server.entities.request;

public class BaseRequest {
    private String id;
    private String action;
    private String token;

    public String getId() {
        return id;
    }

    public String getAction() {
        return action;
    }

    public String getToken() {
        return token;
    }
}
