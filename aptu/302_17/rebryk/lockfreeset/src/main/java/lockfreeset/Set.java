package lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Lock-free linked list
 *
 * Implemented according to the article https://www.microsoft.com/en-us/research/wp-content/uploads/2001/10/2001-disc.pdf
 *
 * @param <T>
 */
public class Set<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> head;
    private final Node<T> tail;

    public Set() {
        tail = new Node<>(null);
        head = new Node<>(null, tail);
    }

    public boolean add(T value) {
        final Node<T> newNode = new Node<>(value);

        do {
            Pair<Node<T>> result = search(value);

            if (result.right != tail && result.right.key.equals(value)) {
                return false; // found the value
            }

            newNode.next.set(result.right, false);

            // try to remove
            if (result.left.next.compareAndSet(result.right, newNode, false, false)) {
                return true;
            }
        } while (true);
    }

    public boolean remove(T value) {
        Pair<Node<T>> result;
        Node<T> rightNext;

        do {
            result = search(value);

            if (result.right == tail || !result.right.key.equals(value)) {
                return false; // nothing to remove
            }

            rightNext = result.right.next.getReference();

            // try to remove node logically
            if (result.right.next.attemptMark(rightNext, true)) {
                break;
            }
        } while (true);

        if (!result.left.next.compareAndSet(result.right, rightNext, false,false)) {
            search(result.right.key); // remove node physically
        }

        return true;
    }

    public boolean contains(T value) {
        Node<T> it = head;

        while (it != tail && (it.key == null || it.next.isMarked() || it.key.compareTo(value) < 0)) {
            it = it.next.getReference();
        }

        return it != tail && it.key.equals(value);
    }

    public boolean isEmpty() {
        while (head.next.getReference() != tail) {
            Node<T> next = head.next.getReference();

            if (!next.next.isMarked()) {
                return false;
            }

            head.next.compareAndSet(next, next.next.getReference(), false, false);
        }

        return true;
    }

    private Pair<Node<T>> search(T key) {
        Node<T> left = head;
        Node<T> right;

        do {
            Node<T> it = head;
            Node<T> left_next = null;

            // find left and right nodes such that:
            // 1) left.key < key <= right.key
            // 2) left and right nodes aren't marked
            while (it != tail && (it.key == null || it.next.isMarked() || it.key.compareTo(key) < 0)) {
                if (!it.next.isMarked()) {
                    left = it;
                    left_next = it.next.getReference();
                }

                it = it.next.getReference();
            }

            right = it;

            // check nodes are adjacent
            if (left_next == right) {
                if (right != tail && right.next.isMarked()) {
                    continue;
                }

                return new Pair<>(left, right);
            }

            // remove marked nodes between left and right nodes
            if (left.next.compareAndSet(left_next, right, false, false)) {
                if (right != tail && right.next.isMarked()) {
                    continue;
                }

                return new Pair<>(left, right);
            }

        } while (true);
    }


    /**
     * Class implements node of the lock-free linked list
     * Contains key and markable reference to the next node
     * @param <T>
     */
    private static class Node<T extends Comparable<T>> {
        private final T key;
        private AtomicMarkableReference<Node<T>> next;

        private Node(T key) {
            this(key, null);
        }

        private Node(T key, Node<T> next) {
            this.key = key;
            this.next = new AtomicMarkableReference<Node<T>>(next, false);
        }
    }

    /**
     * Auxiliary class to return a pair of nodes
     * @param <T>
     */
    private static class Pair<T> {
        private final T left;
        private final T right;

        private Pair(T first, T second) {
            this.left = first;
            this.right = second;
        }
    }
}