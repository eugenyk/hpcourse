import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private AtomicMarkableReference<Node> head = new AtomicMarkableReference<>(new Node(null, null), false);

    @Override
    public boolean add(T value) {
        Node node = new Node(value, null);
        while (true) {
            Map.Entry<Node, Node> prevAndCur = find(value);
            if (prevAndCur.getValue() != null) {
                return false;
            }
            if (prevAndCur.getKey().next.compareAndSet(null, node, false, false)) {
                return true;
            }
        }

    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Map.Entry<Node, Node> prevAndCurrent = find(value);
            final Node pred = prevAndCurrent.getKey();
            final Node cur = prevAndCurrent.getValue();
            if (cur != null) {
                Node next = cur.next.getReference();
                if (cur.next.compareAndSet(next, next, false, true)) {
                    pred.next.compareAndSet(cur, next, false, false);
                    return true;
                }
            } else {
                return false;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        return find(value).getValue() != null;
    }

    @Override
    public boolean isEmpty() {
        AtomicMarkableReference<Node> cur = head.getReference().next;
        while (cur.getReference() != null) {
            if (!cur.isMarked()) {
                return false;
            }
            cur = cur.getReference().next;
        }
        return true;
    }


    @Override
    public Iterator<T> iterator() {
        while (true) {
            List<T> firstSnapshot = getValues();
            List<T> secondSnapshot = getValues();
            if (firstSnapshot.equals(secondSnapshot)) {
                return secondSnapshot.iterator();
            }
        }
    }

    private Map.Entry<Node, Node> find(T value) {
        Node pred = head.getReference();
        Node cur = pred.next.getReference();
        while (cur != null) {
            if (!cur.next.isMarked() && value.equals(cur.value)) {
                return new AbstractMap.SimpleEntry<>(pred, cur);
            }
            pred = cur;
            cur = cur.next.getReference();
        }
        return new AbstractMap.SimpleEntry<>(pred, null);
    }


    private List<T> getValues() {
        final List<T> res = new ArrayList<>();
        Node cur = head.getReference().next.getReference();
        while (cur != null) {
            if (!cur.next.isMarked()) {
                res.add(cur.value);
            }
            cur = cur.next.getReference();
        }
        return res;
    }

    private class Node {
        private T value;
        private AtomicMarkableReference<Node> next;

        private Node(T value, AtomicMarkableReference<Node> next) {
            this.value = value;
            this.next = next == null ?
                    new AtomicMarkableReference<>(null, false)
                    : next;

        }

    }
}
