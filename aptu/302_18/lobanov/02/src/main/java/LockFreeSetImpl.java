import java.util.Random;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;

import static java.util.concurrent.atomic.AtomicReferenceFieldUpdater.newUpdater;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Random random = new Random(239566);
    private static final AtomicReferenceFieldUpdater UPDATER =
            newUpdater(LockFreeSetImpl.class, Node.class, "root");
    private volatile Node<T> root = null;

    @Override
    public boolean add(T value) {
        final int y = random.nextInt();
        while (true) {
            final Node currentRoot = root;
            final Node result = insert(root, value, y);
            if (result == currentRoot) {
                return false;
            }
            //noinspection unchecked
            if (UPDATER.compareAndSet(this, currentRoot, result)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            final Node currentRoot = root;
            final Node result = remove(root, value);
            if (result == currentRoot) {
                return false;
            }
            //noinspection unchecked
            if (UPDATER.compareAndSet(this, currentRoot, result)) {
                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        return contains(root, value);
    }

    @Override
    public boolean isEmpty() {
        return root == null;
    }

    //----------Treap-implementation-----------//

    private Node<T> insert(Node<T> root, T value, int y) {
        if (root == null) {
            return new Node<>(null, null, value, y);
        }
        final int cmp = value.compareTo(root.value);
        if (cmp == 0) {
            return root;
        } else if (root.y < y) {
            final Pair<T> pair = split(root, value);
            final T maxLeft = maxElement(pair.left);
            if (maxLeft != null && maxLeft.compareTo(value) == 0) {
                return root;
            }
            return new Node<>(pair.left, pair.right, value, y);
        } else if (cmp > 0) {
            final Node<T> right = insert(root.right, value, y);
            return right == root.right ? root : new Node<>(root.left, right, root.value, root.y);
        } else {
            final Node<T> left = insert(root.left, value, y);
            return left == root.left ? root : new Node<>(left, root.right, root.value, root.y);
        }
    }

    private Node<T> remove(Node<T> root, T value) {
        if (root == null) {
            return null;
        }
        final int cmp = value.compareTo(root.value);
        if (cmp == 0) {
            return merge(root.left, root.right);
        } else if (cmp > 0) {
            final Node<T> right = remove(root.right, value);
            return right == root.right ? root : new Node<>(root.left, right, root.value, root.y);
        } else {
            final Node<T> left = remove(root.left, value);
            return left == root.left ? root : new Node<>(left, root.right, root.value, root.y);
        }
    }

    private boolean contains(Node<T> root, T value) {
        if (root == null) {
            return false;
        }
        final int cmp = value.compareTo(root.value);
        return cmp == 0 || (cmp < 0 ? contains(root.left, value) : contains(root.right, value));
    }

    private T maxElement(Node<T> root) {
        if (root == null) {
            return null;
        }
        return root.right == null ? root.value : maxElement(root.right);
    }

    private Node<T> merge(Node<T> left, Node<T> right) {
        if (left == null || right == null) {
            return right == null ? left : right;
        } else if (left.y > right.y) {
            return new Node<>(left.left, merge(left.right, right), left.value, left.y);
        } else {
            return new Node<>(merge(left, right.left), right.right, right.value, right.y);
        }
    }

    private Pair<T> split(Node<T> node, T key) {
        if (node == null) {
            return new Pair<>();
        } else if (key.compareTo(node.value) >= 0) {
            final Pair<T> pair = split(node.right, key);
            final Node<T> left = new Node<>(node.left, pair.left, node.value, node.y);
            return new Pair<>(left, pair.right);
        } else {
            final Pair<T> pair = split(node.left, key);
            final Node<T> right = new Node<>(pair.right, node.right, node.value, node.y);
            return new Pair<>(pair.left, right);
        }
    }

    private static class Node<T> {
        private final Node<T> left;
        private final Node<T> right;
        private final T value;
        private final int y;

        private Node(Node<T> left, Node<T> right, T value, int y) {
            this.left = left;
            this.right = right;
            this.value = value;
            this.y = y;
        }
    }

    private static class Pair<T> {
        private final Node<T> left;
        private final Node<T> right;

        private Pair(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }

        private Pair() {
            this.left = null;
            this.right = null;
        }
    }
}
