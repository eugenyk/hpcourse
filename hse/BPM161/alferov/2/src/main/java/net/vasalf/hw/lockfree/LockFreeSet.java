package net.vasalf.hw.lockfree;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements MySet<T> {
    private static class Node<T extends Comparable<T>> {
        private T value;
        private AtomicMarkableReference<Node<T>> next;

        Node(T value, Node<T> next) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        public T getValue() {
            return value;
        }

        public AtomicMarkableReference<Node<T>> getNext() {
            return next;
        }
    }

    private Node<T> head;
    private Node<T> tail;

    public LockFreeSet() {
        tail = new Node<>(null, null);
        head = new Node<>(null, tail);
    }

    @Override
    public boolean add(T value) {
        while (true) {
            Node<T> prev = null;
            boolean prevMarked = false;
            Node<T> cur = head;
            boolean[] curMarked = new boolean[1];
            Node<T> curNext = head.getNext().get(curMarked);
            while (cur != tail) {
                if (cur != head && cur.getValue().compareTo(value) == 0 && !curMarked[0]) {
                    return false;
                }
                prev = cur;
                prevMarked = curMarked[0];
                cur = curNext;
                curNext = cur.getNext().get(curMarked);
            }
            Node<T> newNode = new Node<>(value, tail);
            if (prev.getNext().compareAndSet(tail, newNode, prevMarked, prevMarked)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            boolean rep = false;
            Node<T> cur = head;
            boolean[] curMarked = new boolean[1];
            Node<T> curNext = head.getNext().get(curMarked);
            while (cur != tail) {
                if (cur != head && cur.getValue().compareTo(value) == 0 && !curMarked[0]) {
                    if (cur.getNext().compareAndSet(curNext, curNext, false, true)) {
                        return true;
                    } else {
                        rep = true;
                        break;
                    }
                }
                cur = curNext;
                curNext = cur.getNext().get(curMarked);
            }
            if (!rep) {
                return false;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        Node<T> cur = head;
        boolean[] curMarked = new boolean[1];
        Node<T> curNext = head.getNext().get(curMarked);
        while (cur != tail) {
            if (cur != head && cur.getValue().compareTo(value) == 0 && !curMarked[0]) {
                return true;
            }
            cur = curNext;
            curNext = cur.getNext().get(curMarked);
        }
        return false;
    }

    @Override
    public boolean isEmpty() {
        Node<T> cur = head;
        boolean[] curMarked = new boolean[1];
        Node<T> curNext = head.getNext().get(curMarked);
        while (cur != tail) {
            if (cur != head && !curMarked[0]) {
                return false;
            }
            cur = curNext;
            curNext = cur.getNext().get(curMarked);
        }
        return true;
    }

    @Override
    public Iterator<T> iterator() {
        List<T> prev = null;
        List<T> cur = new ArrayList<>();

        do {
            prev = new ArrayList<>(cur);
            cur.clear();
            Node<T> node = head;
            boolean[] marked = new boolean[1];
            while (node != tail) {
                Node<T> next = node.getNext().get(marked);
                if (node != head && !marked[0]) {
                    cur.add(node.value);
                }
                node = next;
            }
        } while (!prev.equals(cur));

        return cur.iterator();
    }
}
