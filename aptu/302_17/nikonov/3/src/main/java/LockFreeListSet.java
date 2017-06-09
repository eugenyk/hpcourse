import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

public class LockFreeListSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private volatile AtomicInteger size = new AtomicInteger(0);
    private final Node<T> rootNode;
    private final Node<T> endNode;

    @Override
    public boolean add(final T value) {
        Node<T> newNode = new Node<>(value);
        while (true) {
            MarkedReference<T> reference = rootNode.next.get();
            if (contains(value)) { // if set contains unmarked node with value
                return false;
            }
            newNode.next = new AtomicReference<>(reference);
            if (rootNode.next.compareAndSet(reference, new MarkedReference<>(newNode, false))) {
                size.incrementAndGet();
                return true;
            }
        }
    }

    // Invariant: remove method marks and deletes the same node.
    @Override
    public boolean remove(final T value) {
        Node<T> removingNode = findAndMark(value);
        if (removingNode == null) { // value not found
            return false;
        }
        while (true) {
            Node<T> curNode = rootNode;
            while(curNode.next.get().node != removingNode) {
                curNode = curNode.next.get().node;
            }
            MarkedReference<T> reference = curNode.next.get();
            if (reference.isMarked) {
                continue;
            }
            if (curNode.next.compareAndSet(reference, new MarkedReference<>(removingNode.next.get().node, false))) {
                size.decrementAndGet();
                return true;
            }
        }
    }

    @Override
    public boolean contains(final T value) {
        Node<T> curNode = rootNode.next.get().node;
        while (curNode != endNode) {
            if (curNode.value.equals(value) && !curNode.isMarked()) {
                return true;
            }
            curNode = curNode.next.get().node;
        }
        return false;
    }

    @Override
    public boolean isEmpty() {
        return size.get() == 0;
    }

    public LockFreeListSet() {
        endNode = new Node<>(null);
        rootNode = new Node<>(null);
        rootNode.next.set(new MarkedReference<>(endNode, false));
    }

    private Node<T> findAndMark(final T value) {
        while (true) {
            Node<T> curNode = rootNode;
            MarkedReference<T> reference = curNode.next.get();
            while (reference.node != endNode && !reference.node.value.equals(value)) {
                curNode = reference.node;
                reference = curNode.next.get();
            }
            if (reference.node == endNode) {
                return null;
            }
            Node<T> nextNode = reference.node;
            MarkedReference<T> nextReference = nextNode.next.get();
            if (reference.isMarked || nextReference.isMarked) {
                continue;
            }
            MarkedReference<T> newNextReference = new MarkedReference<>(nextReference.node, true);
            if (nextNode.next.compareAndSet(nextReference, newNextReference)) {
                return nextNode;
            }
        }
    }

    private static class MarkedReference<T extends Comparable<T>> {
        public final Node<T> node;
        public final boolean isMarked;
        public MarkedReference(final Node<T> node, final boolean isMarked) {
            this.node = node;
            this.isMarked = isMarked;
        }
    }

    private static class Node<T extends Comparable<T>> {
        public volatile AtomicReference<MarkedReference<T>> next = new AtomicReference<>(null);
        public final T value;
        public Node(final T value) {
            this.value = value;
        }
        public boolean isMarked() {
            return next.get().isMarked;
        }
    }
}
