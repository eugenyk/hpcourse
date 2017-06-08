import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static java.lang.Integer.reverse;

public class LockFreeListSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private class Node implements Comparable <Node> {
        T value;
        AtomicReference<Node> next;
        boolean isDelete;

        Node (T value) {
            this.value = value;
            next = new AtomicReference<>(null);
            isDelete = false;
        }

        Node (T value, AtomicReference<Node> next, boolean isDelete) {
            this.value = value;
            this.next = next;
            this.isDelete = isDelete;
        }

        @Override
        public int compareTo(Node node) {
            if (value == null || node.value == null) return -1;
            return value.compareTo(node.value);
        }
    };

    private class Pair<T> {
        T fr;
        T sc;

        Pair() {
            this.fr = null;
            this.sc = null;
        }

        Pair(T fr, T sc) {
            this.fr = fr;
            this.sc = sc;
        }
    }

    private AtomicReference<Node> head = new AtomicReference<Node>(new Node(null));

    public LockFreeListSet() {}

    @Override
    public boolean add(T value) {
        Pair<AtomicReference<Node>> elem = findElem(value);
        Node newNode = new Node(value);
        while (elem.sc.get() == null || newNode.compareTo(elem.sc.get()) != 0) {
            Pair <Node> old_elem = new Pair<>(elem.fr.get(), elem.sc.get());
            if (old_elem.fr.isDelete || (old_elem.sc != null && old_elem.sc.isDelete)) {
                elem = findElem(value);
                continue;
            }

            if (old_elem.sc != null && old_elem.sc != old_elem.fr.next.get()) {
                elem = findElem(value);
                continue;
            }

            newNode.next = old_elem.fr.next;
            if (!elem.fr.compareAndSet(old_elem.fr, new Node(old_elem.fr.value, new AtomicReference<Node>(newNode), false))) {
                elem = findElem(value);
                continue;
            }
            return true;
        }

        return false;
    }

    @Override
    public boolean remove(T value) {
        Pair<AtomicReference<Node>> elem = findElem(value);
        while (elem.sc.get() != null) {
            Pair<Node>old_elem = new Pair<>(elem.fr.get(), elem.sc.get());

            if (old_elem.sc == null || old_elem.fr.isDelete || old_elem.sc.isDelete) {
                elem = findElem(value);
                continue;
            }
            if (elem.sc.compareAndSet(old_elem.sc, new Node(old_elem.sc.value, old_elem.sc.next, true))) {
                return true;
            }
            elem = findElem(value);
        }
        return false;
    }

    @Override
    public boolean contains(T value) {
        Pair<AtomicReference<Node>> cur = findElem(value);
        return cur.sc.get() != null && cur.sc.get().compareTo(new Node(value)) == 0;
    }

    @Override
    public boolean isEmpty() {
        return head.get().next.get() == null;
    }

    private boolean tryDelElem(AtomicReference<Node> pr, AtomicReference<Node> nx) {
        Node old_pr = pr.get();
        if (old_pr.next != nx) {
            return false;
        }

        if (old_pr.isDelete) {
            return false;
        }

        return pr.compareAndSet(old_pr, new Node(old_pr.value, nx.get().next, false));

    }

    private Pair<AtomicReference<Node>> findElem(T value) {
        Pair<AtomicReference<Node>> res = new Pair<>(head, head);
        Node cur = new Node(value);
        while (res.sc.get() != null && res.sc.get().compareTo(cur) != 0) {
            res.fr = res.sc;
            res.sc = res.sc.get().next;
            if (res.sc.get() != null && res.sc.get().isDelete) {
                tryDelElem(res.fr, res.sc);
                res.fr = head;
                res.sc = head;
            }
        }
        return res;
    }
}
