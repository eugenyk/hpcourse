package ru.eltech.diogenova.lockfree;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {
    final Node<E> head;

    public LockFreePriorityQueue() {
        head = new Node<>(null);
    }

    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean offer(E e) {
        checkData(e);

        Node<E> node = new Node<>(e);

        while(true) {
            Position<E> position = find(e);

            node.setNext(position.getSucc());

            if (position.getPrev().lazySetNext(position.getSucc(), node)) {
                return true;
            }
        }
    }

    @Override
    public E poll() {
        while (true) {
            Node<E> curr = head.getNext();

            if (curr == null)
                return null;

            if (curr.isMarked()) {
                head.lazySetNext(curr, curr.getNext());
                continue;
            }

            if (curr.setMarked(true)) {
                head.lazySetNext(curr, curr.getNext());
                return curr.getData();
            }
        }
    }

    @Override
    public E peek() {
        while (true) {
            Node<E> curr = head.getNext();

            if (curr == null)
                return null;

            if (curr.isMarked()) {
                head.lazySetNext(curr, curr.getNext());
                continue;
            }

            return curr.getData();
        }
    }

    @Override
    public boolean isEmpty() {
        while (true) {
            Node<E> curr = head.getNext();

            if (curr == null)
                return true;

            if (curr.isMarked()) {
                head.lazySetNext(curr, curr.getNext());
                continue;
            }

            return curr == null;
        }
    }

    protected void checkData(final E data) {
        if (data == null)
            throw new NullPointerException();
    }

    protected Position<E> find(final E data) {
        Node<E> prev = head;
        Node<E> succ = head.getNext();

        while(true) {
            if (succ == null || data.compareTo(succ.getData()) < 0) {
                return new Position<>(prev, succ);
            }

            if (succ.isMarked()) {
                prev.lazySetNext(succ, succ.getNext());
            }

            prev = succ;
            succ = succ.getNext();
        }
    }

    protected final class Position<E> {
        protected final Node<E> prev;
        protected final Node<E> succ;

        public Position(Node<E> prev, Node<E> succ) {
            this.prev = prev;
            this.succ = succ;
        }

        public Node<E> getPrev() {
            return prev;
        }

        public Node<E> getSucc() {
            return succ;
        }
    }

    protected final class Node<E> {
        protected final E data;
        protected final AtomicMarkableReference<Node<E>> next;

        public Node(final E data) {
            this.data = data;
            this.next = new AtomicMarkableReference<>(null, false);
        }

        public E getData() {
            return data;
        }

        public Node<E> getNext() {
            return next.getReference();
        }

        public void setNext(final Node<E> next) {
            this.next.set(next, false);
        }

        public boolean lazySetNext(final Node<E> expected, final Node<E> update) {
            return next.compareAndSet(expected, update, false, false);
        }

        public boolean isMarked() {
            return next.isMarked();
        }

        public boolean setMarked(boolean marked) {
            return next.compareAndSet(next.getReference(), next.getReference(), !marked, marked);
        }
    }
}
