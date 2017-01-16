package client.response;

import client.request.Callback;

import java.lang.reflect.Type;

public class ResponseProcessingData {
    private Type resultType;
    private Callback onSuccess;
    private Callback onError;

    public ResponseProcessingData(Type resultType, Callback onSuccess, Callback onError) {
        this.resultType = resultType;
        this.onSuccess = onSuccess;
        this.onError = onError;
    }

    public Type getResultType() {
        return resultType;
    }

    public Callback getOnSuccess() {
        return onSuccess;
    }

    public Callback getOnError() {
        return onError;
    }
}