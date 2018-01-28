import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Created by Anderson on 27.12.2017.
 */
class ListBasedLockFreeSet<T extends Comparable<T>> implements LockFreeSet<T> {

    public class Node {
        T item;
        int key;
        AtomicMarkableReference<Node> next;

        public Node(T item) {
            this.item = item;
            this.key = item.hashCode();
        }

        private Node() {
            //! for head and tail only
        }
    }

    private Node head = new Node();
    private Node tail = new Node();

    public ListBasedLockFreeSet() {
        head.key = Integer.MIN_VALUE;
        head.next = new AtomicMarkableReference<>(tail, false);

        tail.key = Integer.MAX_VALUE;
    }

    private class Window {
        public Node pred, curr;

        public Window(Node pred, Node curr) {
            this.pred = pred;
            this.curr = curr;
        }
    }

    private Window find(int key) {
        Node pred, curr, succ;
        boolean[] marked = {false};
        boolean snip;
        retry:
        while (true) {
            pred = head;
            curr = pred.next.getReference();
            if (curr == tail)
                // ! if head == tail, return tail and head
                return new Window(pred, tail);
            while (true) {
                if (curr == tail)
                    // ! if on tail, return tail and pred,
                    // key of the tail is always greater than any other (RSentiel = INT.MAX)
                    return new Window(pred, tail);
                succ = curr.next.get(marked);
                while (marked[0]) {
                    snip = pred.next.compareAndSet(curr, succ, false, false);
                    if (!snip)
                        continue retry;
                    curr = succ;
                    succ = curr.next.get(marked);
                }
                if (curr.key >= key)
                    return new Window(pred, curr);
                pred = curr;
                curr = succ;
            }
        }
    }

    @Override
    public boolean add(T value) {
        int key = value.hashCode();
        while (true) {
            Window window = find(key);
            Node pred = window.pred;
            Node curr = window.curr;
            if (curr.key == key)
                return false;
            else {
                Node node = new Node(value);
                node.next = new AtomicMarkableReference<>(curr, false);
                if (pred.next.compareAndSet(curr, node, false, false))
                    return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        int key = value.hashCode();
        boolean snip;
        while (true) {
            Window window = find(key);
            Node pred = window.pred;
            Node curr = window.curr;
            if (curr.key != key) {
                return false;
            } else {
                Node succ = curr.next.getReference();
                snip = curr.next.attemptMark(succ, true);
                if (!snip)
                    continue;
                pred.next.compareAndSet(curr, succ, false, false);
                return true;
            }
        }
    }

    @Override
    public boolean contains(T value) {
        boolean[] marked = {false};
        int key = value.hashCode();
        Node curr = head;
        while (curr.key < key) {
            curr = curr.next.getReference();

            if (curr == tail)
                //! if reached tail then does not contain value
                return false;

            curr.next.get(marked);
        }
        return (curr.key == key && !marked[0]);
    }

    @Override
    public boolean isEmpty() {
        Node succ;
        boolean[] marked = {false};
        while (true) {
            Node pred = head;
            Node curr = head.next.getReference();
            if (curr == tail)
                return true;
            succ = curr.next.get(marked);
            if (!marked[0])
                return false;

            pred.next.compareAndSet(curr, succ, true, false);
        }
    }
}