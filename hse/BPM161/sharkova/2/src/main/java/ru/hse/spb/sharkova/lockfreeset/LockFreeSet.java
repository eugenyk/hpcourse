package ru.hse.spb.sharkova.lockfreeset;

import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicStampedReference;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private static final int REMOVED = -1;
    private static final int NEW = 0;

    private final AtomicStampedReference<Node> head =
            new AtomicStampedReference<>(new Node(null), NEW);

    @Override
    public boolean add(@NotNull T value) {
        Node node = new Node(value);

        while (true) {
            Pair<Node, Node> values = find(value);
            Node previous = values.first;
            Node current = values.second;

            if (current != null) {
                return false;
            }

            int stamp = previous.next.getStamp();
            if (stamp == REMOVED) {
                removeElements();
            } else {
                if (previous.next.compareAndSet(null, node, stamp, stamp + 1)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        while (true) {
            Pair<Node, Node> values = find(value);
            Node previous = values.first;
            Node current = values.second;

            if (current != null) {
                Node node = current.next.getReference();
                int stamp = current.next.getStamp();
                if (stamp != REMOVED && current.next.compareAndSet(node, node, stamp, REMOVED)) {
                    removeFromSet(previous, current);
                    return true;
                }
            } else {
                return false;
            }
        }
    }

    @Override
    public boolean contains(@NotNull T value) {
        return find(value).second != null;
    }

    @Override
    public boolean isEmpty() {
        return !iterator().hasNext();
    }

    @Override
    public Iterator<T> iterator() {
        List<T> values = getAtomicSnapshot();
        Collections.sort(values);
        return values.iterator();
    }

    private boolean isPresent(@NotNull Node node) {
        return node.next.getStamp() != REMOVED;
    }

    private Pair<Node, Node> find(@NotNull T value) {
        Node previous = head.getReference();
        Node current = previous.next.getReference();
        while (current != null) {
            if (isPresent(current) && value.equals(current.value)) {
                return new Pair<>(previous, current);
            }
            previous = current;
            current = previous.next.getReference();
        }

        return new Pair<>(previous, null);
    }

    private void removeFromSet(Node previous, Node current) {
        Node next = current;
        while (next != null && !isPresent(next)) {
            next = next.next.getReference();
        }

        int stamp = previous.next.getStamp();
        if (stamp != REMOVED) {
            previous.next.compareAndSet(current, next, stamp, stamp + 1);
        }
    }

    private void removeElements() {
        Node previous = head.getReference();
        Node current = previous.next.getReference();
        while (current != null) {
            if (!isPresent(current)) {
                removeFromSet(previous, current);
            }
            previous = current;
            current = current.next.getReference();
        }
    }

    private List<T> getAtomicSnapshot() {
        Pair<Node, List<T>> snapshot1 = getSnapshot();
        Pair<Node, List<T>> snapshot2 = getSnapshot();

        while (!snapshot1.equals(snapshot2)) {
            snapshot1 = getSnapshot();
            snapshot2 = getSnapshot();
        }

        return snapshot1.second;
    }

    private Pair<Node, List<T>> getSnapshot() {
        List<T> values = new ArrayList<>();
        Node previous = head.getReference();
        Node current = head.getReference().next.getReference();

        while (current != null) {
            if (isPresent(current)) {
                values.add(current.value);
            }
            previous = current;
            current = current.next.getReference();
        }

        return new Pair<>(previous, values);
    }

    private class Node {
        private final T value;

        AtomicStampedReference<Node> next = new AtomicStampedReference<>(null, NEW);

        Node(T value) {
            this.value = value;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj.getClass() != getClass()) {
                return false;
            }

            Node other = (Node) obj;
            boolean valueEquals = (value == null && other.value == null) ||
                    (value != null && value.equals(other.value));
            return valueEquals && other.next.equals(next);
        }
    }

    private class Pair<K, V> {
        private final K first;
        private final V second;

        Pair(K first, V second) {
            this.first = first;
            this.second = second;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj.getClass() != getClass()) {
                return false;
            }

            Pair<K, V> other = (Pair<K, V>) obj;
            boolean firstEquals = (first == null && other.first == null) ||
                    (first != null && first.equals(other.first));
            boolean secondEquals = (second == null && other.second == null) ||
                    (second != null && second.equals(other.second));
            return firstEquals && secondEquals;
        }
    }
}
