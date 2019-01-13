package ru.spbau.mit.lara;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSortedList<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    AtomicInteger size = new AtomicInteger(0);
    private final Node<E> head;
    private final Node<E> tail;

    public LockFreeSortedList() {
        head = new Node<>(null, null, false);
        tail = new Node<>(null, null, false);
        head.set(tail, false);
    }

    @Override
    public boolean offer(E key) {
        final Node<E> new_node = new Node<>(key, null, false);
        while (true) {
            final Pair<E> p = find(key);
            final Node<E> prev_node = p.first;
            final Node<E> current_node = p.second;
            new_node.set(current_node, false);
            if (prev_node.compareAndSet(current_node, new_node, false, false)) {
                increase_size();
                break;
            }
        }
        return true;
    }

    @Override
    public E poll() {
        final E result;
        while (true) {
            final Node<E> current_node = getRealHead();
            if (current_node == tail) {
                result = null;
                break;
            }
            final Node<E> next_node = current_node.getReference();
            if (!current_node.compareAndSet(next_node, next_node, false, true)) {
                continue;
            }
            head.compareAndSet(current_node, next_node, false, false);
            result = current_node.value;
            decrease_size();
            break;
        }
        return result;
    }

    @Override
    public E peek() {
        return getRealHead().value;
    }

    @Override
    public int size() {
        return size.get();
    }

    @Override
    public boolean isEmpty() {
        return size() == 0;
    }

    @Override
    public Iterator<E> iterator() {
        return null; // no lock-free iterator
    }

    private Node<E> getRealHead() {
        final Node<E> prev_node = head;
        Node<E> current_node = null;
        Node<E> next_node = null;
        boolean markHolder[] = {false};

        while (true) {
            current_node = head.getReference();
            if (current_node == tail) {
                return tail;
            }
            next_node = current_node.get(markHolder);
            if (markHolder[0]) {
                prev_node.compareAndSet(current_node, next_node, false, false);
                continue;
            }
            return current_node;
        }
    }

    private Pair<E> find(E key) {
        Node<E> prev_node = null;
        Node<E> current_node = null;
        Node<E> next_node = null;

        while (true) {
            if (head.getReference() == tail) {
                return new Pair<>(head, tail);
            }
            prev_node = head;
            boolean markHolder[] = {false};

            while (true) {
                current_node = prev_node.getReference();
                next_node = current_node.get(markHolder);

                if (markHolder[0]) {
                    if (!prev_node.compareAndSet(current_node, next_node, false, false)) {
                        break; // continue outer while
                    }
                    continue;
                }

                if (current_node == tail || current_node.value.compareTo(key) >= 0) {
                    return new Pair<>(prev_node, current_node);
                }
                prev_node = current_node;
            }
        }
    }

    private void decrease_size() {
        boolean success;
        do {
            int tmp = size.get();
            success = size.compareAndSet(tmp, tmp - 1);
        } while (!success);
    }

    private void increase_size() {
        boolean success;
        do {
            int tmp = size.get();
            success = size.compareAndSet(tmp, tmp + 1);
        } while (!success);
    }

    private static class Node<E extends Comparable<E>> extends AtomicMarkableReference<Node<E>> {
        final E value;

        public Node(E value, Node<E> next, boolean mark) {
            super(next, mark);
            this.value = value;
        }
    }

    private static class Pair<E extends Comparable<E>> {
        final Node<E> first;
        final Node<E> second;

        Pair(final Node<E> first, final Node<E> second) {
            this.first = first;
            this.second = second;
        }
    }
}

