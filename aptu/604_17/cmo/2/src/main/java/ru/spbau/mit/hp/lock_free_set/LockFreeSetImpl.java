package ru.spbau.mit.hp.lock_free_set;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    @NotNull
    private final Node<T> head = new Node<>();
    @NotNull
    private final Node<T> tail = new Node<>();

    {
        head.setNext(tail);
    }

    @Override
    public boolean add(@NotNull final T key) {
        final Node<T> newNode = new Node<>(key);

        while (true) {
            final SearchResult<T> result = search(key);
            final Node<T> leftNode = result.leftNode;
            final Node<T> rightNode = result.rightNode;
            if (rightNode != tail && rightNode.key != null && rightNode.key.compareTo(key) == 0) {
                return false;
            }

            newNode.setNext(rightNode);
            if (leftNode.next.compareAndSet(rightNode, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(@NotNull final T key) {
        Node<T> leftNode, rightNode, rightNodeNext;

        while (true) {
            final SearchResult<T> result = search(key);
            leftNode = result.leftNode;
            rightNode = result.rightNode;
            if (rightNode == tail || rightNode.key != null && rightNode.key.compareTo(key) != 0) {
                return false;
            }

            rightNodeNext = rightNode.getNext();
            if (!rightNode.isMarked() && rightNode.next.attemptMark(rightNodeNext, true)) {
                break;
            }
        }

        if (!leftNode.next.compareAndSet(rightNode, rightNodeNext, false, false)) {
            search(rightNode.key);
        }

        return true;
    }

    @Override
    public boolean contains(@NotNull final T key) {
        final SearchResult<T> result = search(key);
        final Node<T> leftNode = result.leftNode;
        final Node<T> rightNode = result.rightNode;
        return leftNode != tail && rightNode.key != null && rightNode.key.compareTo(key) == 0;
    }

    @Override
    public boolean isEmpty() {
        for (Node<T> next = head.getNext(); next != tail; next = head.getNext()) {
            if (!next.isMarked()) {
                return false;
            }

            head.next.compareAndSet(next, next.getNext(), false, false);
        }

        return true;
    }

    @NotNull
    private SearchResult<T> search(final T key) {
        Node<T> leftNode = head, leftNodeNext = null, rightNode;

        while (true) {
            Node<T> node = head;

            /* 1: Find leftNode and rightNode */
            do {
                if (!node.isMarked()) {
                    leftNode = node;
                    leftNodeNext = leftNode.getNext();
                }

                node = node.getNext();
                if (node == tail) {
                    break;
                }
            } while (node.isMarked() || node.key != null && node.key.compareTo(key) < 0);
            rightNode = node;

            /* 2: Check nodes are adjacent */
            if (leftNodeNext == rightNode) {
                if (rightNode != tail && rightNode.isMarked()) {
                    continue;
                } else {
                    return new SearchResult<>(leftNode, rightNode);
                }
            }

            /* 3: Remove one or more marked nodes */
            final boolean isMarked = leftNode.isMarked();
            if (leftNode.next.compareAndSet(leftNodeNext, rightNode, isMarked, isMarked)) {
                if (rightNode == tail || !rightNode.isMarked()) {
                    return new SearchResult<>(leftNode, rightNode);
                }
            }
        }
    }

    private static class Node<T extends Comparable<T>> {
        @Nullable
        private final T key;
        @NotNull
        private final AtomicMarkableReference<Node<T>> next =
                new AtomicMarkableReference<>(null, false);

        private Node() {
            this.key = null;
        }

        Node(@NotNull final T key) {
            this.key = key;
        }

        @NotNull
        Node<T> getNext() {
            return next.getReference();
        }

        void setNext(@NotNull final Node<T> node) {
            next.set(node, false);
        }

        boolean isMarked() {
            return next.isMarked();
        }
    }

    private static class SearchResult<T extends Comparable<T>> {
        @NotNull
        final Node<T> leftNode;
        @NotNull
        final Node<T> rightNode;

        private SearchResult(@NotNull final Node<T> leftNode, @NotNull final Node<T> rightNode) {
            this.leftNode = leftNode;
            this.rightNode = rightNode;
        }
    }
}
