package ru.ifmo.priorityqueue;

import java.util.*;

public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

    private List<E> elements = new ArrayList<>();
    private Integer size = 0;

    public LockFreePriorityQueue() {
    }

    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }

    @Override
    public int size() {
        return size;
    }

    /**
     * Inserts the specified element into this priority queue.
     */
    @Override
    public boolean offer(E e) {
        elements.add(e);
        size++;
        increaseKey(size - 1, e);
        return true;
    }

    /**
     * Retrieves and removes the head of this queue,
     * or returns null if this queue is empty.
     */
    @Override
    public E poll() {
        if (!isEmpty()) {
            E first = elements.get(0);
            E last = elements.get(size - 1);
            elements.set(0, last);
            elements.remove(size - 1);
            size--;
            if (size > 0) {
                heapify(0);
            }
            return first;
        }
        return null;
    }

    /**
     * Retrieves, but does not remove, the head of this queue,
     * or returns null if this queue is empty.
     */
    @Override
    public E peek() {
        return (size == 0) ? null : elements.get(0);
    }

    private void increaseKey(int k, E e) {
        while (k > 0 && elements.get(k / 2).compareTo(e) > 0) {
            elements.set(k, elements.get(k / 2));
            k /= 2;
        }
        elements.set(k, e);
    }

    private void heapify(int root) {
        E last = elements.get(root);
        int child;
        int k = root;
        while (2 * (k + 1) < size) {
            child = 2 * (k + 1) - 1;
            if (child < size &&
                    elements.get(child).compareTo(elements.get(child + 1)) > 0) {
                child++;
            }
            if (last.compareTo(elements.get(child)) <= 0) {
                break;
            } else {
                elements.set(k, elements.get(child));
                k = child;
            }
        }
        elements.set(k, last);
    }
}
