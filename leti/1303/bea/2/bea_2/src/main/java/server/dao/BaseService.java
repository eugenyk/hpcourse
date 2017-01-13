package server.dao;

import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.boot.MetadataSources;
import org.hibernate.boot.registry.StandardServiceRegistry;
import org.hibernate.boot.registry.StandardServiceRegistryBuilder;

public class BaseService<E> {
    protected static SessionFactory sessionFactory;

    static {
        final StandardServiceRegistry registry = new StandardServiceRegistryBuilder()
                .configure()
                .build();

        sessionFactory = new MetadataSources( registry ).buildMetadata().buildSessionFactory();
    }

    public void add(E entity) {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        session.save(entity);

        session.getTransaction().commit();
        session.close();
    }

    public void delete(E entity) {
        Session session = sessionFactory.openSession();
        session.beginTransaction();

        session.delete(entity);

        session.getTransaction().commit();
        session.close();
    }
}