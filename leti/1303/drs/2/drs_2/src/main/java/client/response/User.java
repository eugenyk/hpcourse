package client.response;

public class User {
    private String id;

    private String name;

    private String email;

    private String pwdHash;

    public User(String id, String name, String email, String pwdHash) {
        this.id = id;
        this.name = name;
        this.email = email;
        this.pwdHash = pwdHash;
    }

    public User() {
        this("", "", "", "");
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getId() {
        return email;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public String getPwdHash() {
        return pwdHash;
    }

    public void setPwdHash(String pwdHash) {
        this.pwdHash = pwdHash;
    }
}
