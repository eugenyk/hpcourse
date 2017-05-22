import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicReference;

import static java.lang.Integer.reverse;

public class LockFreeHashSet  <T extends Comparable<T>> implements LockFreeSet<T> {
    private class Node implements Comparable <Node> {
        boolean isBucket;
        T value;
        int hash;
        AtomicReference<Node> next;
        boolean isDelete;

        Node (boolean isBucket, T value, int hash) {
            this.isBucket = isBucket;
            this.value = value;
            this.hash = hash;
            next = null;
            isDelete = false;
        }

        Node (boolean isBucket, T value, int hash, AtomicReference<Node> next, boolean isDelete) {
            this.isBucket = isBucket;
            this.value = value;
            this.hash = hash;
            this.next = next;
            this.isDelete = isDelete;
        }

        @Override
        public int compareTo(Node node) {
            Integer v1 = reverse(hash);
            Integer v2 = reverse(node.hash);

            return v1.compareTo(v2);
        }
    };

    private static final int maxStep = 4;

    private AtomicReference<Integer> cntElem = new AtomicReference<>(0);

    private ArrayList< AtomicReference<Node> > buckets = new ArrayList<>();

    private int bucketsSize = 1;

    private AtomicReference< Node > findBucket(int hash) {
        return buckets.get(hash % bucketsSize);
    }

    private void rehash() {
        int newSize = bucketsSize * 2;
        for (int curNewB = bucketsSize; curNewB < newSize; ++curNewB) {
            Node node = new Node(true, null, curNewB);
            while (!addNode(node));
            buckets.add(new AtomicReference<Node>(node));
        }

        bucketsSize = newSize;
    }

    private boolean addNode(Node node) {
        AtomicReference<Node> curNode = findBucket(node.hash);

        int cnt = 0;

        while (curNode.get().next != null && curNode.get().next.get().compareTo(node) < 0) {
            curNode = curNode.get().next;
            ++cnt;
        }

        if (cnt > maxStep && !node.isBucket) {
            rehash();
            return false;
        }

        Node pr = curNode.get();
        AtomicReference<Node> nx = curNode.get().next;

        if (pr.isDelete || (nx != null && nx.get().isDelete)) {
            return false;
        }

        node.next = nx;
        AtomicReference<Node> cur = new AtomicReference<>(node);

        return curNode.compareAndSet(pr, new Node(pr.isBucket, pr.value, pr.hash, cur,false));
    }

    public LockFreeHashSet() {
        bucketsSize = 2;
        buckets.add(new AtomicReference<Node>(new Node(true, null, 0)));
        buckets.add(new AtomicReference<Node>(new Node(true, null, 1)));
        buckets.get(0).get().next = buckets.get(1);
    }

    @Override
    public boolean add(T value) {
        while (!contains(value)) {
            if (addNode(new Node(false, value, value.hashCode()))) {
                while (true) {
                    int oldVal = cntElem.get();
                    if (cntElem.compareAndSet(oldVal, oldVal + 1)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    @Override
    public boolean remove(T value) {
        while (contains(value)) {
            Node node = new Node(false, value, value.hashCode());
            AtomicReference<Node> cur = findBucket(value.hashCode());
            while (cur.get().next != null && cur.get().next.get().compareTo(node) < 0) {
                cur = cur.get().next;
            }

            if (cur.get().next == null) return false;

            Node curn = cur.get();
            Node del = cur.get().next.get();
            AtomicReference<Node> nxt = del.next;
            if (del.compareTo(node) != 0 || del.isBucket) {
                continue;
            }

            Node newDel = new Node(false, del.value, del.hash, del.next, true);
            if (!curn.next.compareAndSet(del, newDel)) continue;
            if (cur.compareAndSet(curn, new Node(curn.isBucket, curn.value, curn.hash, nxt,
                    curn.isDelete))) {
                while (true) {
                    Integer oldVal = cntElem.get();
                    if (cntElem.compareAndSet(oldVal, oldVal - 1)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    @Override
    public boolean contains(T value) {
        AtomicReference<Node> cur = findBucket(value.hashCode());
        Node node = new Node(false, value, value.hashCode());
        while (cur.get().next != null && cur.get().next.get().compareTo(node) <= 0) {
            cur = cur.get().next;
        }

        return cur.get().compareTo(node) == 0 && !cur.get().isBucket;
    }

    @Override
    public boolean isEmpty() {
        return cntElem.get() > 0;
    }
}
