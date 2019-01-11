package com.stupid58fly.concurrent;

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

    /**
     * Returns the number of elements in this queue. If this queue contains more than Integer.MAX_VALUE elements, returns Integer.MAX_VALUE.
     * Beware that, unlike in most collections, this method is NOT a constant-time operation. Because of the asynchronous nature of these queues, determining the current number of elements requires an O(n) traversal. Additionally, if elements are added or removed during execution of this method, the returned result may be inaccurate. Thus, this method is typically not very useful in concurrent applications.
     * @return the number of elements in this queue
     */
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
                Node<E> afterNext = next.getNext();
                current = next;
                next = afterNext;
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
            Node<E> afterTop = top.getNext();

            if (top != tail && !top.setDeleted(true)) {
                continue;
            }

            if (top.getNext() != afterTop) {
                top.setDeleted(false);
                continue;
            }

            if (!head.casNext(top, afterTop)) {
                top.setDeleted(false);
                continue;
            }

            return top.getItem();
        }
    }

    @Override
    public E peek() {
        return head.getNext().getItem();
    }

    @Override
    public boolean isEmpty() {
        return head.getNext() == tail;
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
