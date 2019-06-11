package net.vasalf.hw.lockfree;

import java.util.*;
import java.util.stream.Collectors;

public class SynchronizedSet<T extends Comparable<T>> implements MySet<T> {
    private TreeSet<T> wrapped;

    SynchronizedSet() {
        wrapped = new TreeSet<>();
    }

    @Override
    public synchronized boolean add(T value) {
        return wrapped.add(value);
    }

    @Override
    public synchronized boolean remove(T value) {
        return wrapped.remove(value);
    }

    @Override
    public synchronized boolean contains(T value) {
        return wrapped.contains(value);
    }

    @Override
    public synchronized boolean isEmpty() {
        return wrapped.isEmpty();
    }

    @Override
    public synchronized Iterator<T> iterator() {
        List<T> ret = new ArrayList<>(wrapped);
        return ret.iterator();
    }
}
