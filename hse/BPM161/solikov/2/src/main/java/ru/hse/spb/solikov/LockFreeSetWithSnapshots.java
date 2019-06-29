package ru.hse.spb.solikov;

import java.util.*;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.stream.Collectors;

public class LockFreeSetWithSnapshots<T extends Comparable<T>> implements LockFreeSet<T> {

    public ListNode<T> head =
            new ListNode<>();

    private ListNodePair<T> find(T value) {
        ListNodePair<T> res = new ListNodePair<>(null, head);
        while (res.second != null) {
            ListNode<T> nextNode = res.second.next.getReference();
            boolean ok = !res.second.next.isMarked();
            res.first = res.second;
            res.second = nextNode;
            if ((nextNode == null || nextNode.data == value) && ok) return res;
        }
        return res;
    }

    @Override
    public boolean add(T data) {
        ListNode<T> node = new ListNode<>(data);
        while (true) {
            ListNodePair<T> res = find(data);
            ListNode<T> x = res.first;
            ListNode<T> xNext = res.second;
            if (xNext != null) return false;
            if (x.next.compareAndSet(xNext, node, false, false)) return true;
        }
    }

    @Override
    public boolean remove(T data) {
        while (true) {
            ListNodePair<T> res = find(data);
            ListNode<T> x = res.first;
            ListNode<T> xNext = res.second;
            if (xNext == null) return false;
            ListNode<T> next = xNext.next.getReference();
            if (xNext.next.compareAndSet(next, next, false, true)) {
                x.next.compareAndSet(xNext, next, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(T data) {
        return find(data).second != null;
    }

    @Override
    public boolean isEmpty() {
        AtomicMarkableReference<ListNode<T>> next = head.next;
        while (next.getReference() != null) {
            if (next.isMarked()) return false;
            next = next.getReference().next;
        }
        return true;
    }

    private List<ListNode<T>> scan() {
        List<ListNode<T>> res = new ArrayList<>();
        ListNode<T> x = head;
        ListNode<T> xNext;
        boolean ok = x.next.isMarked();
        while (x.next.getReference() != null) {
            xNext = x.next.getReference();
            if (ok) return scan();
            x = xNext;
            res.add(x);
        }
        return res;
    }

    @Override
    public Iterator<T> iterator() {
        while (true) {
            List<ListNode<T>> snap1 = scan();
            List<ListNode<T>> snap2 = scan();
            if (snap1.size() != snap2.size()) continue;
            boolean same = true;
            for (Iterator<ListNode<T>> i = snap1.iterator(),
                 j = snap2.iterator(); i.hasNext(); ) {
                ListNode<T> x = i.next();
                ListNode<T> y = j.next();
                if (x != y) {
                    same = false;
                    break;
                }
            }
            if (same) {
                return snap1
                        .stream()
                        .map(a -> a.data)
                        .collect(Collectors.toSet())
                        .iterator();
            }
        }
    }

    public static class ListNode<T> {

        public T data = null;
        public AtomicMarkableReference<ListNode<T>> next =
                new AtomicMarkableReference<>(null, false);

        private ListNode() {
        }

        private ListNode(T data) {
            this.data = data;
        }

        private ListNode(T data, AtomicMarkableReference<ListNode> next) {
            this.data = data;
            if (next == null) return;
            this.next = new AtomicMarkableReference<>(null, false);
        }

    }

    public static class ListNodePair<T> {

        ListNode<T> first;
        ListNode<T> second;

        ListNodePair(ListNode<T> first, ListNode<T> second) {
            this.first = first;
            this.second = second;
        }
    }
}
