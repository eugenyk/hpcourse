package server.controllers;


import server.DI;
import server.entities.response.ResponseContainer;
import server.entities.model.User;
import server.errors.Errors;
import server.errors.RequestProcessingException;

import java.util.UUID;

public class UserController {
    public ResponseContainer<String> authorize(String email, String pwdHash)
            throws RequestProcessingException
    {
        if (email == null) {
            throw new RequestProcessingException(Errors.ERR_INVALID_REQUEST.toString());
        }

        User user = DI.instance.userDAO.getByEmail(email);

        if (user == null || !user.getPwdHash().equals(pwdHash)) {
            throw new RequestProcessingException(Errors.ERR_AUTH_ERROR.toString());
        }

        String token = UUID.randomUUID().toString() + UUID.randomUUID().toString();
        DI.instance.session.addUser(user, token);

        return new ResponseContainer<>(token);
    }

    public ResponseContainer<String> registration(String name, String email, String pwdHash)
            throws RequestProcessingException
    {
        if (email == null || pwdHash == null || name == null) {
            throw new RequestProcessingException(Errors.ERR_INVALID_REQUEST.toString());
        }

        User user = DI.instance.userDAO.getByEmail(email);

        if (user != null) {
            throw new RequestProcessingException(Errors.ERR_USER_EXIST.toString());
        }

        if (email.isEmpty() || pwdHash.isEmpty() || name.isEmpty()) {
            throw new RequestProcessingException(Errors.ERR_EMPTY_REQUEST.toString());
        }

        user = new User(UUID.randomUUID().toString(), name, email, pwdHash);
        DI.instance.userDAO.add(user);

        String token = UUID.randomUUID().toString() + UUID.randomUUID().toString();
        DI.instance.session.addUser(user, token);

        return new ResponseContainer<>(token);
    }

    public ResponseContainer getAllUsers() {
        return new ResponseContainer<>(DI.instance.userDAO.getAllUsers());
    }
}
