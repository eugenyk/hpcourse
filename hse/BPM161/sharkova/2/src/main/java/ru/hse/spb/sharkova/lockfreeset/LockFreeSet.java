package ru.hse.spb.sharkova.lockfreeset;

import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private final AtomicMarkableReference<Node> head =
            new AtomicMarkableReference<>(new Node(null), false);

    @Override
    public boolean add(@NotNull T value) {
        Node node = new Node(value);

        while (true) {
            Pair<Node, Node> valuesInList = find(value);
            Node previous = valuesInList.first;
            Node current = valuesInList.second;

            if (current != null && current.value.compareTo(value) <= 0) {
                return false;
            }

            node.next.set(current, false);
            if (previous.next.compareAndSet(current, node, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        while (true) {
            Pair<Node, Node> valuesInList = find(value);
            Node previous = valuesInList.first;
            Node current = valuesInList.second;

            if (current != null) {
                Node next = current.next.getReference();
                if (current.next.compareAndSet(next, next, false, true)) {
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
        return getAtomicSnapshot().iterator();
    }

    private boolean isPresent(Node node) {
        return !node.next.isMarked();
    }

    @NotNull
    private Pair<Node, Node> find(T value) {
        Node lesser = head.getReference();
        Node greater = lesser.next.getReference();

        while (greater != null && (!isPresent(greater) || greater.value.compareTo(value) < 0)) {
            if (isPresent(greater)) {
                lesser = greater;
            }
            greater = greater.next.getReference();
        }

        return new Pair<>(lesser, greater);
    }

    private void removeFromSet(Node previous, Node current) {
        Node next = current.next.getReference();
        while (next != null && !isPresent(next)) {
            next = next.next.getReference();
        }

        previous.next.compareAndSet(current, next, false, false);
    }

    private List<T> getAtomicSnapshot() {
        List<T> snapshot1 = getSnapshot();
        List<T> snapshot2 = getSnapshot();

        while (!snapshot1.equals(snapshot2)) {
            snapshot1 = getSnapshot();
            snapshot2 = getSnapshot();
        }

        return snapshot1;
    }

    private List<T> getSnapshot() {
        List<T> values = new ArrayList<>();
        Node current = head.getReference().next.getReference();

        while (current != null) {
            if (isPresent(current)) {
                values.add(current.value);
            }
            current = current.next.getReference();
        }

        return values;
    }

    private class Node {
        private final T value;

        AtomicMarkableReference<Node> next = new AtomicMarkableReference<>(null, false);

        Node(T value) {
            this.value = value;
        }
    }

    private class Pair<K, V> {
        private final K first;
        private final V second;

        Pair(K first, V second) {
            this.first = first;
            this.second = second;
        }
    }
}
