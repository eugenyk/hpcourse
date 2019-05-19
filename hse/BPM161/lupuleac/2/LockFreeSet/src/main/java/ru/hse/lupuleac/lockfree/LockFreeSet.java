package ru.hse.lupuleac.lockfree;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

public class LockFreeSet<T extends Comparable<T>> implements
        Set<T> {
    private Node head;
    private AtomicLong counter;

    public LockFreeSet() {
        counter = new AtomicLong(0);
        head = new Node(null);
    }

    @Override
    public boolean add(T value) {
        while (true) {
            FindResult findResult = find(value);

            if (findResult.cur.marked) {
                continue;
            }
            if (findResult.cur.next != null && findResult.cur.next.value
                    .compareTo(value) == 0) {
                return false;
            }
            Node node = new Node(value);
            node.nextAndMark.set(findResult.cur);
            NextAndMark newNext = new NextAndMark(node, false);
            if (findResult.prev.nextAndMark.compareAndSet(findResult.cur, newNext)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            FindResult findResult = find(value);
            if (findResult.cur.marked) {
                continue;
            }
            Node nodeToBeRemoved = findResult.cur.next;
            if (nodeToBeRemoved == null) {
                return false;
            }
            NextAndMark curNext = nodeToBeRemoved.nextAndMark.get();
            if (curNext.marked
                    || nodeToBeRemoved.value.compareTo(value) != 0) {
                return false;
            }

            nodeToBeRemoved.nextAndMark.set(new NextAndMark(curNext.next, true));

            if (findResult.prev.nextAndMark.compareAndSet(findResult.cur,
                    new NextAndMark(curNext.next, curNext.marked))) {
                return true;
            }
        }
    }


    private FindResult find(T key) {
        Node cur = head;
        NextAndMark next = cur.nextAndMark.get();
        while (next.next != null && next.next.value.compareTo(key) < 0) {
            cur = next.next;
            next = cur.nextAndMark.get();
        }
        return new FindResult(cur, next);
    }

    @Override
    public boolean contains(T value) {
        Node cur = head;
        while (cur != null && (cur.value == null || cur.value.compareTo
                (value) <
                0)) {
            cur = cur.nextAndMark.get().next;
        }
        return cur != null && cur.value.compareTo(value) == 0 && !cur.nextAndMark
                .get().marked;
    }

    @Override
    public boolean isEmpty() {
        NextAndMark nextAndMark = head.nextAndMark.get();
        while (nextAndMark.next != null) {
            if (!nextAndMark.marked) {
                return false;
            }
            nextAndMark = nextAndMark.next.nextAndMark.get();
        }
        return true;
    }

    private List<AtomicReference<NextAndMark>> getNodes() {
        Node cur = head;
        List<AtomicReference<NextAndMark>> res = new ArrayList<>();
        while (cur != null) {
            AtomicReference<NextAndMark> atomicReference = cur.nextAndMark;
            cur = atomicReference.get().next;
            if (atomicReference.get().marked) {
                continue;
            }
            res.add(atomicReference);
        }
        return res;
    }

    public List<T> scan() {
        List<AtomicReference<NextAndMark>> snapshot = getNodes();
        while (true) {
            List<AtomicReference<NextAndMark>> secondSnapshot = getNodes();
            if (secondSnapshot.size() != snapshot.size()) {
                snapshot = secondSnapshot;
                continue;
            }
            boolean changed = false;
            for (int i = 0; i < snapshot.size(); i++) {
                NextAndMark expected = secondSnapshot.get(i).get();
                if (!snapshot.get(i).compareAndSet(expected, expected)) {
                    changed = true;
                }
            }
            if (!changed) {
                return secondSnapshot.stream().filter(t -> t.get().next != null)
                        .map(t -> t.get().next.value)
                        .collect(
                                Collectors.toList()
                        );
            }
            snapshot = secondSnapshot;
        }
    }


    @Override
    public Iterator<T> iterator() {
        return scan().iterator();
    }

    private class NextAndMark {
        private NextAndMark(Node next, boolean marked) {
            this.next = next;
            this.marked = marked;
            id = LockFreeSet.this.counter.incrementAndGet();
        }

        private long id;
        private Node next;
        private boolean marked;
    }

    private class Node {
        private Node(T value) {
            this.value = value;
            nextAndMark = new AtomicReference<>(new NextAndMark(null, false));
        }

        private T value;
        private AtomicReference<NextAndMark> nextAndMark;
    }

    private class FindResult {
        private Node prev;
        private NextAndMark cur;

        private FindResult(Node prev, NextAndMark cur) {
            this.prev = prev;
            this.cur = cur;
        }
    }
}
