package server;

import server.controllers.NoteController;
import server.controllers.UserController;
import server.dao.AccessRightService;
import server.dao.NoteService;
import server.dao.UserService;
import com.google.gson.Gson;

/* Very simple implementation of DI */
public class DI {
    public static final DI instance = new DI();
    private DI() { }

    public final UserService userDAO = new UserService();
    public final NoteService noteDAO = new NoteService();
    public final AccessRightService accessRightDAO = new AccessRightService();
    public final TokenValidator tokenValidator = new TokenValidator();
    public final ProtocolValidator protocolValidator = new ProtocolValidator();
    public final RequestProcessor requestProcessor = new RequestProcessor();
    public final Session session = new Session();
    public final Gson gson = new Gson();
    public final UserController userController = new UserController();
    public final NoteController noteController = new NoteController();
}
