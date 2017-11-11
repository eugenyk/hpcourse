package com.company;

import org.jetbrains.annotations.NotNull;

import java.util.Objects;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;

public final class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final AtomicReference<Node> head = new AtomicReference<>(null);

    @Override
    public boolean add(@NotNull T value) {
        while(true) {
            Pair<Node, Node> nodePair = this.searchPair(value);
            Node previous = nodePair.first;
            Node current = nodePair.second;
            if (current != null && Objects.equals(current.getValue(), value))
                return false;

            Node node = new Node(value, new AtomicMarkableReference<>(current, false));
            if (Objects.equals(previous, null)) {
                if (this.head.compareAndSet(current, node))
                    return true;
            } else if (previous.getNext().compareAndSet(current, node, false, false))
                return true;
        }
    }

    @Override
    public boolean remove(@NotNull T value) {
        Node previous;
        Node current;
        Node renewed;
        do {
            Pair<Node, Node> nodePair = this.searchPair(value);
            previous = nodePair.first;
            current = nodePair.second;
            if (current == null || !Objects.equals(current.getValue(), value))
                return false;

            renewed = current.getNext().getReference();
        } while (!current.getNext().compareAndSet(renewed, renewed, false, true));

        if (Objects.equals(previous, null))
            this.head.compareAndSet(current, renewed);
        else
            previous.getNext().compareAndSet(current, renewed, false, false);

        return true;
    }

    @Override
    public boolean contains(@NotNull T value) {
//        Pair<Node, Node> foundNodes = this.searchPair(value);
//        Node curr = foundNodes.second;
        Node curr = this.head.get();
        while(curr != null && curr.getValue().compareTo(value) < 0){
            curr = curr.getNext().getReference();
        }
        return curr != null && Objects.equals(curr.getValue(), value) && !curr.getNext().isMarked();
    }

    @Override
    public boolean isEmpty() {
        return this.head.get() == null;
    }

    @NotNull
    private Pair<Node, Node> searchPair(T value) {
        while(true) {
            Node previous = head.get();
            Node next;

            if (previous != null)
                next = previous.getNext().getReference();
            else
                return new Pair<>(null, null);

            if (previous.getNext().isMarked()) {
                head.compareAndSet(previous, previous.getNext().getReference());
                continue;
            }

            if (previous.getValue().compareTo(value) >= 0)
                return new Pair<>(null, previous);

            Node current = next;
            boolean isTryAgain = false;
            while (current != null) {
                Node currentNext = current.getNext().getReference();
                if (current.getNext().isMarked()) {
                    boolean isCompareAndSetPassed = previous.getNext().compareAndSet(current, currentNext, false, false);
                    if (!isCompareAndSetPassed) {
                        isTryAgain = true;
                        break;
                    }

                    current = currentNext;
                } else {
                    if (current.getValue().compareTo(value) >= 0) {
                        return new Pair<>(previous, current);
                    }

                    previous = current;
                    current = currentNext;
                }
            }

            if (isTryAgain)
                continue;

            return new Pair<>(previous, null);
        }
    }

    private final class Node {
        @NotNull
        private final T value;
        @NotNull
        private final AtomicMarkableReference<Node> next;

        @NotNull
        final T getValue() {
            return this.value;
        }

        @NotNull
        final AtomicMarkableReference<Node> getNext() {
            return this.next;
        }

        Node(@NotNull final T value, @NotNull final AtomicMarkableReference<Node> next) {
            this.value = value;
            this.next = next;
        }
    }

    private final class Pair<A, B> {
        final A first;
        final B second;

        Pair(final A first, final B second) {
            this.first = first;
            this.second = second;
        }
    }

}