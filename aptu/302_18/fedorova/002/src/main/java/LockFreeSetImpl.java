import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> head;
    private final Node<T> tail;

    public LockFreeSetImpl() {
        head = new Node<T>(null);
        tail = new Node<T>(null);
        head.next.set(tail, false);
    }

    public boolean add(T value) {
        Node<T> new_node = new Node<T>(value);
        Node<T> right_node, left_node;
        NodePair<T> node_pair;
        do {
            node_pair = search(value);
            right_node = node_pair.right;
            left_node = node_pair.left;
            if ((right_node != tail) && right_node.getKey().equals(value))
                return false;
            new_node.next.set(right_node, false);
            if (left_node.next.compareAndSet(right_node, new_node, false, false))
                return true;
        } while (true);
    }

    public boolean remove(T value) {
        Node<T> right_node, left_node;
        NodePair<T> node_pair;
        Node<T> right_next;
        do {
            node_pair = search(value);
            right_node = node_pair.right;
            left_node = node_pair.left;
            if ((right_node == tail) || !right_node.getKey().equals(value))
                return false;
            right_next = right_node.next.getReference();
            if (right_node.next.compareAndSet(right_next, right_next, false, true))
                break;
        } while (true);

        if (!left_node.next.compareAndSet(right_node, right_next, false, false)) {
            search(value);
        }
        return true;
    }

    public boolean contains(T value) {
        NodePair<T> node_pair;
        node_pair = search(value);
        Node<T> right_node = node_pair.right;
        return ((right_node != tail) && (right_node.getKey().equals(value)));
    }

    public boolean isEmpty() {
        Node<T> helper = head;
        Node<T> helper_next;
        do{
            helper_next = helper.next.getReference();
            if (helper_next == tail)
                return true;
            if (!helper.next.isMarked()) {
                return false;
            } else {
                if (!head.next.compareAndSet(helper, helper.next.getReference(), false, false))
                    helper = head;
            }
        } while (helper.next.isMarked() || helper.getKey() == null);
        return true;
    }

    private class Node<U extends Comparable<U>> {
        final U key;
        AtomicMarkableReference<Node<U>> next;
        Node(U key) {
            this.key = key;
            next = new AtomicMarkableReference<Node<U>>(null, false);
        }
        U getKey() {
            return key;
        }
    }
    
    private NodePair<T> search(T value) {
        Node<T> left = head;
        Node<T> left_next = left.next.getReference();
        Node<T> right;

        do {
            Node<T> helper = head;
            /* 1: Find left and right unmarked node*/
            do {
                if (!helper.next.isMarked()) {
                    left = helper;
                    left_next = left.next.getReference();
                }
                helper = helper.next.getReference();
                if (helper == tail) {
                    break;
                }
            } while (helper.next.isMarked() || helper.getKey() == null || helper.getKey().compareTo(value) < 0);
            right = helper;

            /* 2: Check nodes are adjacent */
            if (left_next == right) {
                if (right.next.isMarked()) {
                    continue;
                } else {
                    return new NodePair<T>(left, right);
                }
            }

            /* 3: Remove one or more marked nodes */
            boolean ref = left.next.isMarked();
            if (left.next.compareAndSet(left_next, right, ref, ref)) {
                if (!right.next.isMarked()) {
                    return new NodePair<T>(left, right);
                }
            }
        } while (true);
    }

    private class NodePair<W extends Comparable<W>> {
        private final Node<W> left;
        private final Node<W> right;

        private NodePair(Node<W> left, Node<W> right) {
            this.left = left;
            this.right = right;
        }
    }
}
