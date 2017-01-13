package client.response;

public class ResponseContainer<T> extends BaseResponse {
    private T payload;

    public ResponseContainer(T payload) {
        super("", 0);
        this.payload = payload;
    }

    public ResponseContainer(int code, T payload) {
        super("", code);
        this.payload = payload;
    }

    public T getPayload() {
        return payload;
    }

    public void setPayload(T payload) {
        this.payload = payload;
    }
}
