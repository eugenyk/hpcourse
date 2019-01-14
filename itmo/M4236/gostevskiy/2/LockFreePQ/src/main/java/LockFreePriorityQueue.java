import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private Node<E> head;
    private Node<E> tail;

    LockFreePriorityQueue() {
        head = new Node<>(null);
        tail = new Node<>(null);
        head.next = new AtomicMarkableReference<>(tail, false);
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
            Node<E> node = new Node<>(e);
            node.next.set(cur, false);
            if (pred.next.compareAndSet(cur, node, false, false)) return true;
        }
    }

    public PairNode<E> find(E e) {
        retry: while (true) {
            Node<E> pred = head;
            Node<E> cur = pred.next.getReference();
            if (cur == null || cur == tail) return new PairNode<>(pred, cur);
            Node<E> suc;
            while (true) {
                if (cur == null || cur == tail) return new PairNode<>(pred, cur);
                AtomicMarkableReference<Node<E>> suc_t = cur.next;
                suc = suc_t.getReference();
                boolean mrk = suc_t.isMarked();
                if (mrk) {
                    if (!pred.next.compareAndSet(cur, suc, false, false)) continue retry;
                    cur = suc;
                } else {
                    if (e == null || cur.elem.compareTo(e) >= 0) return new PairNode<>(pred, cur);
                    pred = cur;
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
            if (cur == tail) return null;
            Node<E> succ = cur.next.getReference();
            if (!cur.next.compareAndSet(succ, succ, false, true)) continue;
            pred.next.compareAndSet(cur, succ, false, false);
            return cur.elem;
        }
    }

    public E peek() {
        PairNode<E> nodes = find(null);
        Node<E> cur = nodes.second;
        if (cur == tail) return null;
        return cur.elem;
    }

    private static class Node<E> {
        private E elem;
        private AtomicMarkableReference<Node<E>> next;
        private Node(E val) {
            elem = val;
            next = new AtomicMarkableReference<>(null, false);
        }
    }
    private static class PairNode<E> {
        private Node<E> first;
        private Node<E> second;
        private PairNode(Node<E> first, Node<E> second) {
            this.first = first;
            this.second = second;
        }
    }
}
