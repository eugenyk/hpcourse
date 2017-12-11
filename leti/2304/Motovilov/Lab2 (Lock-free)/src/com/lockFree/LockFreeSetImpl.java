package com.lockFree;

import java.util.Objects;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;
import java.util.ArrayList;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final AtomicReference<Node> head = new AtomicReference<>(null);

    @Override
    public boolean add(T value) {
        while(true) {
            NodePair<Node> nodePair = this.searchPair(value);
            Node previous = nodePair.previous;
            Node current = nodePair.current;

            //если уже есть во множестве
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
    public boolean remove(T value) {
        Node previous;
        Node current;
        Node renewed;
        do {
            NodePair<Node> nodePair = this.searchPair(value);
            previous = nodePair.previous;
            current = nodePair.current;

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
    public boolean contains(T value) {
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

    @Override
    public String toString() {
        ArrayList<T> result = new ArrayList<>();
        Node cur = this.head.get();
        while (cur != null) {
            if (contains(cur.getValue())) {
                result.add(cur.getValue());
            }
            cur = cur.getNext().getReference();
        }
        return result.toString();
    }

    private NodePair<Node> searchPair(T value) {
        while(true) {
            Node previous = head.get();
            Node next;

            if (previous != null)
                next = previous.getNext().getReference();
            else
                return new NodePair<>(null, null);

            if (previous.getNext().isMarked()) {
                head.compareAndSet(previous, previous.getNext().getReference());
                continue;
            }

            if (previous.getValue().compareTo(value) >= 0)
                return new NodePair<>(null, previous);

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
                        return new NodePair<>(previous, current);
                    }

                    previous = current;
                    current = currentNext;
                }
            }

            if (isTryAgain)
                continue;

            return new NodePair<>(previous, null);
        }
    }

    private final class Node {
        private final T value;

        private final AtomicMarkableReference<Node> next;

        final T getValue() {
            return this.value;
        }

        final AtomicMarkableReference<Node> getNext() {
            return this.next;
        }

        Node(final T value, final AtomicMarkableReference<Node> next) {
            this.value = value;
            this.next = next;
        }
    }

    private final class NodePair<T> {
        final T previous;
        final T current;

        NodePair(final T first, final T second) {
            this.previous = first;
            this.current = second;
        }
    }
}
