package server.controllers;

import server.DI;
import server.entities.model.Note;
import server.entities.response.ResponseContainer;
import server.errors.Errors;
import server.errors.RequestProcessingException;

import java.util.List;
import java.util.UUID;

public class NoteController {
    public ResponseContainer<Note> createNote(String title, String body, String id) throws RequestProcessingException {
        if (title == null || body == null) {
            throw new RequestProcessingException(Errors.ERR_INVALID_REQUEST.toString());
        }

        if (title.isEmpty() || body.isEmpty()) {
            throw new RequestProcessingException(Errors.ERR_EMPTY_REQUEST.toString());
        }

        Note note = new Note(UUID.randomUUID().toString(), body, title, id);
        DI.instance.noteDAO.add(note);

        return new ResponseContainer<>(note);
    }

    public ResponseContainer<Note> deleteNote(String noteId) throws RequestProcessingException {
        if (noteId == null) {
            throw new RequestProcessingException(Errors.ERR_INVALID_REQUEST.toString());
        }

        Note note = DI.instance.noteDAO.get(noteId);

        if (note == null) {
            throw new RequestProcessingException(Errors.ERR_ID_NOT_FOUND.toString());
        }

        DI.instance.noteDAO.delete(note);
        return new ResponseContainer<>(note);
    }

    public ResponseContainer<List<Note>> getNotes(String userId) throws RequestProcessingException {
        return new ResponseContainer<>(DI.instance.noteDAO.getNotesForUser(userId));
    }
}
