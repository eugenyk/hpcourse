package server.entities.model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;
import java.util.UUID;

@Entity
@Table(name = "arights")
public class AccessRight {
    @Id
    String id;

    @Column(name = "userId")
    private String userId;

    @Column(name = "noteId")
    private String noteId;

    public AccessRight(String id, String userId, String noteId) {
        this.id = id;
        this.userId = userId;
        this.noteId = noteId;
    }

    public AccessRight() {
        this("", "");
    }

    public AccessRight(String userId, String noteId) {
        this.userId = userId;
        this.noteId = noteId;
        id = UUID.randomUUID().toString();
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getNoteId() {
        return noteId;
    }

    public void setNoteId(String noteId) {
        this.noteId = noteId;
    }
}
