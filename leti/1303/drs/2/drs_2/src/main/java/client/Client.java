package client;

import client.request.*;
import client.response.*;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;

public class Client extends WebSocketClient {

    private static final Gson gson = new Gson();

    private Map<String, ResponseProcessingData> callbacks = new HashMap<>();
    private String [] cmdTokens;
    private int requestNumber;
    private String accessToken;

    Client() throws URISyntaxException {
        super(new URI("http://localhost:8282"));
    }

    @Override
    public void onOpen(ServerHandshake handshakedata) {

    }

    @Override
    public void onMessage(String jsonResponse) {
        BaseResponse response = gson.fromJson(jsonResponse, BaseResponse.class);
        if(!callbacks.containsKey(response.getId())) {
            System.out.println(".. > An internal error has occurred.");
            return;
        }

        ResponseProcessingData rpd = callbacks.get(response.getId());

        if(response.getCode() == 200) {
            rpd.getOnSuccess().exec(jsonResponse);
        } else {
            rpd.getOnError().exec(jsonResponse);
        }

        callbacks.remove(response.getId());
    }

    @Override
    public void onClose(int i, String s, boolean b) {

    }

    @Override
    public void onError(Exception e) {
        System.out.println("A severe error has occurred.");
        // todo: remove print
        e.printStackTrace();
    }

    public void execShell() {
        Scanner in = new Scanner(System.in);

        showHeader();

        while (true) {
            cmdTokens = in.nextLine().trim().split(" ");

            if(cmdTokens.length > 0 && cmdTokens[0].isEmpty()) {
                System.out.print(requestNumber + " > ");
                continue;
            }

            if(cmdTokens.length > 0) {
                switch (cmdTokens[0]) {
                    case "reg":
                        processReg();
                        break;
                    case "auth":
                        processAuth();
                        break;
                    case "get":
                        processGet();
                        break;
                    case "add":
                        processAdd();
                        break;
                    case "del":
                        processDel();
                        break;
                    case "exit":
                        close();
                        return;
                    case "help":
                        showHelp();
                        break;
                    default:
                        System.out.println("Unknown command: '" + cmdTokens[0] + "'.");
                        System.out.println("Type help to see the list of available commands.");
                        break;
                }

                System.out.print(requestNumber + " > ");
            }
        }
    }

    private void processDel() {
        if(accessToken == null) {
            showAuthError();
            return;
        }

        if(cmdTokens.length < 3) {
            System.out.println("Invalid command syntax. Too few arguments.");
            return;
        }

        if(cmdTokens[1].equals("note")) {
            processDelNote();
            return;
        }

        if(cmdTokens[1].equals("ar")) {
            processDelAR();
            return;
        }

        System.out.println("Invalid command syntax.");
        System.out.println("Expected 'note' or 'ar', but '" + cmdTokens[1] + "' found.");
    }

    private void processDelNote() {
        String id = cmdTokens[2];
        RequestContainer<String> request = new RequestContainer<>(id);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.DELETE_NOTE.toString());
        request.setToken(accessToken);

