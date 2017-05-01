public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private class Node {
        // 'next' is marked iff all of the following are true:
        // 1. 'next' if the node is not going to change.
        // 2. The node is going to be deleted.
        // Invariant: no two consecutive nodes have 'next' marked.
        final AtomicMarkableReferenceWithWaitFreeGet<Node> next;
        final T data;

        private Node(T data) {
            this.next = new AtomicMarkableReferenceWithWaitFreeGet<>(null, false);
            this.data = data;
        }
    }

    private Node head;

    LockFreeSetImpl() {
        head = new Node(null);
    }

    public boolean add(T value) {
        Node[] nodes = (Node[])new LockFreeSetImpl.Node[2];
        Node newn = new Node(value);
        while (true) {
            search(value, nodes);
            if (nodes[1] != null) {
                return false;
            }
            if (!nodes[0].next.compareAndSet(null, newn, false, false)) {
                continue;  // Conflict, try again
            }
            return true;
        }
    }

    public boolean remove(T value) {
        boolean[] mark = new boolean[1];
        Node[] nodes = (Node[])new LockFreeSetImpl.Node[2];
        boolean result = false;
        while (true) {
            search(value, nodes);
            if (nodes[1] == null) {
                return result;
            }
            Node next = nodes[1].next.get(mark);
            if (!nodes[1].next.compareAndSet(next, next, false, true)) {
                continue; // Conflict, try again
            }
            result = true;
            if (!nodes[0].next.compareAndSet(nodes[1], next, false, false)) {
                continue; // Conflict, try again
            }
            return result;
        }
    }

    public boolean contains(T value) {
        Node[] nodes = (Node[])new LockFreeSetImpl.Node[2];
        search(value, nodes);
        return nodes[1] != null;
    }

    /**
     * Looks for <code>value</code> in the list. Returns corresponding node in <code>nodes[1]</code>
     * and its previous node in <code>nodes[0]</code>, if found. If <code>value</code> is not found,
     * returns last element of the list in <code>nodes[0]</code> and <code>null</code> in <code>nodes[1]</code>.
     *
     * Lock-free.
     */
    private void search(T value, Node[] nodes) {
        assert nodes.length == 2;
        while (true) {
            Node prevPrev = null;
            Node prev = head;
            boolean[] mark = new boolean[1];
            while (true) {
                Node cur = prev.next.get(mark);
                if (mark[0]) {
                    if (!prevPrev.next.compareAndSet(prev, cur, false, false)) {
                        break;  // Conflict, 'cur' was concurrently deleted.
                    }
                    prev = prevPrev;
                    prevPrev = null;
                }
                if (cur == null) {
                    nodes[0] = prev;
                    nodes[1] = null;
                    return;
                }
                if (cur.data.equals(value)) {
                    nodes[0] = prev;
                    nodes[1] = cur;
                    return;
                }
                prevPrev = prev;
                prev = cur;
            }
        }
    }

    public boolean isEmpty() {
        boolean[] mark = new boolean[1];
        return head.next.get(mark) == null;
    }
}