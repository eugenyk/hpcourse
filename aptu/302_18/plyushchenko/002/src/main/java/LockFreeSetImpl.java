import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final Node<T> head;
    private final Node<T> tail;

    public LockFreeSetImpl() {
        tail = new Node<>(null, new AtomicMarkableReference<>(null, false));
        head = new Node<>(null, new AtomicMarkableReference<>(tail, false));
    }

    @Override
    public boolean add(T value) {
        Node<T> leftNode;
        Node<T> rightNode;
        do {
            Pair<Node<T>, Node<T>> tmp = search(value);
            leftNode = tmp.left;
            rightNode = tmp.right;
            if (rightNode != tail &&  rightNode.value.equals(value)) {
                return false;
            }
            Node <T> newNode = new Node<>(value, new AtomicMarkableReference<>(rightNode, false));
            if (leftNode.next.compareAndSet(rightNode, newNode, false, false)) {
                return true;
            }

        } while (true);
    }

    @Override
    public boolean remove(T value) {
        Node<T> rightNode;
        Node<T> rightNodeNext;
        Node<T> leftNode;
        do {
            Pair<Node<T>, Node<T>> tmp = search(value);
            leftNode = tmp.left;
            rightNode = tmp.right;
            if (rightNode == tail || !rightNode.value.equals(value)) {
                return false;
            }
            boolean[] markHolder = new boolean[1];
            rightNodeNext = rightNode.next.get(markHolder);
            if (!markHolder[0]) {
                if (rightNode.next.compareAndSet(rightNodeNext, rightNodeNext, false, true)) {
                    break;
                }
            }
        } while (true);
        if (!leftNode.next.compareAndSet(rightNode, rightNodeNext, false, false)) {
            search(value);
        }
        return true;
    }

    @Override
    public boolean contains(T value) {
        Node<T> rightNode;
        Pair<Node<T>, Node<T>> tmp = search(value);
        rightNode = tmp.right;
        return rightNode != tail && rightNode.value.equals(value);

    }

    @Override
    public boolean isEmpty() {
        Node<T> first = head.next.getReference();
        while (first != tail && first.next.isMarked()) {
            Node<T> next = first.next.getReference();
            head.next.compareAndSet(first, next, false, false);
            first = head.next.getReference();
        }
        return first == tail;
    }

    private Pair<Node<T>, Node<T>> search(T value) {
        Node<T> leftNodeNext = null;
        Node<T> leftNode = null;
        Node<T> rightNode;
        do {
            Node<T> t = head;
            boolean[] markHolder = new boolean[1];
            Node<T> tNext = head.next.get(markHolder);
            do {
                if (!markHolder[0]) {
                    leftNode = t;
                    leftNodeNext = tNext;
                }
                t = tNext;
                if (t == tail) {
                    break;
                }
                tNext = t.next.get(markHolder);
            } while (markHolder[0] || t.value.compareTo(value) < 0);
            rightNode = t;
            if (leftNodeNext == rightNode) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                }
                return new Pair<>(leftNode, rightNode);
            }
            if (leftNode != null && leftNode.next.compareAndSet(leftNodeNext, rightNode, false, false)) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                }
                return new Pair<>(leftNode, rightNode);
            }
        } while (true);
    }

    private static class Node<T> {
        private T value;
        private AtomicMarkableReference<Node<T>> next;

        Node(T value, AtomicMarkableReference<Node<T>> next) {
            this.value = value;
            this.next = next;
        }
    }

    private static class Pair<T, U> {
        T left;
        U right;

        Pair(T left, U right) {
            this.left = left;
            this.right = right;
        }
    }
}
