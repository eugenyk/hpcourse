import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private Node<E> head;

    LockFreePriorityQueue() {
        head = new Node<>(null);
    }

    public int size() {
        return 0;
    }

    public boolean isEmpty() {
        return peek() == null;
    }

    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }

    public boolean offer(E e) {
        while (true) {
            PairNode<E> nodes = find(e);
            Node<E> pred = nodes.first;
            Node<E> cur = nodes.second;
            if (cur != null && e.compareTo(cur.elem) > 0) continue;
            Node<E> node = new Node<>(e);
            node.next.set(cur, false);
            if (pred.next.compareAndSet(cur, node, false, false)) return true;
        }
    }

    public PairNode<E> find(E e) {
        retry: while (true) {
            Node<E> pred = head;
            AtomicMarkableReference<Node<E>> cur = pred.next;
            if (cur.getReference() == null || e == null || cur.getReference().elem.compareTo(e) >= 0) return new PairNode<>(pred, cur.getReference());
            AtomicMarkableReference<Node<E>> suc;
            while (true) {
                if (cur.getReference() == null) return new PairNode<>(pred, cur.getReference());
                suc = cur.getReference().next;
                if (cur.isMarked()) {
                    if (!pred.next.compareAndSet(cur.getReference(), suc.getReference(), false, false)) continue retry;
                    cur = suc;
                } else {
                    if (cur.getReference().elem.compareTo(e) >= 0) return new PairNode<>(pred, cur.getReference());
                    pred = cur.getReference();
                    cur = suc;
                }
            }
        }
    }

    public E poll() {
        while (true) {
            PairNode<E> nodes = find(null);
            Node<E> pred = nodes.first;
            Node<E> cur = nodes.second;
            if (cur == null) return null;
            Node<E> succ = cur.next.getReference();
            if (!cur.next.compareAndSet(succ, succ, false, true)) continue;
            pred.next.compareAndSet(cur, succ, false, false);
            return cur.elem;
        }
    }

    public E peek() {
        PairNode<E> nodes = find(null);
        Node<E> cur = nodes.second;
        if (cur == null) return null;
        return cur.elem;
    }

    public static class Node<E> {
        public E elem;
        public AtomicMarkableReference<Node<E>> next;
        public Node(E val) {
            elem = val;
            next = new AtomicMarkableReference<>(null, false);
        }
    }
    public static class PairNode<E> {
        public Node<E> first;
        public Node<E> second;
        public PairNode(Node<E> first, Node<E> second) {
            this.first = first;
            this.second = second;
        }
    }

}
