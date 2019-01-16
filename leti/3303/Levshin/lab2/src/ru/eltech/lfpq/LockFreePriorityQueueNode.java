package ru.eltech.lfpq;

import java.util.concurrent.atomic.AtomicMarkableReference;

class LockFreePriorityQueueNode<E extends   Comparable<E>>{

    E value;
    AtomicMarkableReference<LockFreePriorityQueueNode<E>> next;

    LockFreePriorityQueueNode(E value)
    {
        this.value = value;
        this.next = new AtomicMarkableReference<>(null, false);
    }

    LockFreePriorityQueueNode(E value, LockFreePriorityQueueNode<E> next)
    {
        this.value = value;
        this.next = new AtomicMarkableReference<>(next, false);
    }

}
