package ru.eltech.lfpq;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueueNode<E extends   Comparable<E>>{

    public E value;
    public AtomicMarkableReference<LockFreePriorityQueueNode<E>> next;

    public LockFreePriorityQueueNode(E value)
    {
        this.value = value;
        this.next = new AtomicMarkableReference<>(null, false);
    }

    public LockFreePriorityQueueNode(E value, LockFreePriorityQueueNode<E> next)
    {
        this.value = value;
        this.next = new AtomicMarkableReference<>(next, false);
    }

}
