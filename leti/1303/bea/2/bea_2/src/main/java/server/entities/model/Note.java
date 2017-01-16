package server.entities.model;


import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name = "notes")
public class Note {
    @Id
    private String id;

    @Column(name = "body")
    private String body;

    @Column(name = "title")
    private String title;

    @Column(name = "authorId")
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
