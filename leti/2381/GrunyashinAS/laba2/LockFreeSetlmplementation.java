package com.lockfreeset;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImplementation<T extends Comparable<T>> implements LockFreeSet<T> {

    public final Node<T> head;

    LockFreeSetImplementation() {
        head = new Node<T>(null);
    }

    private Found find(T wanted) {
        Node<T> previousNode;
        Node<T> currentNode;
        Node<T> nextNode;
        boolean[] marked ={false};

        if (this.isEmpty()) {
            return new Found(this.head, null, null);
        }

        retry:
        while (true) {
            previousNode = this.head;
            currentNode = previousNode.get_next().getReference();

            while (true) {
                if (currentNode == null) {
                    return new Found(previousNode, null, null);
                }
                nextNode = currentNode.get_next().get(marked);
                while (marked[0]) {
                    if (!previousNode.get_next().compareAndSet(currentNode, nextNode, false, false)) {
                        continue retry;
                    }

                    currentNode = nextNode;
                    nextNode = currentNode.get_next().get(marked);
                }
                if (currentNode == null || wanted.compareTo(currentNode.get_value()) < 0) {
                    return new Found(previousNode, null, currentNode);
                } else {
                    if (wanted.compareTo(currentNode.get_value()) == 0) {
                        return new Found(previousNode, currentNode, nextNode);
                    }
                }
                previousNode = currentNode;
                currentNode = nextNode;
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

            node.get_next().set(found.getNext(), false);
            if (found.getPrevious().get_next().compareAndSet(found.getNext(), node, false, false)) {
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

            if (found.getPrevious().get_next().compareAndSet(found.getCurrent(), found.getNext(), false, false)) {
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
        AtomicMarkableReference<Node<T>> current = this.head.get_next();
        if (current.getReference() == null) {
            return true;
        } else {
            while (current.getReference() != null) {
                if (!current.isMarked()) {
                    return false;
                }

                current = current.getReference().get_next();
            }
            return true;
        }
    }

    public void printResult() {
        java.util.ArrayList<T> result = new java.util.ArrayList<>();
        Node<T> current = this.head.get_next().getReference();

        while (current != null) {
            if (contains(current.get_value())) {
                result.add(current.get_value());
            }
            current = current.get_next().getReference();
        }
        System.out.println("Result " + result.toString());
        System.out.println("Result " + result.size());
    }
}