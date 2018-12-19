package org.anstreth.queue;

import java.util.AbstractQueue;
import java.util.Iterator;

public class ConcurrentPriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private final Node<E> head;
    private final Node<E> tail;

    public ConcurrentPriorityQueue() {
        tail = new Node<>(null, null);
        head = new Node<>(null, tail);
    }

    @Override
    public boolean isEmpty() {
        return head == tail;
    }

    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException("You cannot iterate over concurrent queue");
    }

    @Override
    public int size() {
        int size = 0;
        Node<E> current = head;
        while (current.next != tail) {
            size++;
            current = current.next;
        }

        return size;
    }

    @Override
    synchronized public boolean offer(E e) {
        Node<E> current = head;

        while (current.next != tail && current.next.element.compareTo(e) >= 0) {
            current = current.next;
        }

        current.next = new Node<>(e, current.next);
        return true;
    }

    @Override
    synchronized public E poll() {
        if (head.next == tail) return null;

        E elem = head.next.element;
        head.next = head.next.next;

        return elem;
    }

    @Override
    synchronized public E peek() {
        if (isEmpty()) return null;

        return head.next.element;
    }

    private static class Node<T> {
        final T element;
        Node<T> next;

        private Node(T element, Node<T> next) {
            this.element = element;
            this.next = next;
        }

        public T getElement() {
            return element;
        }

        public Node<T> getNext() {
            return next;
        }
    }
}
