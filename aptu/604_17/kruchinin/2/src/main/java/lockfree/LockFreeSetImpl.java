package lockfree;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<E extends Comparable<E>> implements LockFreeSet<E> {
    private final Node<E> tail = new Node<>(null, null);
    private final Node<E> head = new Node<>(tail, null);

    @Override
    public boolean add(E value) {
        while (true) {
            final Bounds<E> bounds = find(value);
            final Node<E> pred = bounds.pred;
            final Node<E> curr = bounds.curr;

            if (curr != tail && curr.value.compareTo(value) == 0) {
                return false;
            }

            final Node<E> node = new Node<>(curr, value);
            if (pred.next.compareAndSet(curr, node, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(E value) {
        while (true) {
            final Bounds<E> bounds = find(value);
            final Node<E> pred = bounds.pred;
            final Node<E> curr = bounds.curr;
            final Node<E> succ = curr.getNext();

            if (curr == tail ||
                !(curr.value.compareTo(value) == 0)) {
                return false;
            }

            if (!curr.next.attemptMark(succ, true)) {
                continue;
            }

            pred.next.compareAndSet(curr, succ, false, false);
            return true;
        }
    }

    @Override
    public boolean contains(E value) {
        final Node<E> node = find(value).curr;
        return ! (node == tail) && (node.value.compareTo(value) == 0);
    }

    @Override
    public boolean isEmpty() {
        Node<E> pred = head;
        Node<E> curr = pred.getNext();
        Node<E> succ;
        boolean[] currWasDeleted = { false };

        while (curr != tail) {
            succ = curr.getNext(currWasDeleted);

            if (pred.nextIsChanged(curr)
                    || currWasDeleted[0]
                    && !pred.next.compareAndSet(curr, succ, false, false))
            {
                pred = head;
                curr = pred.getNext();
                continue;
            }

            if (!currWasDeleted[0]) {
                return false;
            }

            curr = succ;
        }

        return true;
    }

    private static class Node<E extends Comparable<E>> {
        AtomicMarkableReference<Node<E>> next;
        final E value;

        Node(Node<E> next, E value) {
            this.next = new AtomicMarkableReference<>(next, false);
            this.value = value;
        }

        Node<E> getNext() {
            return next.getReference();
        }

        Node<E> getNext(boolean[] mark) {
            return next.get(mark);
        }

        boolean nextIsChanged(Node<E> next) {
            return this.next.isMarked()
                    || this.next.getReference() != next;
        }
    }

    private static class Bounds<E extends Comparable<E>> {
        final Node<E> pred;
        final Node<E> curr;

        Bounds(Node<E> pred, Node<E> curr) {
            this.pred = pred;
            this.curr = curr;
        }
    }

    private Bounds<E> find(E value) {
        boolean[] currWasDeleted = {false};

        Node<E> pred = head;
        Node<E> curr = pred.getNext();
        Node<E> succ;

        while (curr != tail && curr.value.compareTo(value) >= 0) {
            succ = curr.getNext(currWasDeleted);

            if (pred.nextIsChanged(curr)
                    || currWasDeleted[0]
                    && !pred.next.compareAndSet(curr, succ, false, false))
            {
                pred = head;
                curr = pred.getNext();
                continue;
            }

            if (!currWasDeleted[0]) {
                if (curr.value.compareTo(value) == 0)
                    return new Bounds<>(pred, curr);
                else
                    pred = curr;
            }

            curr = succ;
        }

        return new Bounds<>(pred, curr);
    }
}
