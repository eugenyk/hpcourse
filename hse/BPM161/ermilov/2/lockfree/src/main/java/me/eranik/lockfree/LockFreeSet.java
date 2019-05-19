package me.eranik.lockfree;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements Set<T> {

    private Node head;

    public LockFreeSet() {
        head = new Node(null, null);
    }

    private @NotNull ListPosition linearSearch(@NotNull T value, boolean marked) {
        Node prev = head;
        Node next = head.next.getReference();
        while (next != null) {
            if (next.next.isMarked() == marked && value.equals(next.data)) {
                break;
            }
            prev = next;
            next = next.next.getReference();
        }
        return new ListPosition(prev, next);
    }

    @Override
    public boolean add(@NotNull T value) {
        while (true) {
            ListPosition position = linearSearch(value, false);
            Node prev = position.prev;
            Node next = position.next;

            if (next != null) {
                return false;
            }
            Node newNode = new Node(value);
            newNode.next = new AtomicMarkableReference<>(next, false);
            if (prev.next.compareAndSet(null, newNode, false, false)) {
                return true;
            }
        }
    }

    private void removeMarked(@NotNull T value) {
        while (true) {
            ListPosition position = linearSearch(value, true);
            Node prev = position.prev;
            Node next = position.next;

            Node afterNext = next.next.getReference();
            if (prev.next.compareAndSet(next, afterNext, false, false)) {
                break;
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        while (true) {
            ListPosition position = linearSearch(value, false);
            Node next = position.next;

            if (next == null) {
                return false;
            }
            Node afterNext = next.next.getReference();
            if (next.next.compareAndSet(afterNext, afterNext, false, true)) {
                removeMarked(value);
                return true;
            }
        }
    }

    @Override
    public boolean contains(@NotNull T value) {
        ListPosition position = linearSearch(value, false);
        return position.next != null;
    }

    @Override
    public boolean isEmpty() {
        return createSnapshot().isEmpty();
    }

    private List<Node> createSnapshot() {
        ArrayList<Node> list = new ArrayList<>();
        for (Node node = head; node != null; node = node.next.getReference()) {
            if (node != head && !node.next.isMarked()) {
                list.add(node);
            }
        }
        return list;
    }

    @Override
    public Iterator<T> iterator() {
        while (true) {
            List<Node> snapshot1 = createSnapshot();
            List<Node> snapshot2 = createSnapshot();
            if (snapshot1.size() == snapshot2.size() && snapshot1.containsAll(snapshot2)) {
                return snapshot1.stream().map(node -> node.data).iterator();
            }
        }
    }

    private class Node {
        AtomicMarkableReference<Node> next;
        T data;

        Node(@Nullable T data) {
            this.next = new AtomicMarkableReference<>(null, false);
            this.data = data;
        }

        Node(@Nullable Node next, @Nullable T data) {
            this.next = new AtomicMarkableReference<>(next, false);
            this.data = data;
        }
    }

    private class ListPosition {
        Node prev;
        Node next;

        ListPosition(@NotNull Node prev, @Nullable Node next) {
            this.prev = prev;
            this.next = next;
        }
    }
}
