package ru.homework;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class LockFreeList<E extends Comparable<E>> implements LockFreeSet<E> {
    private final Node<E> tail = new Node<>(null);
    private Node<E> head = new Node<>(null, tail);

    public boolean add(E value) {
        Node<E> newNode = new Node<>(value);
        while (true) {
            Pair<E> nodes = find(value);
            Node<E> left = nodes.first;
            Node<E> right = nodes.second;
            if (right != tail && right.getKey().equals(value)) {
                return false;
            }
            newNode.setNext(right);
            if (left.getAtomicNext().compareAndSet(right, newNode, false, false)) {
                return true;
            }
        }
    }

    public boolean remove(E value) {
        Node<E> left;
        Node<E> right;
        Node<E> next;
        while (true) {
            Pair<E> nodes = find(value);
            left = nodes.first;
            right = nodes.second;
            if (right == tail || !right.getKey().equals(value)) return false;
            next = right.getNext();
            if (right.getAtomicNext().attemptMark(next, true)) break;
        }
        if (!left.getAtomicNext().compareAndSet(right, next, false, false)) {
            find(right.getKey());
        }
        return true;
    }

    public boolean contains(E value) {
        Node<E> cur = head.getNext();
        while (cur != tail && (cur.isMarked() || value.compareTo(cur.getKey()) > 0)) {
            cur = cur.getNext();
        }
        return cur != tail && value.equals(cur.getKey());
    }

    public boolean isEmpty() {
        while (head.getNext() != tail) {
            Node<E> next = head.getNext();
            if (!next.isMarked()) {
                return false;
            }
            head.getAtomicNext().compareAndSet(next, next.getNext(), false, false);
        }
        return true;
    }

    public Iterator<E> iterator() {
        return makeAtomicSnapshot();
    }

    private Pair<E> find(E value) {
        Node<E> left = head;
        Node<E> right;

        while (true) {
            Node<E> cur = head;
            Node<E> next = null;

            while (cur != tail && (cur == head || cur.isMarked() || cur.getKey().compareTo(value) < 0)) {
                if (!cur.isMarked()) {
                    left = cur;
                    next = cur.getNext();
                }
                cur = cur.getNext();
            }

            right = cur;

            if (next == right) {
                if (right != tail && right.isMarked()) continue;
                return new Pair<>(left, right);
            }

            if (left.getAtomicNext().compareAndSet(next, right, false, false)) {
                if (right != tail && right.isMarked()) continue;
                return new Pair<>(left, right);
            }
        }
    }

    private Iterator<E> makeAtomicSnapshot() {
        List<E> firstSnapshot = getSnapshot();
        List<E> secondSnapshot = getSnapshot();

        while (!firstSnapshot.equals(secondSnapshot)) {
            firstSnapshot = getSnapshot();
            secondSnapshot = getSnapshot();
        }

        return firstSnapshot.iterator();
    }

    private List<E> getSnapshot() {
        List<E> list = new ArrayList<>();
        Node<E> current = head.getNext();

        while (current != tail) {
            if (!current.isMarked()) {
                list.add(current.getKey());
            }
            current = current.getNext();
        }
        return list;
    }

    private static class Pair<E extends Comparable<E>> {
        private Node<E> first;
        private Node<E> second;

        Pair(Node<E> first, Node<E> second) {
            this.first = first;
            this.second = second;
        }
    }
}
