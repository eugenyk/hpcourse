package etu.hpc.italyantsev;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private Node<E> tail = new Node<>(null, null);
    private Node<E> head = new Node<>(null, tail);

    public boolean offer(E e) {

        Node<E> newNode;
        Pair<Node<E>, Node<E>> pair;

        while (true) {
            pair = search(e);

            newNode = new Node<>(e, pair.getRight());

            if (pair.getLeft().casNext(pair.getRight(), newNode)) {
                return true;
            }
        }
    }

    public E poll() {
        Pair<Node<E>, Node<E>> pair;
        while (true) {
            pair = search(null);
            if (pair.getRight() == tail)
                return null;

            if (pair.getRight().mark()) {
                pair.getLeft().casNext(pair.getRight(), pair.getRight().getNext());

                return pair.getRight().value;
            }
        }
    }

    public E peek() {
        Pair<Node<E>, Node<E>> pair;

        pair = search(null);
        if (pair.getRight() == tail)
            return null;

        return pair.getRight().value;
    }

    @Override
    public boolean isEmpty() {
        while (true) {
            Node<E> top = head.getNext();
            if (top.isMarked()) {
                head.casNext(top, top.getNext());
                continue;
            }
            return top == tail;
        }
    }

    public int size() {
        Node<E> cur = head.getNext();
        int size = 0;
        while (cur != tail) {
            if (!cur.isMarked()) {
                size++;
            }
            cur = cur.getNext();
        }
        return size;
    }

    private Pair<Node<E>, Node<E>> search(E e) {
        Node<E> rightNode;
        Node<E> leftNodeNext = null;
        Node<E> leftNode = null;

        do {
            Node<E> t = head;

            do {
                if (!t.isMarked()) {
                    leftNode = t;
                    leftNodeNext = t.getNext();
                }
                t = t.getNext();
                if (t == tail)
                    break;
            } while (t.isMarked() || (e != null && t.value.compareTo(e) <= 0));

            rightNode = t;
            if (leftNodeNext == rightNode) {
                if ((rightNode != tail) && rightNode.next.isMarked()) {
                    continue;
                } else {
                    return Pair.of(leftNode, rightNode);
                }
            }

            if (leftNode.casNext(leftNodeNext, rightNode)) {
                if (rightNode == tail || !rightNode.next.isMarked()) {
                    return Pair.of(leftNode, rightNode);
                }
            }
        } while (true);
    }

    public Iterator<E> iterator() {
        return new Iterator<E>() {
            Node<E> cur = head;

            @Override
            public boolean hasNext() {
                while (cur.getNext() != tail) {
                    if (cur.isMarked()) {
                        cur = cur.getNext();
                        continue;
                    }
                    return true;
                }
                return false;
            }

            @Override
            public E next() {
                cur = cur.getNext();
                return cur.value;
            }
        };
    }

    private static class Node<E extends Comparable> {
        private E value;
        private AtomicMarkableReference<Node<E>> next;

        Node(E value, Node<E> next) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        boolean isMarked() {
            return next.isMarked();
        }

        Node<E> getNext() {
            return next.getReference();
        }

        boolean mark() {
            return next.compareAndSet(next.getReference(), next.getReference(), false, true);
        }

        boolean casNext(Node<E> expectedNext, Node<E> newNext) {
            return next.compareAndSet(expectedNext, newNext, false, false);
        }
    }

    private static class Pair<L, R> {
        final L left;
        final R right;

        static <L, R> Pair<L, R> of(L left, R right) {
            return new Pair<>(left, right);
        }

        Pair(L left, R right) {
            this.left = left;
            this.right = right;
        }

        L getLeft() {
            return this.left;
        }

        R getRight() {
            return this.right;
        }
    }
}
