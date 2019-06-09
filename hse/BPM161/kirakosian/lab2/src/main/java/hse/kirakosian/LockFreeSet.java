package hse.kirakosian;

import org.jetbrains.annotations.NotNull;

import java.util.*;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements ILockFreeSet<T> {

    private final Node head = new Node(null, new AtomicMarkableReference<>(null, false));

    @Override
    public boolean isEmpty() {
        return getSnapshot().isEmpty();
    }

    @Override
    public boolean add(final T value) {
        // Always insert to the back
        final var newNode = new Node(value, new AtomicMarkableReference<>(null, false));
        while (true) {
            final var pair = find(value);
            final var prev = pair.getKey();
            final var target = pair.getValue();
            if (target != null) {
                return false;
            }
            if (prev.next.compareAndSet(null, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(final T value) {
        while (true) {
            final var pair = find(value);
            final var target = pair.getValue();
            if (target == null) {
                return false;
            }
            final var next = target.next.getReference();
            if (target.next.compareAndSet(next, next, false, true)) {
                return true;
            }
        }
    }

    @Override
    public boolean contains(final T value) {
        var curr = head.next.getReference();
        while (curr != null) {
            if (curr.value.equals(value) && !curr.next.isMarked()) {
                return true;
            }
            curr = curr.next.getReference();
        }
        return false;
    }

    @NotNull
    @Override
    public Iterator<T> iterator() {
        return getSnapshot().iterator();
    }

    /*
        Returns node with the target value and the previous one.
     */
    private Map.Entry<Node, Node> find(final T value) {
        while (true) {
            boolean restart = false;
            var curr = head;
            var next = head.next.getReference();
            while (next != null && !restart) {
                final var ref = next.next;
                if (ref.isMarked()) {
                    curr.next.compareAndSet(next, ref.getReference(), false, false);
                    restart = true;
                } else {
                    if (next.value.equals(value)) {
                        return new AbstractMap.SimpleEntry<>(curr, next);
                    }
                    curr = next;
                    next = ref.getReference();
                }
            }
            if (!restart) {
                return new AbstractMap.SimpleEntry<>(curr, null);
            }
        }
    }

    private List<T> getSnapshot() {
        while (true) {
            final var firstSnap = collect();
            final var secondSnap = collect();
            if (areSnapsEqual(firstSnap, secondSnap)) {
                return firstSnap;
            }
        }
    }

    private boolean areSnapsEqual(final List<T> first, final List<T> second) {
        if (first.size() != second.size()) {
            return false;
        }
        for (int i = 0; i < first.size(); i++) {
            if (!first.get(i).equals(second.get(i))) {
                return false;
            }
        }
        return true;
    }

    private List<T> collect() {
        final var list = new ArrayList<T>();
        var curr = head.next.getReference();
        while (curr != null) {
            final var value = curr.value;
            if (!curr.next.isMarked()) {
                list.add(value);
            }
            curr = curr.next.getReference();
        }
        return list;
    }

    private class Node {

        private final T value;
        private final AtomicMarkableReference<Node> next;

        Node(final T value, final AtomicMarkableReference<Node> next) {
            this.value = value;
            this.next = next;
        }

    }

}
