package server.entities.request;

public class RequestContainer<T> extends BaseRequest {
    private T payload;

    public T getPayload() {
        return payload;
    }
}
