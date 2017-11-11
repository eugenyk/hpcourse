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

        private final T value;

        private final AtomicMarkableReference<Node> nextIsRemoved;


        final T getValue() {
            return this.value;
        }

        final AtomicMarkableReference<Node> getNextIsRemoved() {
            return this.nextIsRemoved;
        }

        Node(final T value, final AtomicMarkableReference<Node> nextIsRemoved) {
            this.value = value;
            this.nextIsRemoved = nextIsRemoved;
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
