package ru.spbhse.karvozavr.lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Lock-free set implementation using linked list.
 *
 * @param <T> Key type
 */
public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {

    /**
     * List node.
     */
    private class Node {

        final T value;

        AtomicMarkableReference<Node> next = new AtomicMarkableReference<>(null, false);

        Node(T value) {
            this.value = value;
        }
    }

    /**
     * Markable reference to the head of the list.
     *
     * Head is fictive Node with null value.
     */
    private final AtomicMarkableReference<Node> head = new AtomicMarkableReference<>(new Node(null), false);

    @Override
    public boolean add(T value) {
        if (value == null) {
            throw new NullPointerException("Error: null values are not allowed in set!");
        }

        Node node = new Node(value);

        while (true) {
            Pair<Node> prevAndCurrent = find(value);
            Node prev = prevAndCurrent.first;
            Node current = prevAndCurrent.second;

            if (current != null) {
                return false;
            } else {
                if (prev.next.compareAndSet(null, node, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        if (value == null) {
            return false;
        }

        while (true) {
            Pair<Node> prevAndCurrent = find(value);
            Node prev = prevAndCurrent.first;
            Node current = prevAndCurrent.second;

            if (current != null) {
                if (current.next.attemptMark(current.next.getReference(), true)) {
                    physicallyRemove(prev, current);
                    return true;
                }
            } else {
                return false;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        if (value == null) {
            return false;
        }

        Node node = find(value).second;

        return node != null;
    }

    @Override
    public boolean isEmpty() {
        return head.getReference().next.getReference() == null;
    }

    @Override
    public java.util.Iterator<T> iterator() {
        return null;
    }

    private Pair<Node> find(T value) {
        Node prev, node;
        for (prev = head.getReference(), node = prev.next.getReference();
             node != null; prev = node, node = node.next.getReference()) {
            if (!isRemoved(node) && value.equals(node.value)) {
                return new Pair<>(prev, node);
            }
        }

        return new Pair<>(prev, null);
    }

    private boolean isRemoved(Node node) {
        return node.next.isMarked();
    }

    private void physicallyRemove(Node prev, Node node) {
        Node next = node;
        do {
            next = next.next.getReference();
        } while (next != null && isRemoved(next));

        prev.next.compareAndSet(node, next, false, false);
    }
}
