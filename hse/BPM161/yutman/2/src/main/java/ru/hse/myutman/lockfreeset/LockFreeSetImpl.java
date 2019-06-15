package ru.hse.myutman.lockfreeset;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicStampedReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements  LockFreeSet<T> {

    private static final int UNMARKED = 0;
    private static final int MARKED = 1;

    private final AtomicInteger counter = new AtomicInteger(0);
    private Node head = new Node(null);

    private class Node {
        final AtomicStampedReference<Node> next;
        final T value;

        Node (T value) {
            this.value = value;
            int version = counter.getAndIncrement();
            next = new AtomicStampedReference<>(null, version * 2 + UNMARKED);
        }

        Node(T value, Node next) {
            this(value);
            int stamp = this.next.getStamp();
            this.next.set(next, stamp);
        }
    }

    private class Pair {
        private Node prev;
        private Node curr;

        Pair(Node prev, Node curr) {
            this.prev = prev;
            this.curr = curr;
        }
    }

    private class LockFreeIterator implements Iterator<T> {

        private ArrayList<Node> snapshot;
        private Iterator<Node> iterator;

        LockFreeIterator() {
            snapshot = new ArrayList<>();
            while (true) {
                snapshot.clear();
                Node curr = head.next.getReference();
                while (curr != null) {
                    Node succ;
                    int[] tmp = new int[1];
                    succ = curr.next.get(tmp);
                    int stamp = tmp[0];
                    if ((stamp & 1) != MARKED) {
                        snapshot.add(curr);
                    }
                    curr = succ;
                }
                int cnt = 0;
                curr = head.next.getReference();
                boolean flag = true;
                while (curr != null) {
                    Node succ;
                    int[] tmp = new int[1];
                    succ = curr.next.get(tmp);
                    int stamp = tmp[0];
                    int mark = tmp[0];
                    if ((mark & 1) != MARKED) {
                        if (cnt >= snapshot.size() || snapshot.get(cnt++).next.getStamp() != stamp) {
                            flag = false;
                            break;
                        }
                    }
                    curr = succ;
                }
                if (flag && cnt == snapshot.size()) {
                    break;
                }
            }
            iterator = snapshot.iterator();
        }

        @Override
        public boolean hasNext() {
            return iterator.hasNext();
        }

        @Override
        public T next() {
            return iterator.next().value;
        }
    }

    private Pair findKey(T key) {
        while (true) {
            Node prev = head;
            Node curr = prev.next.getReference();
            while (true) {
                if (curr == null) return new Pair(prev, curr);
                Node succ;
                int[] tmp = new int[1];
                succ = curr.next.get(tmp);
                int stamp = tmp[0];
                if ((stamp & 1) == MARKED) {
                    int prevStamp = prev.next.getStamp();
                    if (!prev.next.compareAndSet(curr, succ, prevStamp, prevStamp)) {
                        break;
                    }
                    curr = succ;
                } else {
                    if (curr.value.compareTo(key) >= 0) return new Pair(prev, curr);
                    prev = curr;
                    curr = succ;
                }
            }
        }
    }

    @Override
    public boolean add(T value) {
        while (true) {
            Pair pr = findKey(value);
            Node prev = pr.prev;
            Node curr = pr.curr;
            if (curr != null && curr.value.compareTo(value) == 0) return false;
            Node next = new Node(value, curr);

            int stamp = prev.next.getStamp();
            if ((stamp & 1) == MARKED) continue;
            int newStamp = counter.getAndIncrement() * 2 + UNMARKED;

            // If new Node was added, change version
            if (prev.next.compareAndSet(curr, next, stamp, newStamp)) return true;
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Pair pr = findKey(value);
            Node prev = pr.prev;
            Node curr = pr.curr;
            if (curr == null || curr.value.compareTo(value) != 0) return false;
            int[] tmp = new int[1];
            Node succ = curr.next.get(tmp);
            int stamp = tmp[0];

            if (((stamp & 1) == MARKED) || !curr.next.compareAndSet(succ, succ, stamp, stamp ^ 1)) continue;

            int stamp1 = prev.next.getStamp();
            int newStamp = 2 * counter.getAndIncrement() + UNMARKED;
            prev.next.compareAndSet(curr, succ, stamp1, newStamp);
            return true;
        }
    }

    @Override
    public boolean contains(T value) {
        Node curr = head.next.getReference();
        while (true) {
            if (curr == null) return false;
            int[] tmp = new int[1];
            Node succ = curr.next.get(tmp);
            int mark = tmp[0] & 1;
            if (mark == MARKED) {
                curr = succ;
            } else {
                T val = curr.value;
                if (val.compareTo(value) > 0) return false;
                if (val.compareTo(value) == 0) return true;
                curr = succ;
            }
        }
    }

    @Override
    public boolean isEmpty() {
        while (true) {
            int[] tmp = new int[1];
            Node start = head.next.get(tmp);
            int stamp = tmp[0];
            if (start == null) return true;
            int mark = start.next.getStamp() & 1;
            if (mark == UNMARKED) {
                return false;
            } else {
                Node next = start.next.getReference();
                head.next.compareAndSet(start, next, stamp, stamp);
            }
        }
    }

    @Override
    public Iterator<T> iterator() {
        return new LockFreeIterator();
    }
}
