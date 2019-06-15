package hse.kirilenko.lockfreeset;

import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Lock-free set implementation using linked list.
 *
 * @param <T> Key type
 */
public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private final AtomicMarkableReference<Node> head = new AtomicMarkableReference<>(new Node(), false);

    @Override
    public boolean add(@NotNull T value) {
        while (true) {
            PrevCurNodePair prevAndCurrent = findPrevCur(value);
            final Node prev = prevAndCurrent.prev;
            final Node current = prevAndCurrent.cur;

            if (current != null) {
                return false;
            }

            Node node = new Node(value);
            if (prev.next.compareAndSet(null, node, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        while (true) {
            PrevCurNodePair prevAndCurrent = findPrevCur(value);
            final Node prev = prevAndCurrent.prev;
            final Node current = prevAndCurrent.cur;

            if (current == null) {
                return false;
            }

            final Node ref = current.next.getReference();
            if (current.next.compareAndSet(ref, ref, false, true)) {
                Node next = ref;
                while (next != null && next.next.isMarked()) {
                    next = next.next.getReference();
                }

                prev.next.compareAndSet(current, next, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(@NotNull T value) {
        return findPrevCur(value).cur != null;
    }

    @Override
    public boolean isEmpty() {
        return !iterator().hasNext();
    }

    @Override
    public Iterator<T> iterator() {
        return getSnapshot().iterator();
    }

    private List<T> getSnapshot() {
        while (true) {
            SnapshotItem unsafeSnapshot1 = unsafeSnapshot();
            SnapshotItem unsafeSnapshot2 = unsafeSnapshot();

            // if lists are equal, but state is incorrect, last nodes will be different
            if (unsafeSnapshot1.items.equals(unsafeSnapshot2.items) &&
                    unsafeSnapshot1.lastNode == unsafeSnapshot2.lastNode) {
                return unsafeSnapshot1.items;
            }
        }
    }

    private SnapshotItem unsafeSnapshot() {
        List<T> result = new ArrayList<>();
        Node node;

        for (node = head.getReference().next.getReference(); node != null; node = node.next.getReference()) {
            if (!node.next.isMarked()) {
                result.add(node.value);
            }
        }

        return new SnapshotItem(result, node);
    }

    private PrevCurNodePair findPrevCur(@NotNull T value) {
        Node prev, node;
        for (prev = head.getReference(), node = prev.next.getReference(); node != null; prev = node, node = node.next.getReference()) {
            if (node.next.isMarked()) {
                boolean mark = prev.next.isMarked();
                prev.next.compareAndSet(node, node.next.getReference(), mark, mark);
            }

            if (!node.next.isMarked() && value.equals(node.value)) {
                return new PrevCurNodePair(prev, node);
            }
        }

        return new PrevCurNodePair(prev, null);
    }

    private class Node {
        AtomicMarkableReference<Node> next = new AtomicMarkableReference<>(null, false);
        final T value;

        Node(T value) {
            this.value = value;
        }

        Node() {
            this.value = null;
        }
    }

    private class PrevCurNodePair {
        final Node prev;
        final Node cur;

        PrevCurNodePair(Node first, Node second) {
            this.prev = first;
            this.cur = second;
        }
    }

    private class SnapshotItem {
        final List<T> items;
        final Node lastNode;

        SnapshotItem(List<T> items, Node lastNode) {
            this.items = items;
            this.lastNode = lastNode;
        }
    }
}
