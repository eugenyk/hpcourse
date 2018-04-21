package ru.spbau.hpc.lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.function.Predicate;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node head;

    LockFreeSetImpl() {
        head = new Node();
    }

    @Override
    public boolean add(T value) {
        Node newNode = new Node(value);
        while (true) {
            Node pos = null;
            while (pos == null) {
                pos = findByPredicate(node -> {
                    Node nextNode = node.getNext();
                    return nextNode == null || value.compareTo(nextNode.getValue()) < 0;
                });
            }
            if (value.equals(pos.getValue())) {
                return false;
            }
            Node posNext = pos.getNext();
            if (posNext != null && value.compareTo(posNext.getValue()) >= 0) {
                continue;
            }
            boolean failure = false;
            while (!failure) {
                newNode.setNext(newNode.getNext(), posNext);
                if (pos.setNext(posNext, newNode)) {
                    return true;
                } else {
                    if (pos.isDeleted()) {
                        failure = true;
                    } else {
                        posNext = pos.getNext();
                        if (posNext != null && value.compareTo(posNext.getValue()) >= 0) {
                            failure = true;
                        }
                    }
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        Node nodeToRemove = findByValue(value);
        if (nodeToRemove == null) {
            return false;
        }
        nodeToRemove.markDeleted();
        return true;
    }

    @Override
    public boolean contains(T value) {
        return findByValue(value) != null;
    }

    @Override
    public boolean isEmpty() {
        Node afterHead = head.getNext();
        while (afterHead != null) {
            if (afterHead.isDeleted()) {
                head.setNext(afterHead, afterHead.getNext());
                afterHead = head.getNext();
            } else {
                return false;
            }
        }
        return true;
    }

    private Node findByPredicate(Predicate<Node> predicate) {
        Node pos = head;
        while (true) {
            if (pos == null) {
                return null;
            }
            Node nextPos = pos.getNext();
            while (!pos.isDeleted() && nextPos != null && nextPos.isDeleted()) {
                pos.setNext(nextPos, nextPos.getNext());
                nextPos = pos.getNext();
            }
            if (predicate.test(pos)) {
                return pos;
            }

            if (pos.isDeleted()) {
                pos = head;
            } else {
                pos = pos.getNext();
            }
        }
    }

    private Node findByValue(T value) {
        return findByPredicate(n -> value.equals(n.getValue()));
    }

    private class Node {
        private AtomicMarkableReference<Node> next;
        private T value;

        Node(T value) {
            this.value = value;
            next = new AtomicMarkableReference<>(null, false);
        }

        Node() {
            this(null);
        }

        private void markDeleted() {
            boolean done = false;
            while (!done) {
                done = next.attemptMark(next.getReference(), true);
            }
        }

        private boolean isDeleted() {
            return next.isMarked();
        }

        private Node getNext() {
            return next.getReference();
        }

        private boolean setNext(Node oldNext, Node newNext) {
            return next.compareAndSet(oldNext, newNext, false, false);
        }

        private T getValue() {
            return value;
        }
    }
}
