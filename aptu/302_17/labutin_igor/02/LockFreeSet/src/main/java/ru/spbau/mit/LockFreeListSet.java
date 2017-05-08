package ru.spbau.mit;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeListSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private Node<T> head = new Node<T>(null);
    private Node<T> tail = new Node<T>(null);

    public LockFreeListSet() {
        head.next.set(tail, false);
    }

    public boolean add(T value) {
        Node<T> newNode = new Node<T>(value);

        while (true) {
            PairNode<T> pairNode = search(value);

            if (pairNode.right != tail && pairNode.right.value.equals(value)) {
                return false;
            }

            newNode.next.set(pairNode.right, false);

            if (pairNode.left.next.compareAndSet(pairNode.right, newNode, false, false)) {
                return true;
            }
        }
    }

    public boolean remove(T value) {
        PairNode<T> pairNode;
        Node<T> rightNodeNext;
        while (true) {
            pairNode = search(value);
            if (pairNode.right == tail || !pairNode.right.value.equals(value)) {
                return false;
            }
            boolean markHolder[] = new boolean[1];
            rightNodeNext = pairNode.right.next.get(markHolder);
            if (!markHolder[0]) {
                if (pairNode.right.next.compareAndSet(rightNodeNext, rightNodeNext, false, true)) {
                    break;
                }
            }
        }

        boolean leftMark = pairNode.left.next.isMarked();
        if (!pairNode.left.next.compareAndSet(pairNode.right, rightNodeNext, leftMark, leftMark)) {
            search(pairNode.right.value);
        }
        return true;
    }

    public boolean contains(T value) {
        PairNode<T> pairNode = search(value);
        return pairNode.right != tail && pairNode.right.value.equals(value);
    }

    public boolean isEmpty() {
        for (Node<T> node = head.next.getReference(); node != tail; node = node.next.getReference()) {
            if (!node.next.isMarked()) {
                return false;
            }
        }
        return true;
    }

    private PairNode<T> search(T value) {
        Node<T> leftNodeNext = null;
        Node<T> leftNode = null;

        while (true) {
            Node<T> node = head;
            boolean markHolder[] = new boolean[1];
            Node<T> nodeNext = head.next.get(markHolder);
            do {
                if (!markHolder[0]) {
                    leftNode = node;
                    leftNodeNext = nodeNext;
                }
                node = nodeNext;
                if (node == tail) {
                    break;
                }
                nodeNext = node.next.get(markHolder);
            } while (markHolder[0] || node.value.compareTo(value) < 0);
            Node<T> rightNode = node;

            if (leftNodeNext == rightNode) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                } else {
                    return new PairNode<T>(leftNode, rightNode);
                }
            }

            if (leftNode != null && !leftNode.next.compareAndSet(leftNodeNext, rightNode, false, false)) {
                return new PairNode<T>(leftNode, rightNode);
            }
        }
    }

    private static class Node<T extends Comparable<T>> {
        private T value;
        private AtomicMarkableReference<Node<T>> next;

        Node(T value) {
            this.value = value;
            this.next = new AtomicMarkableReference<Node<T>>(null, false);
        }
    }

    private static class PairNode<T extends Comparable<T>> {
        private Node<T> left;
        private Node<T> right;

        PairNode(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }
    }
}
