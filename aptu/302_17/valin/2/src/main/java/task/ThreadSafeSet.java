package task;

import java.util.concurrent.atomic.AtomicStampedReference;
import java.util.logging.Logger;

public class ThreadSafeSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private static final int CLEAR = 0;
    private static final int REMOVE = 1;
    private static final int MOVE = 2;
    private final Logger logger = Logger.getLogger(getClass().getName());
    private final Node<T> root = new Node<>();

    private Node<T> findPrev(T obj) {
        Node<T> cur = root;
        Node<T> next = root.next.getReference();
        while (next != null && !obj.equals(next.obj)) {
            cur = next;
            next = cur.next.getReference();
        }

        return cur;
    }

    public boolean add(T value) {
        Node<T> node = new Node<>(value);
        while (true) {
            Node<T> prev = findPrev(value);
            Node<T> next = prev.next.getReference();

            if (next != null && value.equals(next.obj)) {
                    return false;
            }

            if (prev.next.compareAndSet(null, node, CLEAR, CLEAR)) {
                return true;
            }
        }
    }

    public boolean remove(T value) {
        while (true) {
            Node<T> prev = findPrev(value);
            Node<T> next = prev.next.getReference();

            if (next == null) {
                return false;
            }

            if (!value.equals(next.obj)) {
                continue;
            }

            if (!prev.next.compareAndSet(next, next, CLEAR, REMOVE)) {
                continue;
            }

            Node<T> nnext = next.next.getReference();
            if (!next.next.compareAndSet(nnext, nnext, CLEAR, MOVE)) {
                prev.next.set(next, CLEAR);
                continue;
            }

            prev.next.set(nnext, CLEAR);

            return true;
        }
    }

    public boolean contains(T value) {
        return findPrev(value).next.getReference() != null;
    }

    public boolean isEmpty() {
        return root.next.getReference() == null;
    }

    private static class Node<T> {
        final T obj;
        AtomicStampedReference<Node<T>> next = new AtomicStampedReference<Node<T>>(null, CLEAR);

        Node() {
            obj = null;
        }

        Node(T o) {
            obj = o;
        }
    }
}