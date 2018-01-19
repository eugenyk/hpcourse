package ru.spbau.mit.impl;

import ru.spbau.mit.api.LockFreeSet;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<E extends Comparable<E>> implements LockFreeSet<E> {

    private static class Node<E extends Comparable<E>> {
        private E key;
        private NodeType nodeType;
        private AtomicMarkableReference<Node<E>> next;

        Node(E key, NodeType nodeType, Node<E> next) {
            this.key = key;
            this.nodeType = nodeType;
            this.next = new AtomicMarkableReference<>(next, false);
        }
    }

    private static class Positions<E extends Comparable<E>> {
        private Node<E> leftNode;
        private Node<E> rightNode;

        Positions(Node<E> leftNode, Node<E> rightNode) {
            this.leftNode = leftNode;
            this.rightNode = rightNode;
        }
    }

    private enum NodeType {
        HEAD,
        NODE,
        TAIL
    }

    private Node<E> tail = new Node<>(null, NodeType.TAIL, null);
    private Node<E> head = new Node<>(null, NodeType.HEAD, tail);

    private static <E extends Comparable<E>> int compare(Node<E> cur, E key) {
        if (cur.nodeType == NodeType.TAIL) {
            return 1;
        }
        return cur.key.compareTo(key);
    }

    @Override
    public boolean add(E key) {
        Node<E> newNode = new Node<>(key, NodeType.NODE, null);
        while (true) {
            Positions<E> pos = find(key);
            if ((pos.rightNode != tail) && (pos.rightNode.key.compareTo(key) == 0)) {
                return false;
            }
            newNode.next.set(pos.rightNode, false);
            if (pos.leftNode.next.compareAndSet(pos.rightNode, newNode, false, false)) {
                return true;
            }
        }
    }

    private Positions<E> find(E key) {
        boolean[] marked = {false};
        Node<E> rightNode;
        Node<E> leftNode;
        Node<E> succ;
        boolean snip;

        retry:
        while (true) {
            leftNode = head;
            rightNode = head.next.getReference();
            while (rightNode != null) {
                succ = rightNode.next.get(marked);
                while (marked[0]) {
                    snip = leftNode.next.compareAndSet(rightNode, succ, false, false);
                    if (!snip) {
                        continue retry;
                    }
                    rightNode = succ;
                    succ = rightNode.next.get(marked);
                }
                if (compare(rightNode, key) >= 0) {
                    return new Positions<>(leftNode, rightNode);
                }
                leftNode = rightNode;
                rightNode = succ;
            }
        }
    }

    @Override
    public boolean remove(E key) {
        while (true) {
            Positions<E> pos = find(key);
            if ((pos.rightNode == tail) || (pos.rightNode.key.compareTo(key) != 0)) {
                return false;
            }

            Node<E> succ = pos.rightNode.next.getReference();
            if (pos.rightNode.next.attemptMark(succ, true)) {
                pos.leftNode.next.compareAndSet(pos.rightNode, succ, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(E key) {
        Positions<E> pos = find(key);
        return pos.rightNode != tail && pos.rightNode.key.compareTo(key) == 0;
    }

    @Override
    public boolean isEmpty() {
        while (head.next.getReference() != tail) {
            Node<E> first = head.next.getReference();
            if (!first.next.isMarked()) {
                return false;
            }
            head.next.compareAndSet(first, first.next.getReference(), false, false);
        }
        return true;
    }

}
