package ru.spbau.yaveyn.lockfree;

import java.util.Optional;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Function;
import java.util.function.Supplier;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final AtomicReference<Node<T>> head = new AtomicReference<>(null);

    private boolean checkedDeleteNext(Node<T> current) {
        Node<T> next = current == null ? head.get() : current.getNext();
        if (next == null || !next.isDeleted()) return false;
        Node<T> newNext = next.getNext();
        return current == null ? head.compareAndSet(next, newNext) : current.trySetNextAnyMark(next, newNext);
    }

    @Override
    public boolean isEmpty() {
        while(true) {
            Node current = head.get();
            if (current == null) return true;
            if (!current.isDeleted()) return false;
            checkedDeleteNext(null);
        }
    }

    private <K> Optional<K> loop(
            T value,
            Function<ThreeNodes<T>, Optional<K>> onBigger,
            Function<ThreeNodes<T>, Optional<K>> onEquals) {
        Node<T> prev = null;
        while (true) {
            checkedDeleteNext(prev);

            if (prev != null && prev.isDeleted()) return Optional.empty();
            Node<T> next = prev == null ? head.get() : prev.getNext();

            Node<T> nonDeletedNext = next;
            while (nonDeletedNext != null && nonDeletedNext.isDeleted()) {
                nonDeletedNext = nonDeletedNext.getNext();
            }

            int comparison = nonDeletedNext == null ? 1 : value.compareTo(nonDeletedNext.getValue());
            if (comparison > 0) {
                Optional<K> res = onBigger.apply(new ThreeNodes<>(prev, next, nonDeletedNext));
                if (res.isPresent()) return res;
            } else if (comparison < 0) {
                prev = nonDeletedNext;
            } else {
                Optional<K> res = onEquals.apply(new ThreeNodes<>(prev, next, nonDeletedNext));
                if (res.isPresent()) return res;
            }
        }
    }

    private Optional<Boolean> tryAdd(T value) {
        return loop(
                value,
                (thr) -> {
                    Node<T> newNode = new Node<>(value, thr.nonDeletedNext);
                    boolean casResult =
                            thr.prev ==
                                    null ?
                                    head.compareAndSet(thr.next, newNode) :
                                    thr.prev.trySetNext(thr.next, newNode, false);
                    if (casResult) return Optional.of(true);
                    else return Optional.empty();
                },
                (thr) -> Optional.of(false));
    }

    private Optional<Boolean> tryRemove(T value) {
        return loop(
                value,
                (thr) -> {
                    if (thr.prev == null || (!thr.prev.isDeleted() && thr.prev.getNext() == thr.next)) return Optional.of(false);
                    else return Optional.empty();
                },
                (thr) -> {
                    while (!thr.nonDeletedNext.isDeleted()) {
                        if (thr.nonDeletedNext.tryDelete(thr.nonDeletedNext.getNext())) {
                            checkedDeleteNext(thr.prev);
                            return Optional.of(true);
                        }
                    }
                    return Optional.empty();
                });
    }

    private static class ThreeNodes<T extends Comparable<T>> {
        final Node<T> prev;
        final Node<T> next;
        final Node<T> nonDeletedNext;

        ThreeNodes(Node<T> prev, Node<T> next, Node<T> nonDeletedNext) {
            this.prev = prev;
            this.next = next;
            this.nonDeletedNext = nonDeletedNext;
        }
    }

    private boolean tryTilSuccess(Supplier<Optional<Boolean>> work) {
        while (true) {
            Optional<Boolean> res = work.get();
            if (!res.isPresent()) continue;
            return res.get();
        }
    }

    @Override
    public boolean add(T value) {
        return tryTilSuccess(() -> tryAdd(value));
    }

    @Override
    public boolean remove(T value) {
        return tryTilSuccess(() -> tryRemove(value));
    }

    @Override
    public boolean contains(T value) {
        Node<T> current = head.get();
        while (current != null) {
            while (checkedDeleteNext(current)) { }
            int comparison = value.compareTo(current.getValue());
            if (!current.isDeleted() && comparison == 0) return true;
            if (comparison > 0) return false;
            current = current.getNext();
        }
        return false;
    }

    private static class Node<T extends Comparable<T>> {
        private final T value;
        private final AtomicMarkableReference<Node<T>> nextAndIsDeleted;

        Node(T value, Node<T> next) {
            this.value = value;
            this.nextAndIsDeleted = new AtomicMarkableReference<>(next, false);
        }

        boolean tryDelete(Node<T> expectedNext) {
            return nextAndIsDeleted.compareAndSet(expectedNext, expectedNext, false, true);
        }

        T getValue() {
            return value;
        }

        Node<T> getNext() {
            return nextAndIsDeleted.getReference();
        }

        boolean isDeleted() {
            return nextAndIsDeleted.isMarked();
        }

        boolean trySetNext(Node<T> expectedNext, Node<T> newNext, boolean expectedMark) {
            return nextAndIsDeleted.compareAndSet(expectedNext, newNext, expectedMark, expectedMark);
        }

        boolean trySetNextAnyMark(Node<T> expectedNext, Node<T> newNext) {
            return trySetNext(expectedNext, newNext, nextAndIsDeleted.isMarked());
        }
    }

}
