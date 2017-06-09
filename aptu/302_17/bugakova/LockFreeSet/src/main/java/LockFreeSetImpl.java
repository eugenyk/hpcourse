import java.rmi.activation.ActivationGroup_Stub;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Created by nadya-bu on 28/05/2017.
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private class Node {
        private T value;
        private boolean isDelete;
        private AtomicReference<Node> next;

        public Node(T value, boolean isDelete, AtomicReference<Node> next) {
            this.value = value;
            this.isDelete = isDelete;
            this.next = next;
        }

        T getValue() {
            return value;
        }

        boolean isDelete() {
            return isDelete;
        }

        AtomicReference<Node> getNext() {
            return next;
        }
    }

    private AtomicReference<Node> root = new AtomicReference<Node>(new Node(null, false,
            new AtomicReference<Node>(null)));

    public boolean add(T value) {
        AtomicReference<Node> newNode =
                new AtomicReference<Node>(new Node(value, false, new AtomicReference<Node>(null)));
        while (true) {
            AtomicReference<Node> prevNodeRef = root;
            Node prevNode = prevNodeRef.get();
            AtomicReference<Node> curNodeRef = prevNode.getNext();
            Node curNode = curNodeRef.get();
            while (curNode != null) {
                if (curNode.isDelete()) {
                    if (!tryToConnect(prevNodeRef, prevNode, curNodeRef, curNode)) {
                        prevNodeRef = curNodeRef;
                        prevNode = curNode;
                        curNodeRef = curNode.getNext();
                        curNode = curNodeRef.get();
                    } else {
                        prevNode = prevNodeRef.get();
                        curNodeRef = prevNode.getNext();
                        curNode = curNodeRef.get();
                    }
                    continue;
                }
                if (curNode.getValue() != null && curNode.getValue().equals(value)) {
                    return false;
                }
                prevNodeRef = curNodeRef;
                prevNode = curNode;
                curNodeRef = curNode.getNext();
                curNode = curNodeRef.get();
            }
            Node newLastNode = new Node(prevNode.getValue(), prevNode.isDelete(), newNode);
            if (prevNodeRef.compareAndSet(prevNode, newLastNode)) {
                return true;
            }
        }
    }

    public boolean remove(T value) {
        while (true) {
            AtomicReference<Node> curNodeRef = root;
            Node curNode = curNodeRef.get();
            while (curNode != null) {
                if (curNode.isDelete()) {
                    curNodeRef = curNode.getNext();
                    curNode = curNodeRef.get();
                    continue;
                }
                if (curNode.getValue() != null && curNode.getValue().equals(value)) {
                    Node newCurNode = new Node(curNode.getValue(), true, curNode.getNext());
                    if (curNodeRef.compareAndSet(curNode, newCurNode)) {
                        return true;
                    } else {
                        break;
                    }
                }
                curNodeRef = curNode.getNext();
                curNode = curNodeRef.get();
            }
            if (curNode == null)
                return false;
        }
    }

    public boolean contains(T value) {
        AtomicReference<Node> curNodeRef = root;
        Node curNode = curNodeRef.get();
        while (curNode != null) {
            if (curNode.isDelete()) {
                curNodeRef = curNode.getNext();
                curNode = curNodeRef.get();
                continue;
            }
            if (curNode.getValue() != null && curNode.getValue().equals(value)) {
                return true;
            }
            curNodeRef = curNode.getNext();
            curNode = curNodeRef.get();
        }
        return false;
    }

    public boolean isEmpty() {
        AtomicReference<Node> curNodeRef = root.get().getNext();
        Node curNode = curNodeRef.get();
        while (curNode != null && curNode.isDelete()) {
            curNodeRef = curNode.getNext();
            curNode = curNodeRef.get();
        }

        return curNode == null;
    }

    private boolean tryToConnect(AtomicReference<Node> prevNodeRef, Node prevNode,
                                 AtomicReference<Node> curNodeRef, Node curNode) {
        if (curNode.getNext().get() == null) {
            return false;
        }
        Node newPrevNode = new Node(prevNode.getValue(), prevNode.isDelete(), curNode.getNext());
        return prevNodeRef.compareAndSet(prevNode, newPrevNode);
//        return false;
    }
}
