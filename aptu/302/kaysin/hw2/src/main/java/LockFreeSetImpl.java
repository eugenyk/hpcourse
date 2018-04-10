
import java.util.concurrent.atomic.AtomicMarkableReference;


/**
 * This is an implementation of Lock Free set based on Lock Free List described by
 * the following paper: https://www.cl.cam.ac.uk/research/srg/netos/papers/2001-caslists.pdf
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private static class Node<T> {
        private T value;
        private AtomicMarkableReference<Node<T>> next;

        private Node() {
        }

        private Node(T value) {
            this.value = value;
        }
    }

    private Node<T> head;
    private Node<T> tail;

    public LockFreeSetImpl() {
           tail = new Node<>();
           head = new Node<>();
           head.next = new AtomicMarkableReference<>(tail, false);
    }

    private static class PairOfNodes<T> {
        private Node<T> first;
        private Node<T> second;

        private PairOfNodes(Node<T> first, Node<T> second) {
            this.first = first;
            this.second = second;
        }
    }

    private PairOfNodes<T> search(T value) {

        Node<T> leftNode = head;
        Node<T> leftNodeNext = head;
        Node<T> rightNode;

        while (true) {
            Node<T> t = head;
            AtomicMarkableReference<Node<T>> tNext = head.next;

            do {
                if (!tNext.isMarked()) {
                    leftNode = t;
                    leftNodeNext = tNext.getReference();
                }
                t = tNext.getReference();
                if (t == tail) {
                    break;
                }
                tNext = t.next;
            } while (tNext.isMarked() || t.value.compareTo(value) < 0);
            rightNode = t;

            if (leftNodeNext == rightNode) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                } else {
                    return new PairOfNodes<>(leftNode, rightNode);
                }
            }

            if (leftNode.next.compareAndSet(leftNodeNext, rightNode, false, false)) {
                if (rightNode != tail && rightNode.next.isMarked()) {
                    continue;
                }
                return new PairOfNodes<>(leftNode, rightNode);
            }
        }
    }

    /**
     * Time complexity: O(n)
     */
    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value);
        Node<T> rightNode;
        Node<T> leftNode;
        while (true) {
            PairOfNodes<T> leftRightNodes = search(value);
            leftNode = leftRightNodes.first;
            rightNode = leftRightNodes.second;

            if (rightNode != tail && rightNode.value == value) {
                return false;
            }
            newNode.next = new AtomicMarkableReference<>(rightNode, false);
            if (leftNode.next.compareAndSet(rightNode, newNode, false, false)) {
                return true;
            }
        }
    }

    /**
     * Time complexity: O(n)
     */
    @Override
    public boolean remove(T value) {
        Node<T> rightNode;
        Node<T> rightNodeNext;
        Node<T> leftNode;
        while (true) {
            PairOfNodes<T> leftRightNodes = search(value);
            leftNode = leftRightNodes.first;
            rightNode = leftRightNodes.second;
            if (rightNode == tail || rightNode.value != value) {
                return false;
            }

            boolean[] isMarked = new boolean[1];
            rightNodeNext = rightNode.next.get(isMarked);

            if (!isMarked[0]) {
                if (rightNode.next.attemptMark(rightNodeNext, true)) {
                    break;
                }
            }
        }
        if (!leftNode.next.compareAndSet(rightNode, rightNodeNext, false, false)) {
            search(rightNode.value); // to delete the element implicitly, I guess
        }
        return true;
    }

    /**
     * Time complexity: O(n)
     */
    @Override
    public boolean contains(T value) {

        PairOfNodes<T> leftRightNodes = search(value);
        Node<T> rightNode = leftRightNodes.second;

        return rightNode != tail && rightNode.value == value;
    }

    /**
     * Time complexity: O(1)
     */
    @Override
    public boolean isEmpty() {
        return head.next.getReference() == tail;
    }
}
