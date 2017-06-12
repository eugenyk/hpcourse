import java.util.concurrent.atomic.AtomicMarkableReference;

public class Set<T extends Comparable<T>> implements LockFreeSet<T> {

    private volatile Node<T> head = new Node<>(null);
    private volatile Node<T> tail = new Node<>(null);

    {head.next.set(tail, false);}

    private static class Node<T> {
        final private T key;
        final private AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);

        public Node(T key) {
            this.key = key;
        }

    }

    private static class Pair<T> {
        public Node<T> fst = null;
        public Node<T> snd = null;

        public Pair(Node<T> snd) {
            this.snd = snd;
        }

        public Pair<T> fillFst(Node<T> filler) {
            if (fst == null) {
                fst = filler;
            }
            return this;
        }
    }

    private Pair<T> find(final T key, final Node<T> node) {
        if (node == tail) {
            return new Pair<>(tail);
        }
        if (!node.next.isMarked() && node.key != null && node.key.compareTo(key) >= 0) {
            return new Pair<>(node);
        }
        return find(key, node.next.getReference()).fillFst(node);
    }

    private void delete(final Node<T> node) {
        Node<T> nextNode = node.next.getReference();
        if (nextNode == tail || node == tail) {
            return;
        }
        if (nextNode.next.isMarked()) {
            node.next.compareAndSet(nextNode, nextNode.next.getReference(), false, false);
        }
        delete(node.next.getReference());
    }

    @Override
    public boolean add(final T key) {
        final Node<T> newNode = new Node<>(key);
        Pair<T> place;
        do {
            delete(head);
            place = find(key, head);
            if (place.snd.key != null && !place.snd.next.isMarked() && place.snd.key.compareTo(key) == 0) {
                return false;
            }
            newNode.next.set(place.snd, false);
        } while (!place.fst.next.compareAndSet(place.snd, newNode, false, false));
        return true;
    }

    @Override
    public boolean remove(final T key) {
        synchronized (this) {
            Pair<T> place;
            Node<T> next;
            do {
                place = find(key, head);
                if (place.snd.key == null || place.snd.next.isMarked() || place.snd.key.compareTo(key) != 0) {
                    return false;
                }
                next = place.snd.next.getReference();
            } while (!place.snd.next.compareAndSet(next, next, false, true));

            delete(head);
            return true;
        }
    }

    @Override
    public boolean contains(final T key) {
        Pair<T> place = find(key, head);
        return place.snd.key != null && !place.snd.next.isMarked() && place.snd.key.compareTo(key) == 0;
    }

    @Override
    public boolean isEmpty() {
        Node<T> checker = head.next.getReference();
        boolean result = isEmptyRec(head);
        if (!result) {
            return false;
        }
        if (head.next.getReference() == checker) {
            return true;
        }
        return isEmpty();
    }

    private boolean isEmptyRec(Node<T> node) {
        if (node == tail) {
            return true;
        }
        return (node.next.isMarked() || node == head) && isEmptyRec(node.next.getReference());
    }

}