import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<T> {

    T value;
    AtomicMarkableReference<Node<T>> next;

    Node(T value) {
        this.value = value;
        this.next = new AtomicMarkableReference<>(null, false);
    }
}