package client.request;

public class AuthPayload {
    private String email;
    private String pwdHash;

    public AuthPayload() {
        this("", "");
    }

    public AuthPayload(String email, String pwdHash) {
        this.email = email;
        this.pwdHash = pwdHash;
    }

    public String getEmail() {
        return email;
    }

    public String getPwdHash() {
        return pwdHash;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public void setPwdHash(String pwdHash) {
        this.pwdHash = pwdHash;
    }
}
