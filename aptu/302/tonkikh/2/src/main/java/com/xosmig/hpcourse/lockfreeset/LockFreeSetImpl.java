package com.xosmig.hpcourse.lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private volatile AtomicMarkableReference<ListNode<T>> head = new AtomicMarkableReference<>(null, false);

    @Override
    public boolean add(T value) {
        ListNode<T> newNode = new ListNode<>(value);

        AtomicMarkableReference<ListNode<T>> lastNotMarkedRef = head;
        ListNode<T> cur = head.getReference();
        for (; cur != null; cur = cur.next.getReference()) {

            if (cur.next.isMarked()) {
                tryRemoveNode(cur, lastNotMarkedRef);
                continue;
            }

            int compareResult = cur.value.compareTo(value);

            if (compareResult == 0) {
                return false;
            }

            if (compareResult > 0) {
                break;
            }

            lastNotMarkedRef = cur.next;
        }

        // restart if fails
        // cur might be null
        newNode.next.set(cur, false);
        return lastNotMarkedRef.compareAndSet(cur, newNode, false, false) || add(value);
    }

    @Override
    public boolean remove(T value) {

        AtomicMarkableReference<ListNode<T>> lastNotMarkedRef = head;
        for (ListNode<T> cur = head.getReference(); cur != null; cur = cur.next.getReference()) {

            if (cur.next.isMarked()) {
                tryRemoveNode(cur, lastNotMarkedRef);
                continue;
            }

            if (cur.value.equals(value)) {
                // found the element

                boolean removedByMe = true;
                ListNode<T> next;
                do {
                    next = cur.next.getReference();
                    if (cur.next.isMarked()) {
                        removedByMe = false;
                        break;
                    }
                } while (!cur.next.compareAndSet(next, next, false, true));

                tryRemoveNode(cur, lastNotMarkedRef);
                return removedByMe;
            }

            lastNotMarkedRef = cur.next;
        }

        return false;
    }

    @Override
    public boolean contains(T value) {
        AtomicMarkableReference<ListNode<T>> lastNotMarkedRef = head;
        for (ListNode<T> cur = head.getReference(); cur != null; cur = cur.next.getReference()) {

            if (cur.next.isMarked()) {
                tryRemoveNode(cur, lastNotMarkedRef);
                continue;
            }

            if (cur.value.equals(value)) {
                return true;
            }

            lastNotMarkedRef = cur.next;
        }
        return false;
    }

    @Override
    public boolean isEmpty() {
        return head.getReference() == null;
    }

    private void tryRemoveNode(ListNode<T> cur, AtomicMarkableReference<ListNode<T>> lastNotMarkedRef) {
        // Try to remove the element. Ignore if fails.
        lastNotMarkedRef.compareAndSet(cur, cur.next.getReference(), false, false);
    }

    private static class ListNode<T> {
        public final AtomicMarkableReference<ListNode<T>> next;
        public final T value;

        public ListNode(T value) {
            this.next = new AtomicMarkableReference<>(null, false);
            this.value = value;
        }
    }
}
