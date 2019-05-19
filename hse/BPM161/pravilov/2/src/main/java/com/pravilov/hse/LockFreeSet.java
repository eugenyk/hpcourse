package com.pravilov.hse;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private Node head;
    private Node tail;

    public LockFreeSet() {
        head = new Node(null, ValueType.MIN);
        tail = new Node(null, ValueType.MAX);
        head.next.set(tail, false);
    }

    private PredCurrNodePair find(T value) {
        while (true) {
            Node pred = head;
            Node curr = pred.next.getReference();
            Node succ;

            while (true) {
                succ = curr.next.getReference();
                boolean cmk = curr.next.isMarked();
                if (cmk) {
                    if (!pred.next.compareAndSet(curr, succ, false, false)) {
                        break;
                    }
                    curr = succ;
                } else {
                    if (compareNodeWithValue(curr, value) >= 0) {
                        return new PredCurrNodePair(pred, curr);
                    }
                    pred = curr;
                    curr = succ;
                }
            }
        }
    }

    @Override
    public boolean add(T value) {
        while(true) {
            PredCurrNodePair predCurr = find(value);
            Node pred = predCurr.pred;
            Node curr = predCurr.curr;

            if (curr.value == value) {
                return false;
            } else {
                Node node = new Node(value);
                node.next.set(curr, false);
                if (pred.next.compareAndSet(curr, node, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            PredCurrNodePair predCurr = find(value);
            Node pred = predCurr.pred;
            Node curr = predCurr.curr;

            if (compareNodeWithValue(curr, value) != 0) {
                return false;
            }
            Node succ = curr.next.getReference();
            if (!curr.next.compareAndSet(succ, succ, false, true)) {
                continue;
            }
            pred.next.compareAndSet(curr, succ, false, false);
            return true;
        }
    }

    @Override
    public boolean contains(T value) {
        Node curr = head;
        while (compareNodeWithValue(curr, value) < 0) {
            curr = curr.next.getReference();
        }
        return compareNodeWithValue(curr, value) == 0 && !curr.next.isMarked();
    }

    @Override
    public boolean isEmpty() {
        while (head.next.getReference() != tail) {
            if (!head.next.isMarked()) {
                return false;
            }
            Node curr = head.next.getReference();
            Node succ = curr.next.getReference();
            head.next.compareAndSet(curr, succ, false, false);
        }
        return true;
    }

    @Override
    public Iterator<T> iterator() {
        while (true) {
            List<Node> nodes1 = getListOfNodes();
            List<Node> nodes2 = getListOfNodes();
            if (nodes1.equals(nodes2)) {
                return nodes1.stream().map(node -> node.value).iterator();
            }
        }
    }

    private List<Node> getListOfNodes() {
        List<Node> nodes = new ArrayList<>();
        Node curr = head.next.getReference();
        while (curr != tail) {
            if (!curr.next.isMarked()) {
                nodes.add(curr);
            }
            curr = curr.next.getReference();
        }
        return nodes;
    }

    private class PredCurrNodePair {
        private Node pred;
        private Node curr;

        private PredCurrNodePair(Node pred, Node curr) {
            this.pred = pred;
            this.curr = curr;
        }
    }

    private enum ValueType {MIN, MAX, ORDINARY}

    private int compareNodeWithValue(Node node, T value) {
        if (node.valueType == ValueType.MIN) {
            return -1;
        }
        if (node.valueType == ValueType.MAX) {
            return 1;
        }
        return node.value.compareTo(value);
    }

    private class Node {
        private T value;
        private ValueType valueType;
        final private AtomicMarkableReference<Node> next = new AtomicMarkableReference<>(null, false);

        private Node(T value, ValueType valueType) {
            this.value = value;
            this.valueType = valueType;
        }

        private Node(T value) {
            this.value = value;
            this.valueType = ValueType.ORDINARY;
        }
    }
}
