package ru.parallel;

import java.util.concurrent.atomic.AtomicStampedReference;

public class Node<T> {
    private final T value;

    private final AtomicStampedReference<Node<T>> next = new AtomicStampedReference<>(null, 1);

    public Node(T value) {
        this.value = value;
    }

    public T getValue() {
        return value;
    }


    public Node<T> getNextNode() {
        return next.getReference();
    }

    public int getNodeVersion() {
        return next.getStamp();
    }

    public AtomicStampedReference<Node<T>> getLink() {
        return next;
    }
}
