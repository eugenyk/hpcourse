import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private class Node {
        // 'next' is marked iff this node is going to be deleted from the list soon.
        final AtomicMarkableReference<Node> next;
        final T data;

        private Node(T data) {
            this.next = new AtomicMarkableReference<>(null, false);
            this.data = data;
        }
    }

    private Node root;

    LockFreeSetImpl() {
        root = new Node(null);
    }

    public boolean add(T value) {
        Node newn = new Node(value);

        while (true) {
            Node lastUnmarked = null, lastUnmarkedNext = null;
            Node prev = root;
            boolean[] mark = new boolean[1];
            while (true) {
                Node cur = prev.next.get(mark);
                if (!mark[0]) {
                    lastUnmarked = prev;
                    lastUnmarkedNext = cur;
                }
                if (cur == null) {
                    if (!lastUnmarked.next.compareAndSet(lastUnmarkedNext, newn, false, false)) {
                        break;  // Conflict, try again
                    }
                    return true;
                }
                if (cur.data.equals(value)) {
                    return false;
                }
                prev = cur;
            }
        }
    }

    public boolean remove(T value) {
        while (true) {
            Node lastUnmarked = null, lastUnmarkedNext = null;
            Node prev = root;
            boolean[] mark = new boolean[1];
            while (true) {
                Node cur = prev.next.get(mark);
                if (!mark[0]) {
                    lastUnmarked = prev;
                    lastUnmarkedNext = cur;
                }

                if (cur == null) {
                    return false;
                }
                if (cur.data.equals(value)) {
                    Node next = cur.next.get(mark);
                    // Do not check mark[0]: even if it's true, then somebody is already trying do
                    // delete the node - let's help them.
                    if (!cur.next.attemptMark(next, true)) {
                        break;  // Conflict, try again.
                    }
                    if (!lastUnmarked.next.compareAndSet(lastUnmarkedNext, next, false, false)) {
                        break; // Conflict, try again
                    }
                    // Nobody inserted an element right after us.
                    assert cur.next.get(mark) == next;
                    assert mark[0];
                    return true;
                }
                prev = cur;
            }
        }
    }

    public boolean contains(T value) {
        Node prev = root;
        boolean[] mark = new boolean[1];
        while (true) {
            Node cur = prev.next.get(mark);
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
        boolean[] mark = new boolean[1];
        return root.next.get(mark) == null;
    }
}