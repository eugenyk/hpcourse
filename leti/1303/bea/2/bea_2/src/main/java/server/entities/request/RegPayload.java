package server.entities.request;


public class RegPayload {
    private String email;
    private String pwdHash;
    private String name;

    public String getEmail() {
        return email;
    }

    public String getPwdHash() {
        return pwdHash;
    }

    public String getName() {
        return name;
    }

    public RegPayload(String email, String pwdHash, String name) {
        this.email = email;
        this.pwdHash = pwdHash;
        this.name = name;
    }

    public void setEmail(String email) {

        this.email = email;
    }

    public void setPwdHash(String pwdHash) {
        this.pwdHash = pwdHash;
    }

    public void setName(String name) {
        this.name = name;
    }
}
