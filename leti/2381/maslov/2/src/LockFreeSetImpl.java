import java.util.concurrent.atomic.AtomicMarkableReference;


class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

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

    public LockFreeSetImpl() {
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
            curr = pred.next.get(marked);
            if (curr == tail)
                // ! if head == tail, return tail and head
                return new Window(head, tail);
            while (true) {
                if (curr == tail)
                    // ! if on tail, return tail and pred,
                    // key of the tail is always greater than any other (RSentiel = INT.MAX)
                    return new Window(pred, curr);
                while (marked[0]) {
                    succ = curr.next.get(marked);
                    snip = pred.next.compareAndSet(curr, succ, true, marked[0]);
                    if (!snip)
                        continue retry;
                    curr = succ;
                }
                if (curr.key >= key)
                    return new Window(pred, curr);
                pred = curr;
                curr = pred.next.getReference();
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
                snip = pred.next.attemptMark(curr, true);
                if (!snip)
                    continue;
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

            if (curr == tail)
                //! if reached tail then does not contain value
                return false;

            curr = curr.next.get(marked);
        }
        return (curr.key == key && !marked[0]);
    }

    @Override
    public boolean isEmpty() {
        Node succ;
        boolean[] marked = {false};
        while (true) {
            Node pred = head;
            Node curr = head.next.get(marked);
            if (!marked[0]) {
                if (curr == tail) {
                    return true;
                } else {
                    return false;
                }
            } else {
                succ = curr.next.get(marked);
                if (succ == tail) {
                    return true;
                } else {
                    if (!marked[0]) {
                        return false;
                    } else {
                        pred.next.compareAndSet(curr, succ, true, true);
                    }
                }
            }
        }
    }
}
