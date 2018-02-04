package ru.spbau.mit;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private static class Node<E> {
        private final E key;
        private final AtomicMarkableReference<Node<E>> next =
                new AtomicMarkableReference<Node<E>>(null, false);

        private Node(E _key) {
            key = _key;
        }
    }

    private Node<T> head = new Node<>(null);
    private Node<T> tail = new Node<>(null);

    public LockFreeSetImpl() {
        head.next.set(tail, false);
    }

    @Override
    public boolean append(final T key) {
        if (key == null) {
            throw new IllegalArgumentException("LockFreeSet: null is not allowed");
        }

        final Node<T> toAppend = new Node<>(key);

        do {
            final Node<T>[] result = find(key);
            final Node<T> left = result[0];
            final Node<T> right = result[1];

            if (right != tail && right.key.compareTo(key) == 0) {
                return false; // already here
            }

            toAppend.next.set(right, false);
            if (left.next.compareAndSet(right, toAppend, false, false)) {
                return true;
            }
        } while (true);
    }

    @Override
    public boolean remove(final T key) {
        Node<T> right;
        Node<T> next;

        do {
            final Node<T>[] result = find(key);
            right = result[1];
            next = right.next.getReference();

            if (right == tail) {
                return false;
            } else if (right.key.compareTo(key) != 0) {
                return false;
            }
        } while (!right.next.compareAndSet(next, next, false, true));

//        left.next.compareAndSet(right, rightNext, false, false);
        return true;
    }

    @Override
    public boolean contains(final T key) {
        if (key == null) {
            throw new IllegalArgumentException("LockFreeSet: null is not allowed");
        }

        final Node<T>[] result = find(key);
        return result[1] != tail && result[1].key.compareTo(key) == 0;
    }

    @Override
    public boolean isEmpty() {
        final boolean[] f = new boolean[1];
        Node<T> cur = head.next.getReference();

        do {
            Node<T> next = cur.next.get(f);
            if (cur == tail) {
                return true;
            } else if (!f[0]) {
                return false;
            }
            cur = next;
        } while (true);
    }


    private Node<T>[] find(final T key) {
        Node<T> left = head;
        Node<T> leftNext = null;
        Node<T> right;

        do {
            right = head;

            do {
                Node<T> rightNext = right.next.getReference();
                if (!right.next.isMarked()) {
                    left = right;
                    leftNext = rightNext;
                }
                right = rightNext;
            } while (right != tail && (right.next.isMarked() || right.key.compareTo(key) < 0));

            if (leftNext == right) {
                return (Node<T>[])new Node[] {left, right};
            }

            // cleanup
            final boolean isMarked = left.next.isMarked();
            left.next.compareAndSet(leftNext, right, isMarked, isMarked);
        } while (true);
    }
}
