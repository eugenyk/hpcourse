package ru.spbau.gbarto;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicStampedReference;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private enum NodeType {HEAD, TAIL, USUAL}
    private final static int MARKED = 1;
    private final static int UNMARKED = 0;

    private final AtomicInteger version;

    private class Node {
        private T value;
        private NodeType type;
        private AtomicStampedReference<Node> next;

        private Node(T value) {
            this.value = value;
            this.type = NodeType.USUAL;
            int v = version.getAndIncrement();
            this.next = new AtomicStampedReference<>(null, 2 * v + UNMARKED);
        }

        private Node(T value, NodeType type) {
            this.value = value;
            this.type = type;
            int v = version.getAndIncrement();
            this.next = new AtomicStampedReference<>(null, 2 * v + UNMARKED);
        }

        private T getValue() {
            return value;
        }

        private Node getNext() {
            return next.getReference();
        }

        private void setNext(Node next) {
            int v = version.getAndIncrement();
            this.next.set(next, 2 * v + UNMARKED);
        }

        private boolean isMarked() {
            return next.getStamp() % 2 == 1;
        }

        private int getVersion() {
            return next.getStamp() / 2;
        }

        private boolean compareAndSet(Node expectedReference,
                                      Node newReference,
                                      boolean newMark) {
            int expectedStamp = (this.next.getStamp() / 2) * 2 + UNMARKED;
            int v = version.getAndIncrement();
            int newStamp = v * 2 + (newMark ? MARKED : UNMARKED);
            return next.compareAndSet(expectedReference, newReference, expectedStamp, newStamp);
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

    private class PairNodeVersion {
        private Node node;
        private int version;

        private PairNodeVersion(Node node, int version) {
            this.node = node;
            this.version = version;
        }
    }

    private Node head;
    private Node tail;

    public LockFreeSet() {
        version = new AtomicInteger(0);
        head = new Node(null, NodeType.HEAD);
        tail = new Node(null, NodeType.TAIL);
        head.setNext(tail);
    }

    private List<PairNodeVersion> toList() {
        List<PairNodeVersion> list = new ArrayList<>();
        for (Node n = head.getNext(); n != tail; n = n.getNext()) {
            if (!n.isMarked()) {
                list.add(new PairNodeVersion(n, n.getVersion()));
            }
        }
        return list;
    }

    private boolean compareListsNodeVersion(List<PairNodeVersion> list1, List<PairNodeVersion> list2) {
        if (list1.size() != list2.size()) {
            return false;
        }

        Iterator<PairNodeVersion> it1 = list1.iterator();
        Iterator<PairNodeVersion> it2 = list2.iterator();
        while (it1.hasNext()) {
            PairNodeVersion p1 = it1.next();
            PairNodeVersion p2 = it2.next();
            if (p1.node != p2.node || p1.version != p2.version) {
                return false;
            }
        }

        return true;
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
        while (true) {
            Node n = head;
            Node nc = n.getNext();

            if (nc.isMarked()) {
                Node ncc = nc.getNext();
                n.compareAndSet(nc, ncc, false);
            } else {
                return (nc == tail);
            }

        }
    }

    @Override
    public Iterator<T> iterator() {
        List<PairNodeVersion> list1;
        List<PairNodeVersion> list2;
        do {
            list1 = toList();
            list2 = toList();
        } while (!compareListsNodeVersion(list1, list2));

        return list1.stream().map(p -> p.node.getValue()).iterator();
    }
}
