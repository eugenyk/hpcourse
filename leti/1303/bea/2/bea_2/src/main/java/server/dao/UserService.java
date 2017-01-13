package server.dao;

import server.dao.BaseService;
import server.entities.model.User;
import org.hibernate.Session;
import org.hibernate.criterion.Restrictions;

import java.util.List;

public class UserService extends BaseService<User> {
    public List<User> getAllUsers() {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        List<User> users = session
                .createCriteria(User.class)
                .list();

        session.close();
        return users;
    }

    public User getByEmail(String email) {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        User user = (User)session
                .createCriteria(User.class)
                .add(Restrictions.eq("email", email))
                .uniqueResult();

        session.close();
        return user;
    }
}
