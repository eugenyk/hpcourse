import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class LockFreeSetStandardLibImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private static final Object EXISTS = new Object();
    private ConcurrentMap<T, Object> s = new ConcurrentHashMap<>();

    @Override
    public synchronized boolean add(T value) {
        return s.put(value, EXISTS) == null;
    }

    @Override
    public synchronized boolean remove(T value) {
        return s.remove(value) != null;
    }

    @Override
    public synchronized boolean contains(T value) {
        return s.containsKey(value);
    }

    @Override
    public synchronized boolean isEmpty() {
        return s.isEmpty();
    }
}
