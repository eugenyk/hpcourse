package client.request;

public class BaseRequest {
    public void setAction(String action) {
        this.action = action;
    }

    public void setToken(String token) {
        this.token = token;
    }

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

    public void setId(String id) {
        this.id = id;
    }
}