        Callback onSuccess = jsonResponse -> {
            BaseResponse response = gson.fromJson(
                    jsonResponse,
                    BaseResponse.class
            );

            System.out.println("\n" + response.getId() + " > Note was deleted.");
            System.out.println(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processDelAR() {
        if(cmdTokens.length < 4) {
            System.out.println("Too few arguments.");
            return;
        }

        String userId = cmdTokens[2];
        String noteId = cmdTokens[3];
        AccessRight ar = new AccessRight(userId, noteId);

        RequestContainer<AccessRight> request = new RequestContainer<>(ar);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.REMOVE_ACCESS_RIGHT.toString());
        request.setToken(accessToken);

        Callback onSuccess = jsonResponse -> {
            BaseResponse response = gson.fromJson(jsonResponse, BaseResponse.class);

            System.out.println("\n" + response.getId() + " > " + "Access was terminated.");
            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processAdd() {
        if(accessToken == null) {
            showAuthError();
            return;
        }

        if(cmdTokens.length < 4) {
            System.out.println("Invalid command syntax. Too few arguments.");
            return;
        }

        if(cmdTokens[1].equals("note")) {
            processAddNote();
            return;
        }

        if(cmdTokens[1].equals("ar")) {
            processAddAR();
            return;
        }

        System.out.println("Invalid command syntax.");
        System.out.println("Expected 'note' or 'ar', but '" + cmdTokens[1] + "' found.");
    }

    private void processAddAR() {
        if(cmdTokens.length < 4) {
            System.out.println("Too few arguments.");
            return;
        }

        String userId = cmdTokens[2];
        String noteId = cmdTokens[3];
        AccessRight ar = new AccessRight(userId, noteId);

        RequestContainer<AccessRight> request = new RequestContainer<>(ar);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.ADD_ACCESS_RIGHT.toString());
        request.setToken(accessToken);

        Callback onSuccess = jsonResponse -> {
            BaseResponse response = gson.fromJson(jsonResponse, BaseResponse.class);

            System.out.println("\n" + response.getId() + " > " + "Access was set up.");
            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processAddNote() {
        CreateNotePayload payload = new CreateNotePayload();
        payload.setTitle(cmdTokens[2]);
        payload.setBody(cmdTokens[3]);

        RequestContainer<CreateNotePayload> request = new RequestContainer<>(payload);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.CREATE_NOTE.toString());
        request.setToken(accessToken);

        Callback onSuccess = jsonResponse -> {
            ResponseContainer<Note> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<Note>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + "Note was saved.");
            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processGet() {
        if(accessToken == null) {
            showAuthError();
            return;
        }

        if(cmdTokens.length < 2) {
            System.out.println("Invalid command syntax. Too few arguments.");
            return;
        }

        if(cmdTokens[1].equals("notes")) {
            processGetNotes();
            return;
        }

        if(cmdTokens[1].equals("users")) {
            processGetUsers();
            return;
        }

        System.out.println("Invalid command syntax.");
        System.out.println("Expected 'users' or 'notes', but '" + cmdTokens[1] + "' found.");
    }

    private void showAuthError() {
        System.out.println("You are note authorized to perform this action.");
    }

    private void processGetUsers() {
        RequestContainer request = new RequestContainer<>(null);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.GET_USERS.toString());
        request.setToken(accessToken);

        Callback onSuccess = jsonResponse -> {
            ResponseContainer<List<User>> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<List<User>>>(){}.getType()
            );

            List<User> users = response.getPayload();

            System.out.println("\n" + response.getId() + " > Registered users:");
            for(User user : users) {
                System.out.println("name: " + user.getName());
                System.out.println("email: " + user.getEmail());
                System.out.println();
            }

            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processGetNotes() {
        RequestContainer request = new RequestContainer<>(null);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.GET_NOTES.toString());
        request.setToken(accessToken);

        Callback onSuccess = jsonResponse -> {
            ResponseContainer<List<Note>> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<List<Note>>>(){}.getType()
            );

            List<Note> notes = response.getPayload();

            System.out.println("\n" + response.getId() + " > All notes:");
            for(Note note : notes) {
                System.out.println("id: " + note.getId());
                System.out.println("author: " + note.getAuthorId());
                System.out.println("title: " + note.getTitle());
                System.out.println("body: " + note.getBody());
                System.out.println();
            }

            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processAuth() {
        if(cmdTokens.length < 3) {
            System.out.println("Invalid command syntax. Too few arguments.");
            return;
        }

        String email = cmdTokens[1];
        String pass = cmdTokens[2];

        AuthPayload payload = new AuthPayload(email, pass);

        RequestContainer<AuthPayload> request = new RequestContainer<>(payload);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.AUTHORIZATION.toString());

        Callback onSuccess = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            accessToken = response.getPayload();

            System.out.println("\n" + response.getId() + " > You have been authorized.");
            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private void processReg() {
        if(cmdTokens.length < 4) {
            System.out.println("Invalid command syntax. Too few arguments.");
            return;
        }

        String name = cmdTokens[1];
        String email = cmdTokens[2];
        String pass = cmdTokens[3];

        RegPayload payload = new RegPayload(email, pass, name);

        RequestContainer<RegPayload> request = new RequestContainer<>(payload);
        request.setId(Integer.toString(requestNumber++));
        request.setAction(Action.REGISTRATION.toString());

        Callback onSuccess = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                jsonResponse,
                new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            accessToken = response.getPayload();

            System.out.println("\n" + response.getId() + " > You have been registered and authorized.");
            System.out.print(requestNumber + " > ");
        };

        Callback onError = jsonResponse -> {
            ResponseContainer<String> response = gson.fromJson(
                    jsonResponse,
                    new TypeToken<ResponseContainer<String>>(){}.getType()
            );

            System.out.println("\n" + response.getId() + " > " + getErrMsg(response.getPayload()));
            System.out.print(requestNumber + " > ");
        };

        callbacks.put(request.getId(), new ResponseProcessingData(String.class, onSuccess, onError));

        send(gson.toJson(request));
    }

    private String getErrMsg(String errKeyword) {
        // todo: implement
        return errKeyword;
    }

    private void showHeader() {
        System.out.println("ETU LETI; HP Course; Lab #2.");
        System.out.println("Interactive client shell.");
        System.out.println("Type help to see the list of available commands.");
        System.out.println();
        System.out.print(requestNumber + " > ");
    }

    private void showHelp() {
        System.out.println(
                "Type one of the following:\n" +
                "    reg <name> <email> <pass>     : to register\n" +
                "    auth <email> <pass>           : to authenticate\n" +
                "    help                          : to see this help\n" +
                "    exit                          : to exit the shell\n" +
                "    add note <title> <body>       : to add a new note\n" +
                "    add ar <username> <noteid>    : to allow someone to see the given note\n" +
                "    del note <noteid>             : to delete a note\n" +
                "    del ar <username> <noteid>    : to terminate access rights\n" +
                "    get notes                     : to get all available notes\n" +
                "    get users                     : to get the list of registered users\n"
        );
    }
}