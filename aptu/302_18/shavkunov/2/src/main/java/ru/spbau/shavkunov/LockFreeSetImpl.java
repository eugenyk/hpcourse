package ru.spbau.shavkunov;

import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Based on https://www.cl.cam.ac.uk/research/srg/netos/papers/2001-caslists.pdf
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private Node<T> head;
    private Node<T> tail;

    public LockFreeSetImpl() {
        head = new Node<T>(null);
        tail = new Node<T>(null);

        head.next.set(tail, false);
    }

    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value);

        do {
            Pair<Node<T>> pair = search(value);
            Node<T> leftNode = pair.getFirst();
            Node<T> rightNode = pair.getSecond();

            if ((rightNode != tail) && rightNode.getKey().equals(value)) {
                return false;
            }

            newNode.next.set(rightNode, false);
            if (leftNode.next.compareAndSet(rightNode, newNode, false, false)) {
                return true;
            }
        } while (true);
    }

    @Override
    public boolean remove(T value) {
        Node<T> rightNodeNext;
        Node<T> leftNode;
        Node<T> rightNode;

        while (true) {
            Pair<Node<T>> pair = search(value);

            leftNode = pair.getFirst();
            rightNode = pair.getSecond();

            if (rightNode == tail || !rightNode.key.equals(value)) {
                return false;
            }

            rightNodeNext = rightNode.next.getReference();
            if (rightNode.next.compareAndSet(rightNodeNext, rightNodeNext, false, true)) {
                break;
            }
        }

        if (!leftNode.next.compareAndSet(rightNode, rightNodeNext, false, false)) {
            search(rightNode.key);
        }

        return true;
    }

    @Override
    public boolean contains(T value) {
        Pair<Node<T>> pair = search(value);
        Node<T> rightNode = pair.getSecond();

        return rightNode != tail && rightNode.key.equals(value);
    }

    @Override
    public boolean isEmpty() {
        Pair<Node<T>> pair = search(null);

        return pair.getFirst() == head && pair.getSecond() == tail;
    }

    private class Node<T extends Comparable<T>> {
        private final T key;
        AtomicMarkableReference<Node<T>> next;

        Node(T key) {
            this.key = key;
            next = new AtomicMarkableReference<>(null, false);
        }

        public T getKey() {
            return key;
        }
    }

    private Pair<Node<T>> search(T searchKey) {
        Node<T> leftNode = null;
        Node<T> leftNodeNext = null;
        Node<T> rightNode = null;

        do {
            boolean[] curMark = {false};
            Node<T> cur = head;
            Node<T> curNext = head.next.getReference();

            do {
                if (!cur.next.isMarked()) {
                    leftNode = cur;
                    leftNodeNext = curNext;
                }

                cur = cur.next.getReference();

                if (cur == tail) {
                    break;
                }

                curNext = cur.next.get(curMark);
            } while (curMark[0] ||
                    (searchKey != null && (cur.getKey().compareTo(searchKey) < 0)));

            rightNode = cur;

            if (leftNode.next.compareAndSet(leftNodeNext, rightNode, false, false)) {
                if (rightNode == tail || !rightNode.next.isMarked()) {
                    return new Pair<>(leftNode, rightNode);
                }
            }
        } while (true);
    }

    private class Pair<U> {
        private final U first;
        private final U second;

        public Pair(U first, U second) {
            this.first = first;
            this.second = second;
        }

        public U getFirst() {
            return first;
        }

        public U getSecond() {
            return second;
        }
    }
}