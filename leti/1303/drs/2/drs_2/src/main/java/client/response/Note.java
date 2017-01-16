package client.response;

public class Note {
    private String id;

    private String body;

    private String title;

    private String authorId;

    public Note(String id, String body, String title, String authorId) {
        this.id = id;
        this.body = body;
        this.title = title;
        this.authorId = authorId;
    }

    public Note() {
        this("", "", "", "");
    }

    public String getAuthorId() {
        return authorId;
    }

    public void setAuthorId(String authorId) {
        this.authorId = authorId;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getBody() {
        return body;
    }

    public void setBody(String body) {
        this.body = body;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }
}
