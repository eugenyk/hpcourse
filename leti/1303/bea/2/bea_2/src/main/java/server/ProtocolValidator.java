package server;

import server.entities.request.BaseRequest;
import server.errors.ProtocolValidationException;

class ProtocolValidator {
    void validate(BaseRequest baseRequest) throws ProtocolValidationException {
        if (baseRequest.getId() == null || baseRequest.getAction() == null) {
            throw new ProtocolValidationException();
        }

        for (Action action : Action.values()) {
            if (action.toString().equals(baseRequest.getAction())) {
                return;
            }
        }

        throw new ProtocolValidationException();
    }
}
