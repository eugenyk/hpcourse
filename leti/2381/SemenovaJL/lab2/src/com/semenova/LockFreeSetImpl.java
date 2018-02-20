package com.semenova;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> head;
    private final Node<T> tail;

    LockFreeSetImpl() {
        tail = new Node<>(null);
        head = new Node<>(null);
        head.next.set(tail, false);
    }

    static class Node<T> {
        final T value;
        final AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);

        Node(T value) {
            this.value = value;
        }
    }

    static class NodesPair<T> {
        Node<T> prev;
        Node<T> cur;

        NodesPair(Node<T> prev, Node<T> cur) {
            this.prev = prev;
            this.cur = cur;
        }
    }

    public boolean add(T value) {
        final Node<T> newNode = new Node<>(value);
        while (true) {
            final NodesPair<T> nodesPair = search(value);
            final Node<T> prev = nodesPair.prev;
            final Node<T> cur = nodesPair.cur;

            if (cur.value == value) {
                return false;
            } else {
                newNode.next.set(cur, false);
                if (prev.next.compareAndSet(cur, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    public boolean remove(T value) {
        while (true) {
            final NodesPair<T> nodesPair = search(value);
            final Node<T> cur = nodesPair.cur;
            if (cur.value != value) {
                return false;
            }
            final Node<T> next = cur.next.getReference();
            if (cur.next.compareAndSet(next, next, false, true)) {
                return true;
            }
        }
    }

    private NodesPair<T> search(T value) {
        Node<T> prev;
        Node<T> cur;
        Node<T> next;
        boolean[] marked = {false};

        if (isEmpty()) {
            return new NodesPair<>(head, tail);
        }

        retry:
        while (true) {
            prev = head;
            cur = prev.next.getReference();
            while (true) {
                next = cur.next.get(marked);
                while (marked[0]) {
                    if (!prev.next.compareAndSet(cur, next, false, false)) {
                        continue retry;
                    }
                    cur = next;
                    next = cur.next.get(marked);
                }
                if (cur == tail || value.compareTo(cur.value) <= 0) {
                    return new NodesPair<>(prev, cur);
                }
                prev = cur;
                cur = next;
            }
        }
    }

    public boolean contains(T value) {
        final NodesPair<T> nodesPair = search(value);
        final Node<T> cur = nodesPair.cur;

        return (cur.value == value && !cur.next.isMarked());
    }

    public boolean isEmpty() {
        return head.next.getReference() == tail;
    }

    public java.util.ArrayList<T> getAll() {
        java.util.ArrayList<T> result = new java.util.ArrayList<>();
        Node<T> cur = head.next.getReference();
        while (cur != tail) {
            if (contains(cur.value)) result.add(cur.value);
            cur = cur.next.getReference();
        }
        return result;
    }

    public int size(){
        return this.getAll().size();
    }
}