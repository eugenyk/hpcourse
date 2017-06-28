import java.util.concurrent.atomic.*;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T>{
    private static class Node<T extends Comparable<T>> {
        private final T key;
        private AtomicMarkableReference<Node<T>> next;

        private Node(T key) {
            this(key, null);
        }

        private Node(T key, Node<T> next) {
            this.key = key;
            this.next = new AtomicMarkableReference<Node<T>>(next, false);
        }

        boolean isUnset() {
            return key == null;
        }
    }

    private class SearchResultSegment {
        private final Node<T> begin;
        private final Node<T> end;

        private SearchResultSegment(Node<T> first, Node<T> second) {
            this.begin = first;
            this.end = second;
        }

        private boolean contains(T value) {
            return !atEnd(end) && end.key.equals(value);
        }
    }


    private final Node<T> head;
    private final Node<T> tail;

    public LockFreeSetImpl() {
        tail = new Node<T>(null);
        head = new Node<T>(null, tail);
    }

    private boolean atEnd(Node<T> node) {
        return node == tail;
    }

    private boolean stepExpected(Node<T> it, T value) {
        return !atEnd(it) && (it.isUnset() || it.next.isMarked() || it.key.compareTo(value) < 0);
    }

    private SearchResultSegment search(T key) {
        Node<T> left = head;
        Node<T> right;

        while (true) {
            Node<T> it = head;
            Node<T> left_next = null;

            while (stepExpected(it, key)) {
                if (!it.next.isMarked()) {
                    left = it;
                    left_next = it.next.getReference();
                }

                it = it.next.getReference();
            }

            right = it;

            if (left_next == right || left.next.compareAndSet(left_next, right, false, false)) {
                if (!atEnd(right)  && right.next.isMarked()) {
                    continue;
                }

                return new SearchResultSegment(left, right);
            }

        }
    }

    private boolean tryRemovePhys(SearchResultSegment result) {
        Node<T> rightNext = result.end.next.getReference();
        if (!result.begin.next.compareAndSet(result.end, rightNext, false,false)) {
            search(result.end.key);
            return true;
        }
        return false;
    }

    @Override
    public boolean add(T value) {
        final Node<T> newNode = new Node<T>(value);

        while (true) {
            SearchResultSegment result = search(value);

            if (result.contains(value))
                return false;

            newNode.next.set(result.end, false);

            if (result.begin.next.compareAndSet(result.end, newNode, false, false)) {
                return true;
            }
        }
    }



    @Override
    public boolean remove(T value) {
        SearchResultSegment result;
        Node<T> rightNext;

        while (true) {
            result = search(value);

            if (!result.contains(value))
                return false;

            rightNext = result.end.next.getReference();

            if (result.end.next.attemptMark(rightNext, true)) {
                break;
            }
        };

        tryRemovePhys(result);

        return true;
    }

    @Override
    public boolean contains(T value) {
        Node<T> it = head;

        while (stepExpected(it, value)) {
            it = it.next.getReference();
        }

        return (!atEnd((it))) && it.key.equals(value);
    }

    @Override
    public boolean isEmpty() {
        while (true) {
            Node<T> next = head.next.getReference();

            if (atEnd(next))
                break;

            if (!next.next.isMarked()) {
                return false;
            }

            head.next.compareAndSet(next, next.next.getReference(), false, false);
        }

        return true;
    }
}
