package lockfree_set;

public class MySet<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> head;
    private final Node<T> tail;

    public MySet() {
        head = new Node<T>(null);
        tail = new Node<T>(null);
        head.next.set(tail, false);
    }

    /**
     * {@inheritDoc}
     */
    public boolean add(T value) {
        Node<T> newNode = new Node<T>(value);
        MyPair<T> pair;

        do {
            pair = search(value);
            if ((pair.right != tail) && pair.right.getKey().equals(value)) {
                return false;
            }
            newNode.next.set(pair.right, false);
            if (pair.left.next.compareAndSet(pair.right, newNode, false, false)) {
                return true;
            }
        } while (true);
    }

    /**
     * {@inheritDoc}
     */
    public boolean remove(T value) {
        Node<T> rightNext;
        MyPair<T> pair;

        do {
            pair = search(value);
            if (pair.right == tail || !pair.right.getKey().equals(value))
                return false;
            rightNext = pair.right.next.getReference();
            if (pair.right.next.compareAndSet(rightNext, rightNext, false, true)) {
                break;
            }
        } while (true);

        //boolean ref = pair.left.next.isMarked();
        if (!pair.left.next.compareAndSet(pair.right, rightNext, false, false)) {
            search(value); // see search -- it also does removes
        }
        return true;
    }

    /**
     * {@inheritDoc}
     */
    public boolean contains(T value) {
        Node<T> helper = head;
        do {
            helper = helper.next.getReference();
            if (helper == tail) {
                break;
            }
        } while (helper.next.isMarked() || helper.getKey() == null || helper.getKey().compareTo(value) < 0);
        return (helper != tail && helper.getKey().equals(value) && !helper.next.isMarked());
    }

    /**
     * {@inheritDoc}
     */
    public boolean isEmpty() {
        Node<T> helper = head;
        do {
            helper = helper.next.getReference();
            if (helper == tail) {
                break;
            }
            if (!helper.next.isMarked()) {
                return false;
            }
        } while (true);
        return true;
    }

    /**
     * finds a pair of adjacent nodes (left, right) so that
     * left.getKey() <= value <= right.getKey()
     * @param value
     * @return
     */
    private MyPair<T> search(T value) {
        Node<T> left = head;
        Node<T> leftNext = null;
        Node<T> right;

        do {
            Node<T> helper = head;

            do {
                if (!helper.next.isMarked()) {
                    left = helper;
                    leftNext = left.next.getReference();
                }
                helper = helper.next.getReference();
                if (helper == tail) {
                    break;
                }
            } while (helper.next.isMarked() || helper.getKey() == null || helper.getKey().compareTo(value) < 0);
            right = helper;
            // key should be between left and right

            // check if left and right are adjacent
            if (leftNext == right) {
                if (right.next.isMarked()) {
                    continue;
                } else {
                    return new MyPair<T>(left, right);
                }
            }

            // leftNext != right => leftNext is marked and we can try remove it
            boolean ref = left.next.isMarked();
            if (left.next.compareAndSet(leftNext, right, ref, ref)) {
                if (right.next.isMarked()) {
                    continue;
                } else {
                    return new MyPair<T>(left, right);
                }
            }
        } while (true);
    }

    private static class MyPair<T extends Comparable<T>> {
        private final Node<T> left;
        private final Node<T> right;

        private MyPair(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }
    }

}
