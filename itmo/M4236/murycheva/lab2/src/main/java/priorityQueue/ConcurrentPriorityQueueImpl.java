package priorityQueue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class ConcurrentPriorityQueueImpl<E extends Comparable<E>> extends AbstractQueue<E> implements ConcurrentPriorityQueue<E> {

    private AtomicInteger size = new AtomicInteger(0);

    private Node<E> head;
    private Node<E> tail;

    public ConcurrentPriorityQueueImpl() {
        head = new Node<>(null);
        tail = new Node<>(null);
        head.setNext(tail);
    }

    @Override
    public int size() {
        return size.get();
    }

    @Override
    public boolean isEmpty() {
        return size.get() == 0;
    }

    @Override
    public boolean offer(E value) {
        Node<E> newNode = new Node<>(value);
        LeftAndRightNode<E> leftAndRightNodes;

        while (true) {
            leftAndRightNodes = findNeighbors(value);

            newNode.setNext(leftAndRightNodes.getRight());

            if (leftAndRightNodes.getLeft().casNext(leftAndRightNodes.getRight(), newNode)) {
                size.incrementAndGet();
                return true;
            }
        }
    }

    @Override
    public E poll() {
        LeftAndRightNode<E> leftAndRightNodes;
        Node<E> right, rightNext;

        while (true) {
            leftAndRightNodes = findNeighbors(null);
            right = leftAndRightNodes.getRight();

            if (right == tail) {
                return null;
            } else {
                rightNext = right.getNext();
                if (markAsDeleted(right.getNextReference(), rightNext)) {
                    size.decrementAndGet();
                    return right.getValue();
                }
            }
        }
    }

    @Override
    public E peek() {
        LeftAndRightNode<E> leftAndRightNodes;

        while (true) {
            leftAndRightNodes = findNeighbors(null);
            if (leftAndRightNodes.getRight() == tail) {
                return null;
            } else {
                return leftAndRightNodes.getRight().getValue();
            }
        }
    }

    private boolean markAsDeleted(AtomicMarkableReference<Node<E>> current, Node<E> node) {
        return current.compareAndSet(node, node, false, true);
    }

    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }

    class Node<T> {
        private T value;
        private AtomicMarkableReference<Node<T>> next;

        Node(T value) {
            this.value = value;
        }

        T getValue() {
            return value;
        }

        AtomicMarkableReference<Node<T>> getNextReference() {
            return next;
        }

        Node<T> getNext() {
            return next.getReference();
        }

        boolean casNext(Node<T> prevNext, Node<T> newNext) {
            return next.compareAndSet(prevNext, newNext, false, false);
        }

        void setNext(Node<T> next) {
            this.next = new AtomicMarkableReference<>(next, false);
        }

        boolean isMarked() {
            if (next == null) {
                return true;
            }

            return next.isMarked();
        }
    }

    class LeftAndRightNode<T> {
        private Node<T> left;
        private Node<T> right;

        LeftAndRightNode(Node<T> left, Node<T> right) {
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

    private LeftAndRightNode<E> findNeighbors(E value) {
        Node<E> left = head;
        Node<E> current = head.getNext();
        Node<E> right;

        while (true) {
            Node<E> tmpCurrent = head;
            Node<E> tmpCurrentNext = head.getNext();

            do {

                if (!tmpCurrent.isMarked()) {
                    left = tmpCurrent;
                    current = tmpCurrentNext;
                }

                tmpCurrent = tmpCurrent.getNext();
                if (tmpCurrent == tail) {
                    break;
                }

                tmpCurrentNext = tmpCurrent.getNext();

            } while (tmpCurrent.isMarked() || (value != null && tmpCurrent.getValue().compareTo(value) <= 0));

            right = tmpCurrent;

            if (current == right) {
                if (!(right != tail && right.isMarked())) {
                    return new LeftAndRightNode<>(left, right);
                }
            }

            if (left.casNext(current, right)) {
                if (right == tail || !right.isMarked()) {
                    return new LeftAndRightNode<>(left, right);
                }
            }
        }
    }

}
