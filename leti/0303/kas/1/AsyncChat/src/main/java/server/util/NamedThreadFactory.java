package server.util;

import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;


public class NamedThreadFactory implements ThreadFactory {
    private final AtomicInteger id = new AtomicInteger(1);
    private String name;

    public NamedThreadFactory(String name) {
        this.name = name;
    }

    @Override
    public Thread newThread(Runnable r) {
        final Thread thread = new Thread(r);
        thread.setName(name + "-" + id.incrementAndGet());
        return thread;
    }
}
