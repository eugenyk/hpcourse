package client.request;

public class RequestContainer<T> extends BaseRequest {
    private T payload;

    public void setPayload(T payload) {
        this.payload = payload;
    }

    public RequestContainer(T payload) {

        this.payload = payload;
    }

    public T getPayload() {
        return payload;
    }
}
