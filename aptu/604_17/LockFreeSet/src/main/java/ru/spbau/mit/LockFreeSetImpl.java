package ru.spbau.mit;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> head;
    private final Node<T> tail;

    public LockFreeSetImpl() {
        tail = new Node<>();
        head = new Node<>(tail);
    }

    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value);
        Node<T> previousNode;
        while (true) {
            Neighbours<T> neighbours = searchNeighbours(value);
            previousNode = neighbours.previous;
            Node<T> currentNode = neighbours.current;
            if (currentNode != tail && currentNode.key.compareTo(value) == 0) {
                return false;
            }
            newNode.next = new AtomicMarkableReference<>(currentNode, false);
            if (previousNode.next.compareAndSet(currentNode, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        Node<T> currentNode;
        Node<T> currentNodeNext;
        Node<T> previousNode;
        while (true) {
            Neighbours<T> neighbours = searchNeighbours(value);
            previousNode = neighbours.previous;
            currentNode = neighbours.current;

            if (currentNode == tail) {
                return false;
            }

            currentNodeNext = currentNode.next.getReference();
            if (currentNode.next.compareAndSet(currentNodeNext, currentNodeNext, false, true)) {
                break;
            }
        }

        // We will request searchNeighbours to change links if previousNode.next is marked.
        if (!previousNode.next.compareAndSet(currentNode, currentNodeNext, false, false)) {
            searchNeighbours(value);
        }
        return true;
    }

    @Override
    public boolean contains(T value) {
        Node<T> rightNode;
        rightNode = searchNeighbours(value).current;
        return rightNode != tail && rightNode.key.equals(value);
    }

    @Override
    public boolean isEmpty() {
        return searchTheLastElement() == head;
    }

    private Neighbours<T> searchNeighbours(T keyToSearch) {
        Node<T> previousNode = head;
        Node<T> previousNextNode = head.next.getReference();
        Node<T> currentNode;

        while (true) {
            Node<T> tmpNode = head;
            boolean[] isNextNodeMarked = {false};
            Node<T> tmpNodeNext = head.next.get(isNextNodeMarked);

            do {
                if (!isNextNodeMarked[0]) {
                    previousNode = tmpNode;
                    previousNextNode = tmpNodeNext;
                }
                tmpNode = tmpNodeNext;
                if (tmpNode == tail) {
                    break;
                }
                tmpNodeNext = tmpNode.next.get(isNextNodeMarked);
            } while (isNextNodeMarked[0] || tmpNode.key.compareTo(keyToSearch) < 0);
            currentNode = tmpNode;

            if (previousNode.next.compareAndSet(previousNextNode, currentNode, false, false)) {
                if (currentNode != tail && currentNode.next.isMarked()) {
                    return searchNeighbours(keyToSearch);
                } else {
                    return new Neighbours<>(previousNode, currentNode);
                }
            }
        }
    }

    private Node<T> searchTheLastElement() {
        boolean[] mark = {false};
        Node<T> tmpNode = head;
        while (tmpNode.next.getReference() != tail) {
            tmpNode = tmpNode.next.get(mark);
        }
        if (mark[0])
            return searchTheLastElement();
        else
            return tmpNode;
    }

    private static class Node<T> {
        private T key;
        private AtomicMarkableReference<Node<T>> next;

        Node() {
        }

        Node(Node<T> next) {
            this.next = new AtomicMarkableReference<>(next, false);
        }

        Node(T key) {
            this.key = key;
        }
    }

    private static class Neighbours<T> {
        Node<T> previous;
        Node<T> current;

        Neighbours(Node<T> previous, Node<T> current) {
            this.previous = previous;
            this.current = current;
        }
    }
}
