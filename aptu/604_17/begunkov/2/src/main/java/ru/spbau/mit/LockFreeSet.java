package ru.spbau.mit;

public interface LockFreeSet<T> {
    boolean isEmpty();

    boolean append(T value);

    boolean remove(T value);

    boolean contains(T value);
}
