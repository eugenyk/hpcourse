package ru.spbau.mit;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.PriorityQueue;

public class PriorityQueueLockFreeImplMok<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueueLockFree<E> {
    private final PriorityQueue<E> pq = new PriorityQueue<>();

    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
        synchronized (pq) {
            return pq.size();
        }
    }

    @Override
    public boolean offer(E e) {
        synchronized (pq) {
            return pq.offer(e);
        }
    }

    @Override
    public E poll() {
        synchronized (pq) {
            return pq.poll();
        }
    }

    @Override
    public E peek() {
        synchronized (pq) {
            return pq.peek();
        }
    }
}
