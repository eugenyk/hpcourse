package com.xosmig.hpcourse.lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private volatile AtomicMarkableReference<ListNode<T>> head = new AtomicMarkableReference<>(null, false);

    @Override
    public boolean add(T value) {
        while (true) {
            ListNode<T> newNode = new ListNode<>(value);

            AtomicMarkableReference<ListNode<T>> lastNotMarkedRef = head;
            ListNode<T> cur = head.getReference();
            for (; cur != null; cur = cur.nextNode()) {

                if (cur.isMarked()) {
                    cur = deleteMarked(cur, lastNotMarkedRef);
                    continue;
                }

                int compareResult = cur.value.compareTo(value);

                if (compareResult == 0) {
                    return false;
                }

                if (compareResult > 0) {
                    break;
                }

                lastNotMarkedRef = cur.nextRef;
            }

            // cur might be null
            newNode.nextRef.set(cur, false);

            // restart if fails
            if (lastNotMarkedRef.compareAndSet(cur, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {

        AtomicMarkableReference<ListNode<T>> lastNotMarkedRef = head;
        for (ListNode<T> cur = head.getReference(); cur != null; cur = cur.nextNode()) {

            if (cur.isMarked()) {
                cur = deleteMarked(cur, lastNotMarkedRef);
                continue;
            }

            if (cur.value.equals(value)) {
                // found the element

                boolean removedByMe = true;
                ListNode<T> next;
                do {
                    next = cur.nextNode();
                    if (cur.isMarked()) {
                        removedByMe = false;
                        break;
                    }
                } while (!cur.nextRef.compareAndSet(next, next, false, true));

                deleteMarked(cur, lastNotMarkedRef);
                return removedByMe;
            }

            lastNotMarkedRef = cur.nextRef;
        }

        return false;
    }

    @Override
    public boolean contains(T value) {
        AtomicMarkableReference<ListNode<T>> lastNotMarkedRef = head;
        for (ListNode<T> cur = head.getReference(); cur != null; cur = cur.nextNode()) {

            if (cur.isMarked()) {
                cur = deleteMarked(cur, lastNotMarkedRef);
                continue;
            }

            if (cur.value.equals(value)) {
                return true;
            }

            lastNotMarkedRef = cur.nextRef;
        }
        return false;
    }

    @Override
    public boolean isEmpty() {
        return head.getReference() == null;
    }

    // Returns the last marked node in the sequence.
    // Try to delete all the marked nodes in the sequence, but ignore if the deletion fails.
    private ListNode<T> deleteMarked(ListNode<T> toRemove,
                                     AtomicMarkableReference<ListNode<T>> lastNotMarkedRef) {
        assert (toRemove.isMarked());
        ListNode<T> prev = toRemove;
        ListNode<T> cur = toRemove.nextNode();
        while (cur != null && cur.isMarked()) {
            prev = cur;
            cur = cur.nextNode();
        }
        lastNotMarkedRef.compareAndSet(toRemove, cur, false, false);
        return prev;
    }

    private final static class ListNode<T> {
        public final AtomicMarkableReference<ListNode<T>> nextRef;
        public final T value;

        public ListNode(T value) {
            this.nextRef = new AtomicMarkableReference<>(null, false);
            this.value = value;
        }

        public boolean isMarked() {
            return nextRef.isMarked();
        }

        public ListNode<T> nextNode() {
            return nextRef.getReference();
        }
    }
}
