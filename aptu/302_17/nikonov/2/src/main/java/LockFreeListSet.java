import java.util.concurrent.atomic.AtomicReference;

public class LockFreeListSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> rootNode;
    private final Node<T> endNode;

    public LockFreeListSet() {
        endNode = new Node<>(null);
        rootNode = new Node<>(null);
        rootNode.next.set(new MarkedReference<>(endNode, false));
    }

    @Override
    public boolean add(final T value) {
        removeOldNodes();
        Node<T> newNode = new Node<>(value);
        while (true) {
            MarkedReference<T> reference = rootNode.next.get();
            if (contains(value)) { // if set contains unmarked node with value
                return false;
            }
            newNode.next = new AtomicReference<>(reference);
            if (rootNode.next.compareAndSet(reference, new MarkedReference<>(newNode, false))) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(final T value) {
        boolean res = findAndMark(value);
        removeOldNodes();
        return res;
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
        MarkedReference<T> reference = rootNode.next.get();
        Node<T> curNode = rootNode.next.get().node;
        while (curNode != endNode) {
            if (!curNode.isMarked()) {
                return false;
            }
            curNode = curNode.next.get().node;
        }
        return rootNode.next.get() == reference;
    }

    private boolean findAndMark(final T value) {
        while (true) {
            removeOldNodes();
            Node<T> curNode = rootNode.next.get().node;
            while (curNode != endNode && !curNode.value.equals(value)) {
                curNode = curNode.next.get().node;
            }
            if (curNode == endNode) {
                return false;
            }
            MarkedReference<T> reference = curNode.next.get();
            if (reference.isMarked) {
                continue;
            }

            MarkedReference<T> newNextReference = new MarkedReference<>(reference.node, true);
            if (curNode.next.compareAndSet(reference, newNextReference)) {
                return true;
            }
        }
    }

    private void removeOldNodes() {
        Node<T> curNode = rootNode;
        while (curNode != endNode) {
            MarkedReference<T> reference = curNode.next.get();
            if (reference.node == endNode) return;
            if (!reference.isMarked && reference.node.isMarked()) { // Marked references meaning
                Node<T> targetNode = reference.node;
                curNode.next.compareAndSet(reference, new MarkedReference<>(targetNode.next.get().node, false));
            } else {
                curNode = curNode.next.get().node;
            }
        }
    }

    private static class MarkedReference<T extends Comparable<T>> {
        final Node<T> node;
        final boolean isMarked;
        MarkedReference(final Node<T> node, final boolean isMarked) {
            this.node = node;
            this.isMarked = isMarked;
        }
    }

    private static class Node<T extends Comparable<T>> {
        volatile AtomicReference<MarkedReference<T>> next = new AtomicReference<>(null);
        final T value;
        Node(final T value) {
            this.value = value;
        }
        boolean isMarked() {
            return next.get().isMarked;
        }
    }
}
