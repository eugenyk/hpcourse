package server;

import server.entities.model.AccessRight;
import server.entities.model.Note;
import server.entities.request.*;
import server.entities.response.BaseResponse;
import server.entities.response.ResponseContainer;
import server.entities.model.User;
import com.google.gson.reflect.TypeToken;
import server.errors.Errors;
import server.errors.RequestProcessingException;

class RequestProcessor {
    BaseResponse performAction(String requestInJson, Action action) throws RequestProcessingException {
        switch (action) {
            case AUTHORIZATION:
                return processAuthorizationRequest(requestInJson);

            case REGISTRATION:
                return processRegistrationRequest(requestInJson);

            case GET_USERS:
                return processGetUsersRequest();

            case CREATE_NOTE:
                return processCreateNoteRequest(requestInJson);

            case DELETE_NOTE:
                return processDeleteNoteRequest(requestInJson);

            case GET_NOTES:
                return processGetNotesRequest(requestInJson);

            case ADD_ACCESS_RIGHT:
                return processAddAccessRequest(requestInJson);

            case REMOVE_ACCESS_RIGHT:
                return processRemoveAccessRequest(requestInJson);
        }

        throw new RequestProcessingException(Errors.ERR_UNKNOWN_ACTION.toString());
    }

    private BaseResponse processAddAccessRequest(String requestInJson) throws RequestProcessingException {
        RequestContainer<AccessRight> request = DI.instance.gson.fromJson(
                requestInJson,
                new TypeToken<RequestContainer<AccessRight>>(){}.getType()
        );

        String id = request.getPayload().getNoteId();
        String uId = DI.instance.noteDAO.get(id).getAuthorId();
        if (uId.equals(DI.instance.session.getUser(request.getToken()).getId())) {
            DI.instance.accessRightDAO.add(request.getPayload());
            return new ResponseContainer<>("");
        }

        throw new RequestProcessingException(Errors.ERR_AUTH_ERROR.toString());
    }

    private ResponseContainer processRemoveAccessRequest(String requestInJson) throws RequestProcessingException {
        RequestContainer<AccessRight> request = DI.instance.gson.fromJson(
                requestInJson,
                new TypeToken<RequestContainer<AccessRight>>(){}.getType()
        );


        String id = request.getPayload().getNoteId();
        String uId = DI.instance.noteDAO.get(id).getAuthorId();
        if (uId.equals(DI.instance.session.getUser(request.getToken()).getId())) {
            DI.instance.accessRightDAO.deleteByIds(request.getPayload());
            return new ResponseContainer<>("");
        }

        throw new RequestProcessingException(Errors.ERR_AUTH_ERROR.toString());
    }

    private ResponseContainer processGetNotesRequest(String requestInJson) throws RequestProcessingException {
        BaseRequest request = DI.instance.gson.fromJson(requestInJson, BaseRequest.class);
        User user = DI.instance.session.getUser(request.getToken());
        return DI.instance.noteController.getNotes(user.getId());
    }

    private ResponseContainer processDeleteNoteRequest(String requestInJson) throws RequestProcessingException {
        RequestContainer<String> request = DI.instance.gson.fromJson(
                requestInJson,
                new TypeToken<RequestContainer<String>>(){}.getType()
        );

        String noteId = request.getPayload();

        Note note = DI.instance.noteDAO.get(noteId);
        User user = DI.instance.session.getUser(request.getToken());

        if(!user.getId().equals(note.getAuthorId())) {
            throw new RequestProcessingException(Errors.ERR_AUTH_ERROR.toString());
        }

        return DI.instance.noteController.deleteNote(noteId);
    }

    private ResponseContainer processCreateNoteRequest(String requestInJson) throws RequestProcessingException {
        RequestContainer<CreateNotePayload> request = DI.instance.gson.fromJson(
                requestInJson,
                new TypeToken<RequestContainer<CreateNotePayload>>(){}.getType()
        );

        User user = DI.instance.session.getUser(request.getToken());

        return DI.instance.noteController.createNote(
                request.getPayload().getTitle(),
                request.getPayload().getBody(),
                user.getId()
        );
    }

    private ResponseContainer processGetUsersRequest() {
        return DI.instance.userController.getAllUsers();
    }

    private ResponseContainer processAuthorizationRequest(String requestInJson) throws RequestProcessingException {
        RequestContainer<AuthPayload> request = DI.instance.gson.fromJson(
                requestInJson,
                new TypeToken<RequestContainer<AuthPayload>>(){}.getType()
        );

        return DI.instance.userController.authorize(
                request.getPayload().getEmail(),
                request.getPayload().getPwdHash()
        );
    }

    private ResponseContainer processRegistrationRequest(String requestInJson) throws RequestProcessingException {
        RequestContainer<RegPayload> request = DI.instance.gson.fromJson(
                requestInJson,
                new TypeToken<RequestContainer<RegPayload>>(){}.getType()
        );

        return DI.instance.userController.registration(
                request.getPayload().getName(),
                request.getPayload().getEmail(),
                request.getPayload().getPwdHash()
        );
    }
}
