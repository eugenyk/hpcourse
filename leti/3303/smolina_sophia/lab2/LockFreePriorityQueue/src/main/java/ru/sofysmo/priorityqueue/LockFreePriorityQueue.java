package ru.sofysmo.priorityqueue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

    private final AtomicInteger size = new AtomicInteger(0);
    private final Node<E> tail = new Node<>(null, null);
    private final Node<E> head = new Node<>(null, tail);
    private static final int maxAttempt = 100;

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        return size.get();
    }

    @Override
    public boolean offer(E e) {
        int i = 0;
        while (i < maxAttempt) {
            Position<E> pair = findPosition(e);
            Node<E> previous = pair.getLeft();
            Node<E> next = pair.getRight();
            Node<E> newNode = new Node<>(e, next);
            if(previous.compareAndSetNext(next, newNode)) {
                size.incrementAndGet();
                return true;
            }
            i++;
        }
        return false;
    }
    @Override
    public E poll() {
        int i = 0;
        while (i < maxAttempt) {
            Node<E> position = getFirst();
            if(position == tail) {
                return null;
            }
            Node<E> nextMin = position.getNext();
            if(position.markDeleted(nextMin, nextMin)) {
                head.compareAndSetNext(position, nextMin);
                size.decrementAndGet();
                return position.getValue();
            }
            i++;
        }
        return null;
    }

    @Override
    public E peek() {
        return getFirst().getValue();
    }

    @Override
    public boolean isEmpty() {
        return size.get() == 0;
    }

    private Position<E> findPosition(E e) {
        while(true) {
            Node<E> left = head;
            Node<E> right;
            Node<E> current = head.getNext();
            boolean[] flagDeletion = {false};

            Node<E> tmpNode = head;
            Node<E> tmpNodeNext = head.getNext(flagDeletion);
            do {
                if (!flagDeletion[0]) {
                    left = tmpNode;
                    current = tmpNodeNext;
                }
                tmpNode = tmpNodeNext;
                tmpNodeNext = tmpNode.getNext(flagDeletion);
            } while (tmpNode != tail && (flagDeletion[0] || tmpNode.getValue().compareTo(e) <= 0));

            right = tmpNode;
            if(left.compareAndSetNext(current, right)) {
                if(right == tail || !right.nextIsMarked()) {
                    return new Position<>(left, right);
                }
            }
        }
    }

    private Node<E> getFirst() {
        Node<E> left = head;
        boolean[] flagDeletion = {false};
        while (true) {
            Node<E> current = left.getNext();
            Node<E> next = current.getNext(flagDeletion);
            if(flagDeletion[0]) {
                left.compareAndSetNext(current, next);
                continue;
            }
            return current;
        }
    }

    private static class Node<E extends Comparable<E>> {
        private final E value;
        private AtomicMarkableReference<Node<E>> next;

        Node(E value, Node<E> next) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        E getValue() {
            return value;
        }

        boolean markDeleted(Node<E> nextOld, Node<E> nextNew) {
            return next.compareAndSet(nextOld, nextNew, false, true);
        }

        boolean compareAndSetNext(Node<E> nextOld, Node<E> nextNew) {
            return next.compareAndSet(nextOld, nextNew, false, false);
        }

        Node<E> getNext() {
            return next.getReference();
        }

        Node<E> getNext(boolean flagDeletion[]) {
            return next.get(flagDeletion);
        }

        boolean nextIsMarked() {
            return next.isMarked();
        }
    }

    private static class Position<E extends Comparable<E>> {
        private final Node<E> left;
        private final Node<E> right;

        Position(Node<E> leftNode, Node<E> rightNode) {
            left = leftNode;
            right = rightNode;
        }

        Node<E> getLeft() {
            return left;
        }

        Node<E> getRight() {
            return right;
        }
    }
}