package ru.ifmo.priorityqueue;

import java.util.*;

public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

    private List<E> elements = new ArrayList<>();
    private Integer size = 0;

    public LockFreePriorityQueue() {
        elements.add(null);
    }

    public LockFreePriorityQueue(Collection<E> collection) {
        this();
        elements.addAll(collection);
        size = collection.size();
        for (int i = size / 2; i >= 1; i--) {
            heapify(i, elements.get(i));
        }
    }

    @Override
    public Iterator<E> iterator() {
        return new Iterator<E>() {
            int cur = 1;

            @Override
            public boolean hasNext() {
                return cur <= size;
            }

            @Override
            public E next() {
                return elements.get(cur++);
            }

            @Override
            public void remove() {
                LockFreePriorityQueue.this.removeAt(cur);
            }
        };
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
        increaseKey(size, e);
        return true;
    }

    /**
     * Retrieves and removes the head of this queue,
     * or returns null if this queue is empty.
     */
    @Override
    public E poll() {
        if (!isEmpty()) {
            E first = elements.get(1);
            E last = elements.get(size);
            elements.set(1, last);
            elements.remove((int)size);
            size--;
            if (size > 1) {
                heapify(1, last);
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
        return (size == 0) ? null : elements.get(1);
    }

    @Override
    public boolean isEmpty() {
        return size == 0;
    }

    @Override
    public boolean add(E e) {
        return offer(e);
    }

    @Override
    public boolean remove(Object o) {
        if (!isEmpty() && o != null) {
            int i = indexOf((E) o);
            if (i != -1) {
                removeAt(i);
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean contains(Object o) {
        return elements.contains(o);
    }

    private void increaseKey(int k, E e) {
        while (k > 1 && elements.get(k / 2).compareTo(e) > 0) {
            elements.set(k, elements.get(k / 2));
            k = k / 2;
        }
        elements.set(k, e);
    }

    private void heapify(int root, E e) {
        int child;
        int k = root;
        while (2 * k + 1 <= size) {
            child = 2 * k;
            if (child < size &&
                    elements.get(child).compareTo(elements.get(child + 1)) > 0) {
                child++;
            }
            if (e.compareTo(elements.get(child)) <= 0) {
                break;
            } else {
                elements.set(k, elements.get(child));
                k = child;
            }
        }
        elements.set(k, e);
    }

    private int indexOf(E e) {
        for (int i = 1; i <= size; i++) {
            if (elements.get(i).equals(e)) {
                return i;
            }
        }
        return -1;
    }

    private E removeAt(int i) {
        E e = null;
        if (i == size) {
            e = elements.remove(i);
        } else {
            E moved = elements.get(size);
            heapify(i, moved);
            elements.remove((int) size);
            if (!elements.get(i).equals(moved)) {
                e = moved;
            }
        }
        size--;
        return e;
    }
}
