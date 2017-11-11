package com.company;

import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;

public final class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final AtomicReference<Node> head = new AtomicReference<>(null);

    @Override
    public boolean add( T value) {
        return true;
    }

    @Override
    public boolean remove( T value) {
        return true;
    }

    @Override
    public boolean contains( T value) {
        return true;
    }

    @Override
    public boolean isEmpty() {
        return true;
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
