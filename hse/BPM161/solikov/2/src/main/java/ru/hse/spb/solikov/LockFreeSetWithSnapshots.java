package ru.hse.spb.solikov;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.stream.Collectors;

public class LockFreeSetWithSnapshots<T extends Comparable<T>> implements LockFreeSet<T> {

    public AtomicMarkableReference<ListNode<T>> head =
            new AtomicMarkableReference<>(new ListNode<>(), false);

    private ListNodePair<T> find(T data) {
        ListNode<T> x = head.getReference();
        ListNode<T> xNext = x.next.getReference();

        while (xNext != null) {
            if (data.equals(xNext.data) && !xNext.next.isMarked()) {
                return new ListNodePair<>(x, xNext);
            }
            x = xNext;
            xNext = xNext.next.getReference();
        }

        return new ListNodePair<>(x, null);
    }

    @Override
    public boolean add(T data) {
        ListNode<T> node = new ListNode<>(data);
        while (true) {
            ListNodePair<T> res = find(data);
            if (res.second != null) return false;
            if (res.first.next.compareAndSet(
                    null, node, false, false)) return true;
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
        AtomicMarkableReference<ListNode<T>> next = head.getReference().next;
        while (next.getReference() != null) {
            if (!next.isMarked()) return false;
            next = next.getReference().next;
        }
        return true;
    }

    private Set<ListNode<T>> scan() {
        Set<ListNode<T>> snapshot = new HashSet<>();
        ListNode<T> x = head.getReference();
        x = x.next.getReference();
        while (x != null) {
            if (!x.next.isMarked()) snapshot.add(x);
            x = x.next.getReference();
        }
        return snapshot;
    }

    @Override
    public Iterator<T> iterator() {
        while (true) {
            Set<ListNode<T>> snap1 = scan();
            Set<ListNode<T>> snap2 = scan();
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
