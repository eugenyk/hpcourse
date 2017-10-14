package ru.spbau.mit;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> tail = Node.tailNode();
    private final Node<T> head = Node.headNode(tail);

    @Override
    public boolean add(T value) {
        while (true) {
            NodeWindow nodeWindow = find(value);
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
            NodeWindow nodeWindow = find(value);
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
        NodeWindow nodeWindow = find(value);
        Node<T> currentNode = nodeWindow.getCurrent();
        return currentNode.compareToValue(value) == 0;
    }

    @Override
    public boolean isEmpty() {
        return head.getNext().getReference() == tail;
    }

    private NodeWindow find(T value) {
        retry: while (true) {
            Node<T> previousNode = head;
            Node<T> currentNode = previousNode.getNext().getReference();
            while (true) {
                Node<T> nextNode = currentNode.getNext().getReference();
                boolean currentMarked = currentNode.getNext().isMarked();
                if (currentMarked) { // logical removal.
                    if (!previousNode.getNext().compareAndSet(currentNode, nextNode, false, false)) {
                        // first case: previousNode was removed before CAS, should review list from start.
                        // second case: somebody already removed currentNode.
                        continue retry;
                    }
                    // we succeded in removing currentNode.
                    currentNode = nextNode;
                } else {
                    if (currentNode.compareToValue(value) >= 0) {
                        return new NodeWindow(previousNode, currentNode);
                    }
                    previousNode = currentNode;
                    currentNode = nextNode;
                }
            }
        }
    }

    enum NodeType {
        Usual,
        Head,
        Tail
    }

    private class NodeWindow {
        private final Node<T> previous;
        private final Node<T> current;

        private NodeWindow(Node<T> previous, Node<T> current) {
            this.previous = previous;
            this.current = current;
        }

        Node<T> getPrevious() {
            return previous;
        }

        Node<T> getCurrent() {
            return current;
        }
    }

}
