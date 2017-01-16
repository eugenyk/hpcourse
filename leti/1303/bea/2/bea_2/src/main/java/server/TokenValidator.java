package server;

import server.entities.request.BaseRequest;
import server.errors.TokenValidationException;

class TokenValidator {
    private static final Action[] UNAUTHORIZED_ACTIONS = new Action[] {
            Action.AUTHORIZATION,
            Action.REGISTRATION
    };

    void validate(BaseRequest baseRequest) throws TokenValidationException {
        for (Action action : UNAUTHORIZED_ACTIONS) {
            if (baseRequest.getAction().equalsIgnoreCase(action.toString())) {
                return;
            }
        }

        if (baseRequest.getToken() == null || DI.instance.session.getUser(baseRequest.getToken()) == null) {
            throw new TokenValidationException();
        }
    }
}
