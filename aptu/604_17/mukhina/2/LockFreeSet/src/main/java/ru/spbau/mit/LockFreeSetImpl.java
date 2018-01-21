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
        if (value == null) {
            return false;
        }

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
        if (value == null) {
            return false;
        }
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
        // searching null gives the last element in the set as previous node and tail as current one
        Neighbours<T> neighbours = searchNeighbours(null);
        // if the previous node is "head" then the set is empty
        return neighbours.previous == head && neighbours.current == tail;
    }

    private Neighbours<T> searchNeighbours(T keyToSearch) {
        while (true) {
            Node<T> previousNode = head;
            Node<T> previousNextNode = head.next.getReference();
            Node<T> currentNode;

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
                    continue;
                } else {
                    return new Neighbours<>(previousNode, currentNode);
                }
            }
        }
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
