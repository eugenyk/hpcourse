package com.lockfree;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    public final Node<T> head;
    public final Node<T> tail;

    LockFreeSetImpl() {
        head = new Node<T>(null);
        tail = new Node<T>(null);
    }

    class Node<T> {
        final T value;

        final AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);

        Node(T value) {
            this.value = value;
        }
    }

    class Found {
        Node<T> previous;
        Node<T> current;
        Node<T> next;

        Found(Node<T> previous, Node<T> current , Node<T> next) {
            this.setPrevious(previous);
            this.setCurrent(current);
            this.setNext(next);
        }

        public void setPrevious(Node<T> previous) {
            this.previous = previous;
        }

        public void setCurrent(Node<T> current) {
            this.current = current;
        }

        public void setNext(Node<T> next) {
            this.next = next;
        }

        public Node<T> getPrevious() {
            return this.previous;
        }

        public Node<T> getCurrent() {
            return this.current;
        }

        public Node<T> getNext() {
            return this.next;
        }
    }

    private Found find(T wanted) {
        Node<T> previous;
        Node<T> current;
        Node<T> next;
        boolean[] marked ={false};

        if (this.isEmpty()) {
            return new Found(this.head, null, null);
        }

        retry:
        while (true) {
            previous = this.head;
            current = previous.next.getReference();

            while (true) {
                if (current == null) {
                    return new Found(previous, null, null);
                }
                next = current.next.get(marked);
                while (marked[0]) {
                    if (!previous.next.compareAndSet(current, next, false, false)) {
                        continue retry;
                    }

                    current = next;
                    next = current.next.get(marked);
                }
                if (current == null || wanted.compareTo(current.value) < 0) {
                    return new Found(previous, null, current);
                } else {
                    if (wanted.compareTo(current.value) == 0) {
                        return new Found(previous, current, next);
                    }
                }
                previous = current;
                current = next;
            }
        }
    }

    @Override
    public boolean add(T value) {
        Node<T> node = new Node<T>(value);
        while (true) {

            Found found = find(value);

            if (found.getCurrent() != null) {
                return false;
            }

            node.next.set(found.getNext(), false);

            if (found.getPrevious().next.compareAndSet(found.getNext(), node, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Found found = find(value);

            if (found.getCurrent() == null) {
                return false;
            }

            if (found.getPrevious().next.compareAndSet(found.getCurrent(), found.getNext(), false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        Found found = find(value);
        return (found.getCurrent() != null);
    }

    @Override
    public boolean isEmpty() {
        AtomicMarkableReference<Node<T>> current = this.head.next;

        if (current.getReference() == null) {
            return true;
        } else {
            while (current.getReference() != null) {
                if (!current.isMarked()) {
                    return false;
                }

                current = current.getReference().next;
            }
            return true;
        }
    }
}