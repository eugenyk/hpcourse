import java.util.concurrent.atomic.AtomicReference;


public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private class Node {
        T value;
        boolean isDeleted;
        AtomicReference<Node> next;

        public Node(T value, boolean isDeleted, AtomicReference<Node> next) {
            this.value = value;
            this.isDeleted = isDeleted;
            this.next = next;
        }
    }

    private AtomicReference<Node> head = new AtomicReference<Node>(new Node(null, false, new AtomicReference<Node>(null)));

    public boolean add(T value) {
        AtomicReference<Node> newNode = new AtomicReference<Node>(new Node(value, false, new AtomicReference<Node>(null)));
        while (true) {
            AtomicReference<Node> prevRef = head;
            Node prev = head.get();
            AtomicReference<Node> curRef = head.get().next;
            Node cur = curRef.get();
            while (cur != null) {
                if (cur.isDeleted) {
                    if (wereConnectedUsingCAS(prevRef, prev, curRef, cur)) {
                        prev = prevRef.get();
                        curRef = prev.next;
                        cur = curRef.get();
                    } else {
                        prevRef = curRef;
                        prev = cur;
                        curRef = cur.next;
                        cur = curRef.get();
                    }
                } else {
                    if (cur.value != null && cur.value.equals(value)) return false;
                    prevRef = curRef;
                    prev = cur;
                    curRef = cur.next;
                    cur = curRef.get();
                }
            }
            Node newTail = new Node(prev.value, prev.isDeleted, newNode);
            if (prevRef.compareAndSet(prev, newTail)) return true;
        }
    }

    public boolean remove(T value) {
        while (true) {
            AtomicReference<Node> curRef = head;
            Node cur = curRef.get();
            while (cur != null) {
                if (cur.isDeleted) {
                    curRef = cur.next;
                    cur = curRef.get();
                } else {
                    if (cur.value != null && cur.value.equals(value)) {
                        Node newCur = new Node(cur.value, true, cur.next);
                        if (curRef.compareAndSet(cur, newCur)) return true;
                        else break;
                    }
                    curRef = cur.next;
                    cur = curRef.get();
                }
            }
            if (cur == null) return false;
        }
    }

    public boolean contains(T value) {
        AtomicReference<Node> curRef = head;
        Node cur = curRef.get();
        while (cur != null) {
            if (!cur.isDeleted && cur.value != null && cur.value.equals(value)) return true;
            curRef = cur.next;
            cur = curRef.get();
        }
        return false;
    }

    public boolean isEmpty() {
        AtomicReference<Node> curRef = head.get().next;
        Node cur = curRef.get();
        Node headNextBeforeTraversing = head.get().next.get();
        while (cur != null && cur.isDeleted) {
            curRef = cur.next;
            cur = curRef.get();
        }
        boolean listWasNotModified = head.get().next.get() == headNextBeforeTraversing;
        return cur == null && listWasNotModified;
    }

    private boolean wereConnectedUsingCAS(AtomicReference<Node> prevRef, Node prev, AtomicReference<Node> curRef, Node cur) {
        if (cur.next.get() == null) return false;
        Node newPrev = new Node(prev.value, prev.isDeleted, cur.next);
        return prevRef.compareAndSet(prev, newPrev);
    }
}