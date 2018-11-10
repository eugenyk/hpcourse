package ru.ifmo.priorityqueue;

import java.util.AbstractQueue;
import java.util.Iterator;

public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

    public LockFreePriorityQueue() {

    }

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        return 0;
    }

    @Override
    public boolean offer(E e) {
        return false;
    }

    @Override
    public E poll() {
        return null;
    }

    @Override
    public E peek() {
        return null;
    }
}
