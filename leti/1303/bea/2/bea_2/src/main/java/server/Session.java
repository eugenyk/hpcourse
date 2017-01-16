package server;

import server.entities.model.User;

import java.util.HashMap;
import java.util.Map;

public class Session {
    private Map<String, User> map = new HashMap<>();

    public void addUser(User user, String token) {
        map.put(token, user);
    }

    public User getUser(String token) {
        return map.containsKey(token) ? map.get(token) : null;
    }
}
