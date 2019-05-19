import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private AtomicMarkableReference<Node> head = new AtomicMarkableReference<>(null, false);

    @Override
    public boolean add(T value) {
        return false;
    }

    @Override
    public boolean remove(T value) {
        return false;
    }

    @Override
    public boolean contains(T value) {
        return false;
    }

    @Override
    public boolean isEmpty() {
        return false;
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

    private Map.Entry<Node, Node> find() {
        return new AbstractMap.SimpleEntry<>(new Node(null, null), new Node(null, null));
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
            this.next = next;
        }

    }
}
