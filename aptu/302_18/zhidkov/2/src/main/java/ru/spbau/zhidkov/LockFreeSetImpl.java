package ru.spbau.zhidkov;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> tail = new Node<>(null, new AtomicMarkableReference<>(null, false));
    private final Node<T> head = new Node<>(null, new AtomicMarkableReference<>(tail, false));

    @Override
    public boolean add(T value) {
        while (true) {
            PairOfNodes<T> pair = find(value);
            if (pair.right != tail && pair.right.value.compareTo(value) == 0) {
                return false;
            }
            Node<T> newNode = new Node<>(value, new AtomicMarkableReference<>(pair.right, false));
            if (pair.left.next.compareAndSet(pair.right, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            PairOfNodes<T> pair = find(value);
            if (pair.right == tail || pair.right.value.compareTo(value) != 0 || pair.right.next.isMarked()) {
                return false;
            }
            Node<T> rightNext = pair.right.next.getReference();
            if (pair.right.next.compareAndSet(rightNext, rightNext, false, true)) {
                break;
            }
        }
        return true;
    }

    @Override
    public boolean contains(T value) {
        PairOfNodes<T> pair = find(value);
        return pair.right != tail && pair.right.value.compareTo(value) == 0;
    }

    @Override
    public boolean isEmpty() {
        Node<T> cur = head.next.getReference();
        while (cur != tail) {
            if (!cur.next.isMarked()) {
                return false;
            }
            cur = cur.next.getReference();
        }
        return true;
    }

    private PairOfNodes<T> find(T value) {
        while (true) {
            Node<T> right = head.next.getReference();
            Node<T> left = head;
            Node<T> leftNext = right;
            while (right != tail) {
                Node<T> newNext = right.next.getReference();
                if (right.value.compareTo(value) >= 0 && !right.next.isMarked()) {
                    break;
                }
                if (!right.next.isMarked()) {
                    left = right;
                    leftNext = newNext;
                }
                right = newNext;
            }
            if (leftNext == right) {
                return new PairOfNodes<>(left, right);
            }
            left.next.compareAndSet(leftNext, right, false, false);
        }
    }

    private static class Node<T> {

        private T value;
        private AtomicMarkableReference<Node<T>> next;

        private Node(T value, AtomicMarkableReference<Node<T>> next) {
            this.value = value;
            this.next = next;
        }
    }

    private static class PairOfNodes<T> {

        private Node<T> left;
        private Node<T> right;

        private PairOfNodes(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }

    }
}
