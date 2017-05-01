import java.util.concurrent.atomic.AtomicReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private class Node {
        final AtomicReference<Node> next;
        final T data;

        private Node(T data) {
            this.next = new AtomicReference<>();
            this.data = data;
        }
    }

    private Node root;

    LockFreeSetImpl() {
        root = new Node(null);
    }

    public boolean add(T value) {
        Node newn = new Node(value);

        Node prev = root;
        while (true) {
            Node cur = prev.next.get();
            if (cur == null) {
                if (prev.next.compareAndSet(null, newn)) {
                    return true;
                } else {
                    continue;
                }
            }
            if (cur.data.equals(value)) {
                return false;
            }
            prev = cur;
        }
    }

    public boolean remove(T value) {
        Node prev = root;
        while (true) {
            Node cur = prev.next.get();
            if (cur == null) {
                return false;
            }
            if (cur.data.equals(value)) {
                Node next = cur.next.get();
                if (prev.next.compareAndSet(cur, next)) {
                    return true;
                } else {
                    continue;
                }
            }
            prev = cur;
        }
    }

    public boolean contains(T value) {
        Node prev = root;
        while (true) {
            Node cur = prev.next.get();
            if (cur == null) {
                return false;
            }
            if (cur.data.equals(value)) {
                return true;
            }
            prev = cur;
        }
    }

    public boolean isEmpty() {
        return root.next.get() == null;
    }
}