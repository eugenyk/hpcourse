package ru.spbau.gbarto;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private enum NodeType {HEAD, TAIL, USUAL}

    private class Node {
        private T value;
        private NodeType type;
        private final AtomicMarkableReference<Node> next = new AtomicMarkableReference<>(null, false);

        private Node(T value) {
            this.value = value;
            this.type = NodeType.USUAL;
        }

        private Node(T value, NodeType type) {
            this.value = value;
            this.type = type;
        }

        private T getValue() {
            return value;
        }

        private Node getNext() {
            return next.getReference();
        }

        private void setNext(Node next) {
            this.next.set(next, false);
        }

        private boolean isMarked() {
            return next.isMarked();
        }

        private boolean compareAndSet(Node expectedReference,
                              Node newReference,
                              boolean newMark) {
            return next.compareAndSet(expectedReference, newReference, false, newMark);
        }
        
        private int compare(T value) {
            switch (type) {
                case HEAD:
                    return -1;

                case TAIL:
                    return 1;

                default:
                    return this.value.compareTo(value);
            }
        } 
    }

    private class PairNode {
        private Node first;
        private Node second;
        
        private PairNode(Node first, Node second) {
            this.first = first;
            this.second = second;
        }
    }
    
    private Node head;
    private Node tail;

    public LockFreeSet() {
        head = new Node(null, NodeType.HEAD);
        tail = new Node(null, NodeType.TAIL);
        head.setNext(tail);
    }

    private List<Node> toList() {
        List<Node> list = new ArrayList<>();
        for (Node n = head.getNext(); n != tail; n = n.getNext()) {
            if (!n.isMarked()) {
                list.add(n);
            }
        }
        return list;
    }
    
    private PairNode find(T value) {
        while (true) {
            Node n = head;
            Node nc = n.getNext();

            while (true) {
                Node ncc = nc.getNext();

                if (nc.isMarked()) {
                    if (!n.compareAndSet(nc, ncc, false)) {
                        break;
                    }
                    nc = ncc;
                } else {
                    if (nc.compare(value) >= 0) {
                        return new PairNode(n, nc);
                    }
                    n = nc;
                    nc = ncc;
                }
            }
        }
    }
    
    @Override
    public boolean add(T value) {
        while(true) {
            PairNode pair = find(value);
            Node n = pair.first;
            Node nc = pair.second;

            if (nc.compare(value) == 0) {
                return false;
            } else {
                Node nn = new Node(value);
                nn.setNext(nc);
                if (n.compareAndSet(nc, nn, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            PairNode pair = find(value);
            Node n = pair.first;
            Node nc = pair.second;

            if (nc.compare(value) != 0) {
                return false;
            }

            Node ncc = nc.getNext();
            if (!nc.compareAndSet(ncc, ncc, true)) {
                continue;
            }

            n.compareAndSet(nc, ncc, false);

            return true;
        }
    }

    @Override
    public boolean contains(T value) {
        Node n;
        for (n = head.getNext(); n.compare(value) < 0; n = n.getNext());

        return n.compare(value) == 0 && !n.isMarked();
    }

    @Override
    public boolean isEmpty() {
        for (Node n = head.getNext(); n != tail; n = n.getNext()) {
            if (!n.isMarked()) {
                return false;
            }
        }
        return true;
    }

    @Override
    public Iterator<T> iterator() {
        List<Node> list1;
        List<Node> list2;
        do {
            list1 = toList();
            list2 = toList();
        } while (!list1.equals(list2));

        return list1.stream().map(Node::getValue).iterator();
    }
}
