package ru.spbau.mit;

import java.util.concurrent.atomic.AtomicMarkableReference;

class LockFreeListImpl<T extends Comparable<T>> implements LockFreeList<T> {

    @Override
    public boolean isEmpty() {
        while (head.getReference().next.getReference() != null) {
            if(!head.getReference().next.isMarked()) {
                return false;
            }
            Node curr = head.getReference().next.getReference();
            head.getReference().next.compareAndSet(curr, head.getReference().next.getReference(), false, false);
        }
        return true;
    }

    @Override
    public boolean add(T value) {
        if (value == null) {
            return false;
        }
        while(true) {
            final Tuple result = find(value);
            if (result.curr != null) {
                return false;
            }
            Node newNode = new Node(value);
            newNode.next = new AtomicMarkableReference<>(result.succ, false);
            if (result.pred.next.compareAndSet(result.succ, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while(true) {
            final Tuple result = find(value);
            if (result.curr == null) {
                return false;
            }
            if (!result.curr.next.attemptMark(result.succ, true)) {
                continue;
            }
            result.pred.next.compareAndSet(result.curr, result.succ, false, false);
            return true;
        }
    }

    @Override
    public boolean contains(T value) {
        return find(value).curr != null;
    }

    private Tuple find(T value) {
        Node pred, curr, succ = null;
        final boolean[] mark = new boolean[1];
        retry:
        while(true) {
            pred = this.head.getReference();
            curr = pred.next.getReference();
            while(true) {
                if (curr == null) {
                    return new Tuple(pred, null, succ);
                }
                succ = curr.next.get(mark);
                if (mark[0]) {
                    if (!pred.next.compareAndSet(curr, succ, false, false)) {
                        continue retry;
                    }
                    curr = succ;
                } else {
                    if (curr.value != null && curr.value.compareTo(value) >= 0) {
                        return new Tuple(pred, curr, succ);
                    }
                    pred = curr;
                    curr = succ;
                }
            }
        }
    }

    private final class Node {
        T value;
        AtomicMarkableReference<Node> next;

        Node(T value) {
            this.value = value;
            next = new AtomicMarkableReference<>(null, false);
        }

        Node() {
            next = new AtomicMarkableReference<>(null, false);
        }
    }

    private final class Tuple {
        Node pred;
        Node curr;
        Node succ;

        Tuple(Node p, Node c, Node s) {
            pred = p;
            curr = c;
            succ = s;
        }
    }

    private final AtomicMarkableReference<Node> head = new AtomicMarkableReference<>(new Node(), false);
}
