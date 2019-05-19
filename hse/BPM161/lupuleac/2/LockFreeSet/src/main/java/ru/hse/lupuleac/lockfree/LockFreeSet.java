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
            if (findResult.cur.next != null && findResult.cur.next
                    .valueStorage.value
                    .compareTo(value) == 0) {
                return false;
            }
            Node node = new Node(value);
            node.nextAndMark.set(findResult.cur);
            NodeLink newNext = new NodeLink(node, false);
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
            NodeLink curNext = nodeToBeRemoved.nextAndMark.get();
            if (curNext.marked
                    || nodeToBeRemoved.valueStorage.value
                    .compareTo(value) != 0) {
                return false;
            }

            nodeToBeRemoved.nextAndMark.set(new NodeLink(curNext.next, true));

            if (findResult.prev.nextAndMark.compareAndSet(findResult.cur,
                    new NodeLink(curNext.next, curNext.marked))) {
                return true;
            }
        }
    }


    private FindResult find(T key) {
        Node cur = head;
        NodeLink next = cur.nextAndMark.get();
        while (next.next != null && next.next.valueStorage.value.compareTo(key) <
                0) {
            cur = next.next;
            next = cur.nextAndMark.get();
        }
        return new FindResult(cur, next);
    }

    @Override
    public boolean contains(T value) {
        Node cur = head;
        while (cur != null && (cur.valueStorage.value == null || cur
                .valueStorage.value.compareTo
                        (value) <
                0)) {
            cur = cur.nextAndMark.get().next;
        }
        return cur != null && cur.valueStorage.value.compareTo(value) == 0 && !cur
                .nextAndMark
                .get().marked;
    }

    @Override
    public boolean isEmpty() {
        NodeLink nextAndMark = head.nextAndMark.get();
        while (nextAndMark.next != null) {
            if (!nextAndMark.marked) {
                return false;
            }
            nextAndMark = nextAndMark.next.nextAndMark.get();
        }
        return true;
    }

    private List<ValueStorage> getNodes() {
        Node cur = head;
        List<ValueStorage> res = new ArrayList<>();
        while (cur != null) {
            AtomicReference<NodeLink> atomicReference = cur.nextAndMark;
            cur = atomicReference.get().next;
            if (atomicReference.get().marked || cur == null) {
                continue;
            }
            res.add(cur.valueStorage);
        }
        return res;
    }

    public List<T> scan() {
        List<ValueStorage> snapshot = getNodes();
        while (true) {
            List<ValueStorage> secondSnapshot = getNodes();
            if (secondSnapshot.size() != snapshot.size()) {
                snapshot = secondSnapshot;
                continue;
            }
            boolean changed = false;
            for (int i = 0; i < snapshot.size(); i++) {
                long expected = secondSnapshot.get(i).id;
                if (snapshot.get(i).id != expected) {
                    changed = true;
                }
            }
            if (!changed) {
                return secondSnapshot.stream()
                        .map(t -> t.value)
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

    private class NodeLink {
        private NodeLink(Node next, boolean marked) {
            this.next = next;
            this.marked = marked;

        }

        private Node next;
        private boolean marked;
    }

    private class ValueStorage {
        private ValueStorage(T value) {
            this.value = value;
            this.id = LockFreeSet.this.counter.incrementAndGet();
        }

        private T value;
        private long id;
    }

    private class Node {
        private Node(T value) {
            this.valueStorage = new ValueStorage(value);
            nextAndMark = new AtomicReference<>(new NodeLink(null, false));
        }

        private ValueStorage valueStorage;
        private AtomicReference<NodeLink> nextAndMark;
    }

    private class FindResult {
        private Node prev;
        private NodeLink cur;

        private FindResult(Node prev, NodeLink cur) {
            this.prev = prev;
            this.cur = cur;
        }
    }
}
