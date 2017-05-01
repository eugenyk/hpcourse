package lockfree_set;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private volatile Node<T> head = new Node<>(null);
    private volatile Node<T> tail = new Node<>(null);

    LockFreeSetImpl() {
        head.next.set(tail, false);
    }

    @Override
    public boolean add(final T value) {
        final Node<T> newNode = new Node<>(value);
        boolean marked[] = new boolean[1];

        while (true) {
            final Node<T> node = find(value);
            final Node<T> next = node.next.get(marked);

            if (next != tail && next.value.equals(value)) {
                return false;
            }

            newNode.next.set(next, false);
            if (next == tail || next.value.compareTo(value) > 0) {
                if (node.next.compareAndSet(next, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(final T value) {
        boolean[] marked = new boolean[1];

        while (true) {
            Node<T> node = find(value);
            Node<T> next = node.next.get(marked);
            if (next == tail) {
                return false;
            }
            if (next.value.compareTo(value) < 0) {
                continue;
            }
            Node<T> nextNext = next.next.getReference();
            if (!next.value.equals(value)) {
                return false;
            } else {
                if (next.next.attemptMark(nextNext, true)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean contains(final T value) {
        boolean[] marked = new boolean[1];
        Node<T> curNext = head.next.get(marked);

        while (curNext != tail && !curNext.value.equals(value)) {
            curNext = curNext.next.get(marked);
        }

        return curNext != tail && !curNext.next.isMarked();
    }

    @Override
    public boolean isEmpty() {
        Node<T> cur = head;
        boolean[] marked = new boolean[1];
        Node<T> curNext = head.next.get(marked);
        final Node<T> headNext = curNext;

        while (cur != tail) {
            if (!marked[0] && cur != head) {
                return false;
            }
            cur = curNext;
            curNext = cur.next.get(marked);
        }

        return head.next.getReference() == headNext;
    }

    private Node<T> find(final T key) {
        startFind:
        while (true) {
            Node<T> cur = head;

            boolean[] marked = new boolean[1];
            Node<T> curNext = cur.next.get(marked);
            Node<T> left = null;
            Node<T> leftNext = null;

            while (curNext != tail && (left != null || curNext.value.compareTo(key) < 0)) {
                Node<T> curNextNext = curNext.next.get(marked);
                if (marked[0] && left == null) {
                    left = cur;
                    leftNext = curNext;
                } else if (!marked[0] && left != null) {
                    if (!left.next.compareAndSet(leftNext, curNext, false, false)) {
                        continue startFind;
                    }
                    if (curNext.value.compareTo(key) >= 0) {
                        return left;
                    }
                    left = leftNext = null;

                }
                cur = curNext;
                curNext = curNextNext;
            }

            if (left != null) {
                if (!left.next.compareAndSet(leftNext, tail, false, false)) {
                    continue;
                }
                return left;
            }
            return cur;
        }
    }

    private static class Node<T> {
        final T value;
        final AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);

        Node(final T value) {
            this.value = value;
        }
    }
}
