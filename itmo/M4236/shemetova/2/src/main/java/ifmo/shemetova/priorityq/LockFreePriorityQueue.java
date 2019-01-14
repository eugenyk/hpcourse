package ifmo.shemetova.priorityq;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;
//https://www.cl.cam.ac.uk/research/srg/netos/papers/2001-caslists.pdf


public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private Node<E> head; //empty, next shows first element
    private Node<E> tail; //empty, after the last element

    private AtomicInteger size = new AtomicInteger(0);

    private static class Node<E> {
        private final E data;
        private AtomicMarkableReference<Node<E>> next;


        Node(E element) {
            data = element;
        }

        void setNext(Node<E> n) {
            next = new AtomicMarkableReference<Node<E>>(n, false);
        }

        public Node<E> getNext() {
            return next.getReference();
        }

        public E getData() {
            return data;
        }

        public AtomicMarkableReference<Node<E>> getReference() {
            return next;
        }

        public boolean isMarked() {
            AtomicMarkableReference<Node<E>> node = next;
            if (node != null) {
                return next.isMarked();
            } else return false;
        }

        public boolean casNext(Node<E> address, Node<E> newAddress) {
            return next.compareAndSet(address, newAddress, false, false);
        }

    }

    private boolean mark(AtomicMarkableReference<Node<E>> ref, Node<E> e) {
        return ref.compareAndSet(e, e, false, true);
    }

    private boolean unmark(AtomicMarkableReference<Node<E>> ref, Node<E> e) {
        return ref.compareAndSet(e, e, false, false);
    }


    private static class LeftAndRight<E> {
        private Node<E> left;
        private Node<E> right;

        public LeftAndRight(Node<E> l, Node<E> r) {
            left = l;
            right = r;
        }

        public Node<E> getLeft() {
            return left;
        }

        public Node<E> getRight() {
            return right;
        }

    }

    public LockFreePriorityQueue() {
        head = new Node<E>(null);
        tail = new Node<E>(null);
        head.setNext(tail);
    }

    public int size() {
        return size.get();
    }

    public boolean isEmpty() {
        return peek() == null;
    }

    public Iterator<E> iterator() {
        //throw new NoSuchMethodException("No iterator for priority queue!");
        return null;
    }


    public boolean offer(E e) {
        Node<E> node = new Node<E>(e);
        LeftAndRight<E> lr;
        Node<E> right;

        do {
            lr = search(e);
            right = lr.getRight();
            if (right != tail && right.getData() == e) {
                return false;
            }
            node.setNext(right);
            if (lr.getLeft().casNext(right, node)) {//lr.getLeft().getNext().getReference().compareAndSet(right, node, false, false)) {
                size.incrementAndGet();
                return true;
            }

        } while (true);
    }

    public E poll() {
        LeftAndRight<E> lr;
        Node<E> right;
        do {
            lr = search(null);
            right = lr.getRight();
            if (right == tail) {
                return null;
            } else {
                Node<E> rightNext = right.getNext();
                if (mark(right.getReference(), rightNext)) {
                    size.decrementAndGet();
                    return right.getData();
                }

            }

        } while (true);

    }

    public E peek() {
        while (true) {
            LeftAndRight<E> lr = search(null);
            Node<E> right = lr.getRight();
            if (right == head) {
                return null;
            } else {
                return right.getData();
            }
        }
    }

    private LeftAndRight<E> search(E key) {
        Node<E> right;
        Node<E> left = head;
        Node<E> leftNext = head.next.getReference();
        do {
            Node<E> currentNode = head;
            Node<E> currentNodeNext = head.getNext();
            do {
                if (!currentNode.isMarked()) {
                    left = currentNode;
                    leftNext = currentNodeNext;
                }
                //unmark(currentNode.getReference(), currentNode.getNext());
                currentNode = currentNode.getNext();
                if (currentNode == tail) { //list is over
                    break;
                }

                currentNodeNext = currentNode.getNext();

            }
            while (currentNode.isMarked() || (key != null && currentNode.data.compareTo(key) <= 0));

            right = currentNode;

            if (leftNext == right) {
                if (right != tail && right.isMarked()) {
                    continue;
                } else {
                    return new LeftAndRight<E>(left, right);
                }
            }

            if (left.casNext(leftNext, right)) {
                if (right == tail || !right.isMarked()) {
                    return new LeftAndRight<E>(left, right);
                }

            }


        } while (true);

    }
}
