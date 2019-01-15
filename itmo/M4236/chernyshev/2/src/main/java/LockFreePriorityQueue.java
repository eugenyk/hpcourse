import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private final Node<E> head;
    private final Node<E> tail;
    private AtomicInteger size_ = new AtomicInteger(0);

    LockFreePriorityQueue() {
        tail = new Node<>(null, null);
        head = new Node<>(null, tail);
    }

    @SuppressWarnings("NullableProblems")
    @Override
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException("Iterator is not yet implemented!");
    }

    @Override
    public boolean offer(E key) {
        if (key == null) {
            return false;
        }
        Node<E> newElement = new Node<>(key, null);
        while (true) {
            NodePair<E> insertNeighbours = findPlaceToInsert(key);
            Node<E> previous = insertNeighbours.left;
            Node<E> next = insertNeighbours.right;
            newElement.next = new AtomicMarkableReference<>(next, false);
            if (previous.CASNext(next, newElement)) {
                size_.incrementAndGet();
                return true;
            }
        }
    }

    @Override
    public E poll() {
        E result;
        while (true) {
            Node<E> currentLast = findLast();
            if (currentLast == tail) {
                return null;
            }
            Node<E> newLast = currentLast.next.getReference();
            if (currentLast.next.compareAndSet(newLast, newLast, false, true)) {
                head.next.compareAndSet(currentLast, newLast, false, false);
                result = currentLast.val;
                size_.decrementAndGet();
                break;
            }
        }
        return result;
    }

    @Override
    public E peek() {
        return findLast().val;
    }

    @Override
    public int size() {
        return size_.get();
    }

    @Override
    public boolean isEmpty() {
        return peek() == null;
    }

    static class Node<T> {
        final T val;
        AtomicMarkableReference<Node<T>> next;

        Node(T val, Node<T> next) {
            this.val = val;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        boolean CASNext(Node<T> curNext, Node<T> newNext) {
            return next.compareAndSet(curNext, newNext, false, false);
        }
    }

    private Node<E> findLast() {
        if (size() == 0) {
            return tail;
        }
        final Node<E> previous = head;
        boolean[] holder = {false};

        while (true) {
            Node<E> current = previous.next.getReference();
            Node<E> next = current.next.get(holder);
            if (holder[0]) {
                previous.next.compareAndSet(current, next, false, false);
                continue;
            }
            return current;
        }
    }

    private class NodePair<T> {
        private final Node<T> left;
        private final Node<T> right;

        private NodePair(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }
    }

    private NodePair<E> findPlaceToInsert(E e) {
        while (true) {
            Node<E> prev = head;
            Node<E> cur = head.next.getReference();
            Node<E> next;
            boolean[] holder = {false};

            Node<E> temp = head;
            Node<E> tempNext = head.next.get(holder);
            do {
                if (!holder[0]) {
                    prev = temp;
                    cur = tempNext;
                }
                temp = tempNext;
                if (temp == tail) {
                    break;
                }
                tempNext = temp.next.get(holder);
            } while (holder[0] || temp.val == null || temp.val.compareTo(e) <= 0);

            next = temp;
            if (prev.CASNext(cur, next)) {
                if (next == tail || !next.next.isMarked()) {
                    return new NodePair<>(prev, next);
                }
            }
        }
    }

}