package server;

import com.google.gson.JsonSyntaxException;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.server.WebSocketServer;
import server.entities.request.BaseRequest;
import server.entities.response.BaseResponse;
import server.entities.response.ResponseContainer;
import server.errors.ProtocolValidationException;
import server.errors.RequestProcessingException;
import server.errors.TokenValidationException;

import java.net.InetSocketAddress;

public class Server extends WebSocketServer {

    public Server(Integer wsPort) {
        super(new InetSocketAddress(wsPort));
        System.out.println("Server started: listening on port " + wsPort);
    }

    @Override
    public void onOpen(WebSocket webSocket, ClientHandshake clientHandshake) {
        System.out.println("Client connected: " + Thread.currentThread().getId()
                + " " + Thread.currentThread().getName());
    }

    @Override
    public void onClose(WebSocket webSocket, int i, String s, boolean b) {
        System.out.println("Client disconnected: " + Thread.currentThread().getId()
                + " " + Thread.currentThread().getName());
    }

    @Override
    public void onMessage(WebSocket webSocket, String jsonRequest) {
        BaseRequest baseRequest = null;
        BaseResponse response;
        System.out.println(jsonRequest);
        // todo: remove debug

        try {
            baseRequest = DI.instance.gson.fromJson(jsonRequest, BaseRequest.class);
            DI.instance.protocolValidator.validate(baseRequest);
            DI.instance.tokenValidator.validate(baseRequest);

            response = DI.instance.requestProcessor.performAction(jsonRequest, Action.valueOf(baseRequest.getAction()));
            response.setCode(200);
            response.setId(baseRequest.getId());
        } catch (JsonSyntaxException e) {
            response = new ResponseContainer<>(451,"JSON_PARSE_ERROR");
        } catch (ProtocolValidationException e) {
            response = new ResponseContainer<>(452,"PROTOCOL_VALIDATION_ERROR");
        } catch (TokenValidationException e) {
            response = new ResponseContainer<>(453,"INVALID_TOKEN");
        } catch (RequestProcessingException e) {
            response = new ResponseContainer<>(454, e.getMessage());
        }

        if (baseRequest != null) {
            response.setId(baseRequest.getId());
        }

        String answer = DI.instance.gson.toJson(response);
        webSocket.send(answer);
    }

    @Override
    public void onError(WebSocket webSocket, Exception e) {
        e.printStackTrace();
    }
}
