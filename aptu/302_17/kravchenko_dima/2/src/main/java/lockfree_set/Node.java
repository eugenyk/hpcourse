package lockfree_set;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<T extends Comparable<T>> {

    private final T key;
    public AtomicMarkableReference<Node<T>> next;

    public Node(T key) {
        this.key = key;
        next = new AtomicMarkableReference<Node<T>>(null, false);
    }

    public T getKey() {
        return key;
    }

}
