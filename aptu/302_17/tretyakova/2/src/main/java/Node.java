import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<T extends Comparable<T>> {

    public static final boolean UNMARKED = false;
    public static final boolean MARKED = true;

    private T key;
    private final AtomicMarkableReference<Node<T>> next;

    Node(T key, Node<T> next) {
        this.key = key;
        this.next = new AtomicMarkableReference<Node<T>>(next, UNMARKED);
    }

    public void setNext(Node<T> next) {
        this.next.set(next, UNMARKED);
    }

    public Node<T> getNext() {
        return next.getReference();
    }

    public AtomicMarkableReference<Node<T>> getAtomicNext() {
        return next;
    }

    public T getKey() {
        return key;
    }

    public boolean isMarked() {
        return next.isMarked();
    }
}
