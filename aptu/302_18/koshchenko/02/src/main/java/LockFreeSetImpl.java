import javafx.util.Pair;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> tail = new Node<>(null, null);
    private final Node<T> head = new Node<>(null, tail);

    @Override
    public boolean add(T value) {
        while (true) {
            Pair<Node<T>, Node<T>> pair = find(value);
            Node<T> right = pair.getValue();
            Node<T> left = pair.getKey();
            if (right != tail && right.value.compareTo(value) == 0) return false;
            Node<T> newNode = new Node<>(value, right);
            if (left.next.compareAndSet(right, newNode, false, false)) return true;
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Node<T> right = find(value).getValue();
            if (right == tail || compareNodeToValue(right, value) != 0) return false;
            Node<T> rightNext = right.getNext();
            if (right.next.compareAndSet(rightNext, rightNext, false, true)) break;
        }
        return true;
    }

    @Override
    public boolean contains(T value) {
        Node<T> right = find(value).getValue();
        return right != tail && compareNodeToValue(right, value) == 0;
    }

    @Override
    public boolean isEmpty() {
        Node<T> node = head.getNext();
        while (node != tail) {
            if (node.notMarked()) return false;
            node = node.getNext();
        }
        return true;
    }

    private Pair<Node<T>, Node<T>> find(T value) {
        while (true) {
            Node<T> right = head.getNext();
            Node<T> left = head;
            Node<T> next = right;
            while (right != tail) {
                Node<T> newNext = right.getNext();
                if (compareNodeToValue(right, value) >= 0 && right.notMarked()) break;
                if (right.notMarked()) {
                    left = right;
                    next = newNext;
                }
                right = newNext;
            }
            if (next == right) return new Pair<>(left, right);
            left.next.compareAndSet(next, right, false, false);
        }
    }

    private int compareNodeToValue(Node<T> node, T value) {
        return node.value.compareTo(value);
    }

    private static class Node<T> {

        private AtomicMarkableReference<Node<T>> next;
        private T value;

        private Node(T value, Node<T> node) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(node, false);
        }

        Node<T> getNext() {
            return next.getReference();
        }

        boolean notMarked() {
            return !next.isMarked();
        }
    }
}