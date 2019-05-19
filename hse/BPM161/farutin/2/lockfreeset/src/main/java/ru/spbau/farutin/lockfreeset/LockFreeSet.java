package ru.spbau.farutin.lockfreeset;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private AtomicMarkableReference<Node> head =
            new AtomicMarkableReference<>(new Node(null), false);

    @Override
    public boolean add(@NotNull T value) {
        Node newNode = new Node(value);

        while (true) {
            NodePair valuePosition = find(value);
            Node prev = valuePosition.first;
            Node current = valuePosition.second;

            if (current != null) {
                return false;
            }

            if (prev.next.compareAndSet(null, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        while (true) {
            NodePair valuePosition = find(value);
            Node prev = valuePosition.first;
            Node current = valuePosition.second;

            if (current == null) {
                return false;
            }

            Node next = current.next.getReference();

            if (current.next.compareAndSet(next, next, false, true)) {
                prev.next.compareAndSet(current, next, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(@NotNull T value) {
        NodePair valuePosition = find(value);
        Node current = valuePosition.second;

        return current != null;
    }

    @Override
    public boolean isEmpty() {
        return !iterator().hasNext();
    }

    @Override
    public @NotNull Iterator<T> iterator() {
        return getSnapshot().iterator();
    }

    private NodePair find(T value) {
        Node prev;
        Node current;

        for (prev = head.getReference(), current = prev.next.getReference();
             current != null;
             prev = current, current = current.next.getReference()) {
            if (!isPresent(current)) {
                boolean mark = prev.next.isMarked();
                prev.next.compareAndSet(current, current.next.getReference(), mark, mark);
            }

            if (isPresent(current) && value.equals(current.value)) {
                return new NodePair(prev, current);
            }
        }

        return new NodePair(prev, null);
    }

    private boolean isPresent(Node node) {
        return !node.next.isMarked();
    }

    private List<T> getSnapshot() {
        List<T> values;
        List<T> newValues;

        do {
            values = getCurrentValues();
            newValues = getCurrentValues();
        } while (!values.equals(newValues));

        return values;
    }

    private List<T> getCurrentValues() {
        List<T> values = new ArrayList<>();

        for (Node node = head.getReference().next.getReference();
             node != null;
             node = node.next.getReference()) {
            if (isPresent(node)) {
                values.add(node.value);
            }
        }

        return values;
    }

    private class Node {
        private T value;
        private AtomicMarkableReference<Node> next =
                new AtomicMarkableReference<>(null, false);

        Node(@Nullable T value) {
            this.value = value;
        }
    }

    private class NodePair {
        private Node first;
        private Node second;

        NodePair(@Nullable Node first, @Nullable Node second) {
            this.first = first;
            this.second = second;
        }
    }
}
