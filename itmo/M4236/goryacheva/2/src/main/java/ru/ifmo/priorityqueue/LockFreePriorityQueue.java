package ru.ifmo.priorityqueue;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

    private final Node<E> tail = new Node<>(null, NodeType.TAIL, null);
    private final Node<E> head = new Node<>(null, NodeType.HEAD, tail);
    private AtomicInteger size = new AtomicInteger(0);

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        return size.get();
    }

    /**
     * Inserts the specified element into this priority queue.
     */
    @Override
    public boolean offer(E e) {
        if(e == null) {
            return false;
        }

        Node<E> newElement = new Node<>(e);
        while (true) {
            NodePair<E> pair = findPositionFor(e);
            Node<E> previous = pair.left;
            Node<E> next = pair.right;
            newElement.setNext(next, false);
            if(previous.CASNext(next, newElement, false, false)) {
                addToSize(+1);
                return true;
            }
        }
    }

    /**
     * Retrieves and removes the head of this queue,
     * or returns null if this queue is empty.
     */
    @Override
    public E poll() {
        if(head.getNext() == tail) {
            return null;
        }
        E min;
        while (true) {
            Node<E> currentMin = findMin();
            Node<E> nextMin = currentMin.getNext();
            if(currentMin.CASNext(nextMin, nextMin, false, true)) {
                head.CASNext(currentMin, nextMin, false, false);
                min = currentMin.getKey();
                addToSize(-1);
                break;
            }
        }
        return min;
    }

    /**
     * Retrieves, but does not remove, the head of this queue,
     * or returns null if this queue is empty.
     */
    @Override
    public E peek() {
        return findMin().getKey();
    }

    @Override
    public boolean isEmpty() {
        return size() == 0;
    }

    @Override
    public boolean contains(Object o) {
        E e = (E) o;
        return findPositionFor(e).left.getKey().equals(e);
    }

    private void addToSize(int change) {
        boolean success;
        do {
            int tmp = size.get();
            success = size.compareAndSet(tmp, tmp + change);
        } while (!success);
    }

    /**
     * Finds a pair of sequential nodes in the list,
     * between which the new element may be inserted.
     */
    private NodePair<E> findPositionFor(E e) {
        while(true) {
            Node<E> previous = head;
            Node<E> current = head.getNext();
            boolean holder[] = {false};
            Node<E> next;

            Node<E> tmpNode = head;
            Node<E> nextOfTmp = head.getNext(holder);
            do {
                if (!holder[0]) {
                    previous = tmpNode;
                    current = nextOfTmp;
                }
                tmpNode = nextOfTmp;
                if (tmpNode == tail) {
                    break;
                }
                nextOfTmp = tmpNode.getNext(holder);
            } while (holder[0] || tmpNode.compareTo(e) <= 0);

            next = tmpNode;
            if(previous.CASNext(current, next, false, false)) {
                if(next == tail || !next.nextIsMarked()) {
                    return new NodePair<>(previous, next);
                }
            }
        }
    }

    /**
     * Finds the node that contains current minimum value.
     */
    private Node<E> findMin() {
        if(size() == 0) {
            return tail;
        }
        final Node<E> previous = head;
        Node<E> current;
        Node<E> next;
        boolean holder[] = {false};

        while (true) {
            current = previous.getNext();
            next = current.getNext(holder);
            if(holder[0]) {
                previous.CASNext(current, next, false, false);
                continue;
            }
            return current;
        }
    }

    ////////////////////////////////////////////////////////////////

    private enum NodeType {
        HEAD,
        NODE,
        TAIL
    }

    private static class Node<E extends Comparable<E>> {
        private E key;
        private NodeType type;
        private AtomicMarkableReference<Node<E>> next;

        Node(E key, NodeType type, Node<E> next) {
            this.key = key;
            this.type = type;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        Node(E key) {
            this.key = key;
            type = NodeType.NODE;
            this.next = new AtomicMarkableReference<>(null, false);
        }

        int compareTo(E e) {
            return type == NodeType.TAIL ? 1 :
                    type == NodeType.HEAD ? -1 :
                            key.compareTo(e);
        }

        public E getKey() {
            return key;
        }

        boolean CASNext(Node<E> nextOld, Node<E> nextNew, boolean curMarkOld, boolean curMarkNew) {
            return next.compareAndSet(nextOld, nextNew, curMarkOld, curMarkNew);
        }

        Node<E> getNext() {
            return next.getReference();
        }

        Node<E> getNext(boolean holder[]) {
            return next.get(holder);
        }

        void setNext(Node<E> node, boolean mark) {
            next = new AtomicMarkableReference<>(node, mark);
        }

        boolean nextIsMarked() {
            return next.isMarked();
        }
    }

    private static class NodePair<E extends Comparable<E>> {
        private Node<E> left;
        private Node<E> right;

        NodePair(Node<E> leftNode, Node<E> rightNode) {
            left = leftNode;
            right = rightNode;
        }

        Node<E> getLeft() {
            return left;
        }

        Node<E> getRight() {
            return right;
        }

        public void print() {
            System.out.println("{" + left.getKey() + " " + left.type + ", " + right.getKey() + " " + right.type + "}");
        }
    }
}
