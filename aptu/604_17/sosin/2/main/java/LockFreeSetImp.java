import java.util.concurrent.atomic.AtomicMarkableReference;


public class LockFreeSetImp<T extends Comparable<T>> implements LockFreeSet<T> {

    private class Node implements Comparable<Node> {
        T value;
        AtomicMarkableReference<Node> next;

        Node(T value) {
            this.value = value;
            next = new AtomicMarkableReference<>(null, false);
        }

        Node(T value, Node next) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        @Override
        public int compareTo(Node node) {
            if (value == null || node.value == null) {
                return -1;
            }
            return value.compareTo(node.value);
        }
    }

    private class Pair {
        Node first;
        Node second;

        Pair(Node pred, Node curr) {
            this.first = pred;
            this.second = curr;
        }
    }

    private Node tail;
    private Node head;

    public LockFreeSetImp() {
        tail = new Node(null);
        head = new Node(null, tail);
    }

    Pair search(T value) {
        Node left_node = head;
        Node left_node_next = null;
        Node right_node;

        do {
            Node temporal_node = head;

            // find left_node and right_node
            do {
                if (!temporal_node.next.isMarked()) {
                    left_node = temporal_node;
                    left_node_next = left_node.next.getReference();
                }

                temporal_node = temporal_node.next.getReference();

                if (temporal_node == tail) {
                    break;
                }

            } while (temporal_node.next.isMarked() || (temporal_node.value.compareTo(value) < 0));

            right_node = temporal_node;

            // Check nodes are adjacent
            if (left_node_next == right_node) {
                if (right_node != tail && right_node.next.isMarked()) {
                    continue;
                }
                return new Pair(left_node, right_node);
            }

            // remove one or more marked nodes
            if (left_node.next.compareAndSet(left_node_next, right_node, false, false)) {
                if (right_node != tail && right_node.next.isMarked()) {
                    continue;
                }
                return new Pair(left_node, right_node);
            }

        } while (true);
    }

    @Override
    public boolean add(T value) {
        Node node = new Node(value);
        Node left_node = null;
        Node right_node = null;

        do {
            Pair window = search(value);
            left_node = window.first;
            right_node = window.second;

            if (right_node != tail && right_node.value.compareTo(value) == 0) {
                return false;
            }

            node.next = new AtomicMarkableReference<>(right_node, false);

            if (left_node.next.compareAndSet(right_node, node, false, false)) {
                return true;
            }
        } while (true);
    }

    @Override
    public boolean remove(T value) {
        Node left_node;
        Node right_node;
        Node right_node_next;

        do {
            Pair window = search(value);

            left_node = window.first;
            right_node = window.second;

            if (right_node == tail || right_node.value.compareTo(value) != 0) {
                return false;
            }

            right_node_next = right_node.next.getReference();
            if (right_node.next.attemptMark(right_node_next, true)) {
                break;
            }

        } while (true);

        if (!left_node.next.compareAndSet(right_node, right_node_next, false, false)) {
            search(right_node.value);
        }
        return true;

    }

    @Override
    public boolean contains(T value) {
        Pair window = search(value);

        Node right_node = window.second;

        if (right_node == tail || right_node.value.compareTo(value) != 0) {
            return false;
        }
        return true;
    }

    @Override
    public boolean isEmpty() {
        Node next_node = head.next.getReference();

        while (next_node != tail) {
            if (!next_node.next.isMarked()) {
                return false;
            }
            next_node = next_node.next.getReference();
        }

        return true;
    }
}

