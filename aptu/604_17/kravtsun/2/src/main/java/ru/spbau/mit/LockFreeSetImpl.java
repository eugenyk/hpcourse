package ru.spbau.mit;

import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T>, Iterable<T> {
    private final Node<T> tail = Node.tailNode();
    private final Node<T> head = Node.headNode(tail);

    private static <T extends Comparable<T>> NodeWindow<T> find(T value, Node<T> startNode) {
        retry: while (true) {
            NodeWindow<T> nodeWindow = new NodeWindow<>(startNode, startNode.getNext().getReference());

            while (true) {
                Node<T> nextNode = nodeWindow.getCurrent().getNext().getReference();
                boolean currentMarked = nodeWindow.getCurrent().getNext().isMarked();
                if (currentMarked) { // logical removal.
                    AtomicMarkableReference<Node<T>> previousNext = nodeWindow.getPrevious().getNext();
                    if (!previousNext.compareAndSet(nodeWindow.getCurrent(), nextNode, false, false)) {
                        // first case: previousNode was removed before CAS, should review list from start.
                        // second case: somebody already removed currentNode.
                        continue retry;
                    }
                    // we succeeded in removing currentNode.
                    nodeWindow.setCurrent(nextNode);
                } else {
                    if (nodeWindow.getCurrent().compareToValue(value) >= 0) {
                        return nodeWindow;
                    }
                    nodeWindow.setPrevious(nodeWindow.getCurrent());
                    nodeWindow.setCurrent(nextNode);
                }
            }
        }
    }

    @Override
    public boolean add(T value) {
        while (true) {
            NodeWindow<T> nodeWindow = find(value);
            Node<T> previousNode = nodeWindow.getPrevious();
            Node<T> currentNode = nodeWindow.getCurrent();
            if (currentNode.getValue() != null && currentNode.getValue().equals(value)) {
                return false;
            }
            Node<T> newNode = new Node<>(value, currentNode);
            if (previousNode.getNext().compareAndSet(currentNode, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            NodeWindow<T> nodeWindow = find(value);
            Node<T> previousNode = nodeWindow.getPrevious();
            Node<T> currentNode = nodeWindow.getCurrent();
            if (currentNode.compareToValue(value) != 0) {
                return false;
            }
            Node<T> nextNode = currentNode.getNext().getReference();
            if (currentNode.getNext().compareAndSet(nextNode, nextNode, false, true)) {
                // optimization, unnecessary.
                previousNode.getNext().compareAndSet(currentNode, nextNode, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        NodeWindow<T> nodeWindow = find(value);
        Node<T> currentNode = nodeWindow.getCurrent();
        return currentNode.compareToValue(value) == 0;
    }

    @Override
    public boolean isEmpty() {
        return head.getNext().getReference() == tail;
    }

    private NodeWindow<T> find(T value) {
        return find(value, head);
    }

    @Override
    public Iterator<T> iterator() {
        // not thread-safe iterator.
        return new Iterator<T>() {
            private final NodeWindow<T> nodeWindow = new NodeWindow<>(head, head.getNext().getReference());

            @Override
            public boolean hasNext() {
                return !nodeWindow.getCurrent().isTail();
            }

            @Override
            public T next() {
                T value = nodeWindow.getCurrent().getValue();
                nodeWindow.setPrevious(nodeWindow.getCurrent());
                Node<T> nextNode = nodeWindow.getCurrent().getNext().getReference();
                nodeWindow.setCurrent(nextNode);
                return value;
            }
        };
    }
}
