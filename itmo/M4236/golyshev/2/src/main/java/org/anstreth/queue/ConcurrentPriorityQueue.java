package org.anstreth.queue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class ConcurrentPriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {
    private final AtomicInteger size = new AtomicInteger(0);

    private final Node<E> head;
    private final Node<E> tail;

    public ConcurrentPriorityQueue() {
        tail = new Node<>(null, null);
        head = new Node<>(null, tail);
    }

    @Override
    public boolean isEmpty() {
        return size.get() == 0;
    }

    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException("You cannot iterate over concurrent queue");
    }

    @Override
    public int size() {
        return size.get();
    }

    @Override
    public boolean offer(E element) {
        while (true) {
            Position<E> position = findPositionForInsertion(element);
            Node<E> firstNode = position.getLeft();
            Node<E> secondNode = position.getRight();

            Node<E> newNode = new Node<>(element, secondNode);

            if (firstNode.replaceNext(secondNode, newNode)) {
                size.incrementAndGet();
                return true;
            }
        }
    }

    @Override
    public E poll() {
        while (true) {
            Node<E> firstElement = findPositionForInsertion(null).right;
            if (firstElement == tail) return null;
            Node<E> secondElement = firstElement.getNext();

            if (firstElement.markForDeletion(secondElement)) {
                size.decrementAndGet();
                return firstElement.getElement();
            }
        }
    }

    private Position<E> findPositionForInsertion(E e) {
        Node<E> leftNode = head;
        Node<E> leftNodeNext = head.getNext();
        Node<E> rightNode;

        do {
            Node<E> tmpNode = head;
            Node<E> tmpNodeNext = head.getNext();

            do {
                if (!tmpNode.isMarkedForDeletion()) {
                    leftNode = tmpNode;
                    leftNodeNext = tmpNodeNext;
                }

                tmpNode = tmpNode.getNext();
                if (tmpNode == tail) break;

                tmpNodeNext = tmpNode.getNext();
            } while (tmpNode.isMarkedForDeletion() || (e != null && e.compareTo(tmpNode.getElement()) <= 0));

            rightNode = tmpNode;

            if (leftNodeNext == rightNode) {
                if (rightNode != tail && rightNode.isMarkedForDeletion()) {
                    continue;
                } else {
                    return new Position<>(leftNode, rightNode);
                }
            }

            if (leftNode.replaceNext(leftNodeNext, rightNode)) {
                if (rightNode == tail || !rightNode.isMarkedForDeletion()) {
                    return new Position<>(leftNode, rightNode);
                }
            }

        } while (true);
    }

    @Override
    public E peek() {
        while (true) {
            Node<E> firstElement = head.getNext();
            if (firstElement == head) return null;

            if (!firstElement.isMarkedForDeletion()) {
                return firstElement.getElement();
            }
        }
    }

    private static class Node<T> {
        private final T element;
        /**
         * This reference is markable, but when it's marked it represents
         * that <b>this</b> Node is supposed to be deleted.
         */
        private final AtomicMarkableReference<Node<T>> nextNodeReference;

        Node(T element, Node<T> next) {
            this.element = element;
            this.nextNodeReference = new AtomicMarkableReference<>(next, false);
        }

        T getElement() {
            return element;
        }

        Node<T> getNext() {
            return nextNodeReference.getReference();
        }

        boolean replaceNext(Node<T> currentNext, Node<T> newNext) {
            // we want this node not to be deleted
            return nextNodeReference.compareAndSet(currentNext, newNext, false, false);
        }

        boolean isMarkedForDeletion() {
            return nextNodeReference.isMarked();
        }

        boolean markForDeletion(Node<T> nextReference) {
            // we are marking this node as deleted
            return nextNodeReference.compareAndSet(nextReference, nextReference, false, true);
        }
    }

    private static class Position<T> {
        private final Node<T> left;
        private final Node<T> right;

        Position(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }

        Node<T> getLeft() {
            return left;
        }

        Node<T> getRight() {
            return right;
        }
    }
}