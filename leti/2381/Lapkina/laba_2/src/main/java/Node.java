import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<T> {

    T item;
    int key;
    AtomicMarkableReference<Node> next;

    public Node(T item) {
        this.item = item;
        this.key = item.hashCode();
    }

    public Node() {
        //! for head and tail only
    }
}