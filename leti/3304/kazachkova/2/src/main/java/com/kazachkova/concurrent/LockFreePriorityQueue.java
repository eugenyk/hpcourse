package com.kazachkova.concurrent;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E>  {
    protected final Node<E> head;
    protected final Node<E> tail;
    protected final AtomicInteger version;

    public LockFreePriorityQueue() {
        this.head = new Node<>(null);
        this.tail = new Node<>(null);

        this.head.setNext(tail);
        this.tail.setNext(tail);

        this.version = new AtomicInteger(0);
    }

    @Override
    public Iterator<E> iterator() {
        return new PriorityQueueIterator();
    }

    @Override
    public int size() {
        for(;;) {
            int size = 0;


            for (Node<E> current = head; current != tail; ++size) {
                current = current.getNext();
                if (size == Integer.MAX_VALUE)
                    return size;
                if (current.isDeleted())
                    --size;
            }

            return size;
        }
    }

    @Override
    public boolean offer(E e) {
        if (e == null)
            throw new NullPointerException();

        Node<E> node = new Node<>(e);

        for(;;) {
            Node<E> current = head;
            Node<E> next = head.getNext();

            while (next != tail && e.compareTo(next.getItem()) >= 0) {
                if (next.isDeleted()) {
                    current.casNext(next, next.getNext());
                }
                current = next;
                next = next.getNext();
            }

            node.setNext(next);

            if (current.casNext(next, node))
                return true;
        }
    }

    @Override
    public E poll() {
        for(;;) {
            Node<E> top = head.getNext();

            if (top.isDeleted()) {
                head.casNext(top, top.getNext());
                continue;
            }

            if (top != tail && !top.setDeleted(true)) {
                continue;
            }

            head.casNext(top, top.getNext());

            return top.getItem();
        }
    }

    @Override
    public E peek() {
        for(;;) {
            Node<E> top = head.getNext();

            if (top.isDeleted()) {
                head.casNext(top, top.getNext());
                continue;
            }

            return top.getItem();
        }
    }

    @Override
    public boolean isEmpty() {
        for(;;) {
            Node<E> top = head.getNext();

            if (top.isDeleted()) {
                head.casNext(top, top.getNext());
                continue;
            }

            return top == tail;
        }
    }

    protected final class PriorityQueueIterator implements Iterator<E> {
        protected Node<E> currentNode = head;

        @Override
        public boolean hasNext() {
            return currentNode.getNext() != tail;
        }

        @Override
        public E next() {
            currentNode = currentNode.getNext();
            return currentNode.getItem();
        }
    }

    protected final static class Node<E> {
        protected final E item;
        protected final AtomicMarkableReference<Node<E>> nextRef;

        public Node(E item) {
            this.item = item;
            this.nextRef = new AtomicMarkableReference<>(null, false);
        }

        public E getItem() {
            return item;
        }

        public Node<E> getNext() {
            return nextRef.getReference();
        }

        public void setNext(Node<E> next) {
            nextRef.set(next, false);
        }

        public boolean casNext(Node<E> expected, Node<E> update) {
            return nextRef.compareAndSet(expected, update, false, false);
        }

        public boolean isDeleted() {
            return nextRef.isMarked();
        }

        public boolean setDeleted(boolean deleted) {
            Node<E> next = getNext();
            return nextRef.compareAndSet(next, next, !deleted, deleted);
        }
    }
}
