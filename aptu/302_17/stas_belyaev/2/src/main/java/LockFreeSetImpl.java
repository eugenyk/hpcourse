import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * {@inheritDoc}
 *
 * @author belaevstanislav
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> head;
    private final Node<T> tail;

    LockFreeSetImpl() {
        this.head = new Node<>(null);
        this.tail = new Node<>(null);
        head.next.set(tail, false);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value);
        NodePair<T> pair;

        do {
            pair = search(value);
            if (pair.snd != tail && pair.snd.key.equals(value)) {
                return false;
            }

            newNode.next.set(pair.snd, false);
            if (pair.fst.next.compareAndSet(pair.snd, newNode, false, false)) {
                return true;
            }
        } while (true);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean remove(T value) {
        Node<T> sndNext;
        NodePair<T> pair;

        do {
            pair = search(value);
            if (pair.snd == tail || !pair.snd.key.equals(value))
                return false;
            sndNext = pair.snd.next.getReference();

            if (pair.snd.next.compareAndSet(sndNext, sndNext, false, true)) {
                break;
            }
        } while (true);

        if (!pair.fst.next.compareAndSet(pair.snd, sndNext, false, false)) {
            search(value);
        }
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean contains(T value) {
        Node<T> helper = head;
        do {
            helper = helper.next.getReference();
            if (helper == tail) {
                break;
            }
        } while (helper.next.isMarked() || helper.key == null || helper.key.compareTo(value) < 0);
        
        return (helper != tail && helper.key.equals(value) && !helper.next.isMarked());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isEmpty() {
        Node<T> helper = head;

        do {
            helper = helper.next.getReference();
            if (helper == tail) {
                return true;
            }

            if (!helper.next.isMarked()) {
                return false;
            } else {
                if (!head.next.compareAndSet(helper, helper.next.getReference(), false, false)) {
                    helper = head;
                }
            }
        } while (helper.next.isMarked() || helper.key == null);

        return true;
    }

    /**
     * Finds a pair of adjacent nodes. (l, r)
     *
     * @param value to search
     * @return pair
     */
    private NodePair<T> search(T value) {
        Node<T> fst = head;
        Node<T> fstNext = fst.next.getReference();
        Node<T> snd;

        do {
            Node<T> helper = head;

            do {
                if (!helper.next.isMarked()) {
                    fst = helper;
                    fstNext = fst.next.getReference();
                }

                helper = helper.next.getReference();
                if (helper == tail) {
                    break;
                }
            } while (helper.next.isMarked() || helper.key == null || helper.key.compareTo(value) < 0);
            snd = helper;

            // fst -> fstNext  == snd
            if (fstNext == snd) {
                if (snd.next.isMarked()) {
                    continue;
                } else {
                    return new NodePair<>(fst, snd);
                }
            }

            // removing nodes in between
            boolean ref = fst.next.isMarked();
            if (fst.next.compareAndSet(fstNext, snd, ref, ref)) {
                if (!snd.next.isMarked()) {
                    return new NodePair<>(fst, snd);
                }
            }
        } while (true);
    }

    private static class Node<U extends Comparable<U>> {
        private final U key;
        private AtomicMarkableReference<Node<U>> next;

        private Node(U key) {
            this.key = key;
            this.next = new AtomicMarkableReference<>(null, false);
        }
    }

    private static class NodePair<U extends Comparable<U>> {
        private final Node<U> fst;
        private final Node<U> snd;

        private NodePair(Node<U> fst, Node<U> snd) {
            this.fst = fst;
            this.snd = snd;
        }
    }
}
