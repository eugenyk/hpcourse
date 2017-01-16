package server.dao;

import server.DI;
import server.dao.BaseService;
import server.entities.model.AccessRight;
import server.entities.model.Note;
import org.hibernate.Session;
import org.hibernate.criterion.Restrictions;

import java.util.List;

public class NoteService extends BaseService<Note> {
    public Note get(String id) {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        Note note = (Note)session
                .createCriteria(Note.class)
                .add(Restrictions.eq("id", id))
                .uniqueResult();

        session.close();
        return note;
    }

    public List<Note> getNotesForUser(String userId) {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        List<Note> notes = session
                .createCriteria(Note.class)
                .add(Restrictions.eq("authorId", userId))
                .list();

        session.close();

        List<AccessRight> rights = DI.instance.accessRightDAO.getUsersRights(userId);
        for(AccessRight right : rights) {
            Note note = DI.instance.noteDAO.get(right.getNoteId());
            notes.add(note);
        }

        return notes;
    }
}
