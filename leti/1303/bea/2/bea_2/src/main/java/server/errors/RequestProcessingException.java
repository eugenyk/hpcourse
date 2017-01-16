package server.errors;

public class RequestProcessingException extends Exception {
    public RequestProcessingException(String msg) {
        super(msg);
    }
}
