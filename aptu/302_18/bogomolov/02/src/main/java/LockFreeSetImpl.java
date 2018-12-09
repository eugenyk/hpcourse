import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private class Node {

        final AtomicMarkableReference<Node> markedNext = new AtomicMarkableReference<>(null, false);
        T key;

        Node(T key) {
            this.key = key;
            markedNext.set(null, false);
        }

        Node getNext() {
            return markedNext.getReference();
        }

        boolean isMarked() {
            return markedNext.isMarked();
        }

    }

    private class NodePair {

        final Node first, second;

        NodePair(Node f, Node s) {
            first = f;
            second = s;
        }
    }

    private final Node head;
    private final Node tail;

    LockFreeSetImpl() {
        head = new Node(null);
        tail = new Node(null);
        head.markedNext.set(tail, false);
    }


    @Override
    public boolean add(T value) {
        Node newNode = new Node(value);
        while (true) {
            NodePair nodes = find(value);
            Node left = nodes.first, right = nodes.second;
            if (right != tail && right.key.equals(value)) {
                return false;
            }
            newNode.markedNext.set(right, false);
            if (left.markedNext.compareAndSet(right, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        Node left, right, next;
        while (true) {
            NodePair nodes = find(value);
            left = nodes.first;
            right = nodes.second;
            if (right == tail || !right.key.equals(value)) return false;
            next = right.getNext();
            if (right.markedNext.attemptMark(next, true)) break;
        }
        if (!left.markedNext.compareAndSet(right, next, false, false)) {
            find(right.key);
        }
        return true;
    }

    @Override
    public boolean contains(T value) {
        Node cur = head.getNext();
        while (cur != tail && (cur.isMarked() || value.compareTo(cur.key) > 0)) {
            cur = cur.getNext();
        }
        return cur != tail && value.equals(cur.key);
    }

    @Override
    public boolean isEmpty() {
        // clear marked nodes after head
        while (head.getNext() != tail) {
            Node next = head.getNext();
            if (!next.isMarked()) return false;
            head.markedNext.compareAndSet(next, next.getNext(), false, false);
        }
        return true;
    }

    private NodePair find(T value) {
        Node left = head, right = null;
        while (true) {
            Node cur = head, next = null;
            // find first non-marked node with greater or equal key
            while (cur != tail && (cur == head || cur.isMarked() || cur.key.compareTo(value) < 0)) {
                if (!cur.isMarked()) {
                    left = cur;
                    next = cur.getNext();
                }
                cur = cur.getNext();
            }

            right = cur;
            // right is immediate successor of left
            if (next == right) {
                if (right != tail && right.isMarked()) continue;
                return new NodePair(left, right);
            }
            // try to remove all marked nodes between left and right
            if (left.markedNext.compareAndSet(next, right, false, false)) {
                if (right != tail && right.isMarked()) continue;
                return new NodePair(left, right);
            }

        }
    }
}
