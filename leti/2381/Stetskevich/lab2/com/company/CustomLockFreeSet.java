package com.company;

import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * @author Stetskevich Roman
 * @sinse 1.2
 */
public class CustomLockFreeSet<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> head;
    private final Node<T> tail;

    CustomLockFreeSet() {
        tail = new Node<>(null);
        head = new Node<>(null);
        head.next.set(tail, false);
    }

    @Override
    public boolean add(T value) {
        final Node<T> newNode = new Node<>(value);
        while (true) {
            Node<T> prev = findPrevious(value);
            Node<T> cur = prev.next.getReference();
            if (equals(cur.value, value)) {
                return false;
            } else {
                newNode.next.set(cur, false);
                if (prev.next.compareAndSet(cur, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Node<T> prev = findPrevious(value);
            Node<T> cur = prev.next.getReference();
            if (!equals(cur.value, value)) {
                return false;
            }
            Node<T> next = cur.next.getReference();

            if (cur.next.compareAndSet(next, next, false, true)) {
                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        Node<T> cur = findPrevious(value).next.getReference();
        return (equals(cur.value, value) && !cur.next.isMarked());
    }

    @Override
    public boolean isEmpty() {
        return tail == head.next.getReference();
    }

    private Node<T> findPrevious(T value) {
        if (isEmpty()) {
            return head;
        }
        Node<T> previous, current, next;
        boolean[] marked = {false};
        while (true) {
            previous = head;
            current = previous.next.getReference();
            while (true) {
                next = current.next.get(marked);
                while (marked[0]) {
                    if (!previous.next.compareAndSet(current, next, false, false)) {
                        findPrevious(value);
                    }
                    current = next;
                    next = current.next.get(marked);
                }

                if (current == tail || value.compareTo(current.value) == 0) {
                    return previous;
                }
                previous = current;
                current = next;
            }
        }
    }

    private boolean equals(T value1, T value2) {
        return value1 != null && value1.compareTo(value2) == 0;
    }

    private class Node<T> {
        private T value;
        private AtomicMarkableReference<Node<T>> next;

        Node(T value) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(null, false);
        }
    }

}