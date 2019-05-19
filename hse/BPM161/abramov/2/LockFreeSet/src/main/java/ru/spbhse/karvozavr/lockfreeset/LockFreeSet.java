package ru.spbhse.karvozavr.lockfreeset;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * Lock-free set implementation using linked list.
 *
 * @param <T> Key type
 */
public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {

    private static final int REMOVED = -1;
    private static final int INITIAL = 0;

    /**
     * List node.
     */
    private class Node {

        final T value;

        AtomicStampedReference<Node> next = new AtomicStampedReference<>(null, INITIAL);

        Node(T value) {
            this.value = value;
        }
    }

    /**
     * Stamped reference to the head of the list.
     * Head is fictive Node with null value.
     */
    private final AtomicStampedReference<Node> head = new AtomicStampedReference<>(new Node(null), INITIAL);

    @Override
    public boolean add(@NotNull T value) {
        Node node = new Node(value);

        while (true) {
            Pair<Node> prevAndCurrent = find(value);
            Node prev = prevAndCurrent.first;
            Node current = prevAndCurrent.second;

            if (current != null) {
                return false;
            } else {
                var stamp = prev.next.getStamp();
                if (stamp != REMOVED) {
                    if (prev.next.compareAndSet(null, node, stamp, stamp + 1)) {
                        return true;
                    }
                } else {
                    fixRemove();
                }
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        while (true) {
            Pair<Node> prevAndCurrent = find(value);
            Node prev = prevAndCurrent.first;
            Node current = prevAndCurrent.second;

            if (current != null) {
                Node ref = current.next.getReference();
                var stamp = current.next.getStamp();
                if (stamp != REMOVED && current.next.compareAndSet(ref, ref, stamp, REMOVED)) {
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
        return !iterator().hasNext();
    }

    @Override
    public Iterator<T> iterator() {
        return getSnapshot().iterator();
    }

    private Pair<Node> find(@NotNull T value) {
        Node prev, node;
        for (prev = head.getReference(), node = prev.next.getReference();
             node != null; prev = node, node = node.next.getReference()) {
            if (!isRemoved(node) && value.equals(node.value)) {
                return new Pair<>(prev, node);
            }
        }

        return new Pair<>(prev, null);
    }

    private boolean isRemoved(@NotNull Node node) {
        return node.next.getStamp() == REMOVED;
    }

    private void fixRemove() {
        Node prev, node;
        for (prev = head.getReference(), node = prev.next.getReference();
             node != null; prev = node, node = node.next.getReference()) {
            if (isRemoved(node)) {
                physicallyRemove(prev, node);
            }
        }
    }

    private void physicallyRemove(@NotNull Node prev, @Nullable Node node) {
        Node next = node;
        while (next != null && isRemoved(next)) {
            next = next.next.getReference();
        }

        var stamp = prev.next.getStamp();
        if (stamp != REMOVED) {
            prev.next.compareAndSet(node, next, stamp, stamp + 1);
        }
    }

    private List<T> getSnapshot() {
        List<T> elements;
        List<Integer> versions;

        attempt:
        while (true) {
            elements = new ArrayList<>();
            versions = new ArrayList<>();

            Node node = head.getReference();
            versions.add(node.next.getStamp());
            for (node = node.next.getReference(); node != null; node = node.next.getReference()) {
                if (!isRemoved(node)) {
                    elements.add(node.value);
                    versions.add(node.next.getStamp());
                }
            }

            var elemIter = elements.iterator();
            var versionIter = versions.iterator();

            node = head.getReference();
            if (node.next.getStamp() != versionIter.next()) {
                continue attempt;
            }
            for (node = head.getReference().next.getReference(); node != null; node = node.next.getReference()) {
                if (!isRemoved(node)) {
                    if (!elemIter.hasNext() || elemIter.next() != node.value || versionIter.next() != node.next.getStamp()) {
                        continue attempt;
                    }
                }
            }

            break attempt;
        }

        return elements;
    }
}
