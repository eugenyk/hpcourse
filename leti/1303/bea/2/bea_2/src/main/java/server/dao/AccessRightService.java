package server.dao;

import org.hibernate.Session;
import org.hibernate.criterion.Restrictions;
import server.entities.model.AccessRight;
import server.entities.model.Note;

import java.util.List;

public class AccessRightService extends BaseService<AccessRight> {
    List<AccessRight> getUsersRights(String userId) {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        List<AccessRight> accessRights = session
                .createCriteria(AccessRight.class)
                .add(Restrictions.eq("userId", userId))
                .list();

        session.close();
        return accessRights;
    }

    public void deleteByIds(AccessRight entity) {
            Session session = sessionFactory.openSession();
            session.beginTransaction();

            AccessRight ar = (AccessRight) session
                    .createCriteria(AccessRight.class)
                    .add(Restrictions.eq("userId", entity.getUserId()))
                    .add(Restrictions.eq("noteId", entity.getNoteId()))
                    .uniqueResult();

            session.close();

            delete(ar);
    }
}
