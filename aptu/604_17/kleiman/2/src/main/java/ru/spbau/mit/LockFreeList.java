package ru.spbau.mit;

public interface LockFreeList<T extends Comparable<T>> {
    boolean isEmpty();

    boolean add(T value);

    boolean remove(T value);

    boolean contains(T value);
}