import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private static class Pair<U, V> {

        private U first;
        private V second;

        private Pair(U first, V second) {
            this.first = first;
            this.second = second;
        }

        private U getFirst() {
            return first;
        }

        private V getSecond() {
            return second;
        }
    }


    private class Node {
        private T value;
        private AtomicMarkableReference<Node> next;

        private Node(T value, AtomicMarkableReference<Node> next) {
            this.value = value;
            if (next == null) {
                this.next = new AtomicMarkableReference<>(null, false);
            } else {
                this.next = next;
            }
        }

    }

    private AtomicMarkableReference<Node> head = new AtomicMarkableReference<>(new Node(null, null), false);

    @Override
    public boolean add(T value) {
        Node node = new Node(value, null);
        while (true) {
            Pair<Node, Node> pair = find(value);
            if (pair.getSecond() != null) {
                return false;
            }
            AtomicMarkableReference<Node> next = pair.getFirst().next;
            if (next.compareAndSet(null, node, false, false)) {
                return true;
            }
        }

    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Pair<Node, Node> pair = find(value);
            Node prev = pair.getFirst();
            Node cur = pair.getSecond();
            if (cur != null) {
                Node next = cur.next.getReference();
                if (cur.next.compareAndSet(next, next, false, true)) {
                    prev.next.compareAndSet(cur, next, false, false);
                    return true;
                }
            } else {
                return false;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        return find(value).getSecond() != null;
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

    private boolean identical(List<T> first, List<T> second) {
        if (first.size() != second.size())
            return false;
        for (int i = 0; i < first.size(); i++) {
            if (first.get(i) != second.get(i))
                return false;
        }
        return true;
    }
    
    @Override
    public Iterator<T> iterator() {
        while (true) {
            List<T> snap = snapshot();
            List<T> nextSnap = snapshot();
            if (identical(snap, nextSnap)) {
                return snap.iterator();
            }
        }
    }

    private Pair<Node, Node> find(T value) {
        Node pred = head.getReference();
        Node cur = pred.next.getReference();
        while (cur != null) {
            if (!cur.next.isMarked() && value.equals(cur.value)) {
                return new Pair<>(pred, cur);
            }
            pred = cur;
            cur = cur.next.getReference();
        }
        return new Pair<>(pred, null);
    }


    private List<T> snapshot() {
        List<T> res = new ArrayList<>();
        Node cur = head.getReference().next.getReference();
        while (cur != null) {
            if (!cur.next.isMarked()) {
                res.add(cur.value);
            }
            cur = cur.next.getReference();
        }
        return res;
    }

    public static void main(String[] args) {
        LockFreeSetImpl<Long> set = new LockFreeSetImpl<>();
        set.add(3L);
        set.snapshot().forEach(System.out::println);
    }
}
