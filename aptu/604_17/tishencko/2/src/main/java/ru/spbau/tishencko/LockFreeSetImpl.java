package ru.spbau.tishencko;

import java.util.concurrent.atomic.AtomicMarkableReference;

/* Based on https://timharris.uk/papers/2001-disc.pdf */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T>{
    private final Entry<T> head;
    private final Entry<T> tail;

    public LockFreeSetImpl() {
        head = new Entry<T>(null);
        tail = new Entry<T>(null);
        head.next.set(tail, false);
    }

    public boolean add(T value) {
        Entry<T> newNode = new Entry<T>(value);
        while (true) {
            Window<T> place = searchWindow(value);
            Entry<T> rightNode = place.right;
            Entry<T> leftNode = place.left;
            if ((rightNode != tail) && (rightNode.key == value))
                return false;
            newNode.next.set(rightNode, false);
            if (leftNode.next.compareAndSet(rightNode, newNode, false, false))
                return true;
        }
    }

    public boolean remove(T value) {
        Entry<T> leftNode, rightNode, rightNodeNext;
        while (true) {
            Window<T> place = searchWindow(value);
            rightNode = place.right;
            leftNode = place.left;
            if (rightNode == tail || !rightNode.key.equals(value))
                return false;
            rightNodeNext = rightNode.next.getReference();
            if (rightNode.next.compareAndSet(rightNodeNext, rightNodeNext, false, true))
                break;
        }
        if (!leftNode.next.compareAndSet(rightNode, rightNodeNext, false, false)) {
            searchWindow(rightNode.key);
        }
        return true;

    }

    public boolean contains(T value) {
        Window<T> place = searchWindow(value);
        Entry<T> rightNode = place.right;
        return rightNode != tail && rightNode.key.equals(value);
    }

    public boolean isEmpty() {
        Window<T> place = searchWindow(null);
        return place.left == head && place.right == tail;
    }

    private Window<T> searchWindow(T value) {
        Entry<T> leftNode = null;
        Entry<T> rightNode;
        Entry<T> leftNodeNext = null;
        while (true) {
            boolean[] mark = {false};
            Entry<T> curr = head;
            Entry<T> currNext = head.next.getReference();
            do {
                if (!curr.next.isMarked()) {
                    leftNode = curr;
                    leftNodeNext = currNext;
                }
                curr = curr.next.getReference();
                if (curr == tail)
                    break;
                currNext = curr.next.get(mark);
            } while (mark[0] || (value != null && curr.key.compareTo(value) < 0));
            rightNode = curr;

            if (leftNode.next.compareAndSet(leftNodeNext, rightNode, false, false)) {
                if (rightNode == tail || !rightNode.next.isMarked()) {
                    return new Window<T>(rightNode, leftNode);
                }
            }
        }

    }

    private static class Entry<U extends Comparable<U>> {
        final U key;
        final AtomicMarkableReference<Entry<U>> next;

        Entry(U key) {
            this.key = key;
            next = new AtomicMarkableReference<Entry<U>>(null, false);
        }
    }

    private static class Window<U extends Comparable<U>> {
        final Entry<U> right;
        final Entry<U> left;

        Window(Entry<U> right, Entry<U> left) {
            this.right = right;
            this.left = left;
        }
    }
}
