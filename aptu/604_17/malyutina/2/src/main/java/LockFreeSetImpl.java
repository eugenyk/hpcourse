import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Created by kate on 07.10.17.
 */

/**
 * Harris algo
 * Так как нас по Java просили реализовать Lock-Free List исходя из этого алгоритма,
 * поэтому сделала по это статье
 * https://timharris.uk/papers/2001-disc.pdf
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node tail = new Node(null, new AtomicMarkableReference<>(null, false));
    private final Node head = new Node(null, new AtomicMarkableReference<>(tail, false));

    @Override
    public boolean add(final T value) {
        Node insertNode = new Node(value, new AtomicMarkableReference<>(null, false));

        while (true) {
            Pair position = findPositionNode(value);
            if (position.rightNode != tail && position.rightNode.value.equals(value)) {
                return false;
            }
            insertNode.next.set(position.rightNode, false);
            if (position.leftNode.next.compareAndSet(position.rightNode, insertNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(final T value) {
        Pair position;
        Node right;

        while (true) {
            position = findPositionNode(value);
            if (position.rightNode == tail || !position.rightNode.value.equals(value)) {
                return false;
            }
            right = position.rightNode.next.getReference();
            if (position.rightNode.next.attemptMark(right, true)) {
                break;
            }
        }

        if (position.leftNode.next.compareAndSet(position.rightNode, right, false, false)) {
            findPositionNode(position.rightNode.value);
        }
        return true;
    }

    @Override
    public boolean contains(final T value) {
        Pair position = findPositionNode(value);
        if (position.rightNode == tail || !position.rightNode.value.equals(value)) {
            return false;
        }
        return true;
    }

    @Override
    public boolean isEmpty() {
        while (head.next.getReference() != tail) {
            Node next = head.next.getReference();
            if (!head.next.isMarked()) {
                return false;
            }
            head.next.compareAndSet(next, next.next.getReference(), false,
                    false);
        }
        return true;
    }

    private Pair findPositionNode(final T value) {
        Node leftNode = head;
        Node rightNode;

        while (true) {
            Node prev = head;
            Node next = null;

            /* 1: Find left_node and right_node */
            while (prev != tail &&
                    (prev.next.isMarked() || prev.value == null || prev.value.compareTo(value) < 0)) {
                if (!prev.next.isMarked()) {
                    leftNode = prev;
                    next = prev.next.getReference();
                }
                prev = prev.next.getReference();
            }
            rightNode = prev;

            /* 2: Check nodes are adjacent */
            if (leftNode == rightNode) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                }
                return new Pair(leftNode, rightNode);
            }
            /* 3: Remove one or more marked nodes */
            if (leftNode.next.compareAndSet(next, rightNode, false, false)) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                }
                return new Pair(leftNode, rightNode);
            }
        }
    }

    private final class Node {
        private final T value;
        private final AtomicMarkableReference<Node> next;

        public Node(final T value, final AtomicMarkableReference<Node> next) {
            this.value = value;
            this.next = next;
        }
    }

    private final class Pair {
        private final Node leftNode;
        private final Node rightNode;

        public Pair(final Node leftNode, final Node rightNode) {
            this.leftNode = leftNode;
            this.rightNode = rightNode;
        }
    }
}
