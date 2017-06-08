import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSet<T extends Comparable<T>> {
    private final Node<T> tail = new Node<T>(null, null);
    private final AtomicMarkableReference<Node<T>> head = new AtomicMarkableReference<Node<T>>(tail, Node.UNMARKED);

    private static class SearchResult<T extends Comparable<T>> {
        public final AtomicMarkableReference<Node<T>> prev;
        public final Node<T> next;

        public SearchResult(AtomicMarkableReference<Node<T>> prev, Node<T> next) {
            this.prev = prev;
            this.next = next;
        }
    }

    public boolean add(T value) {
        SearchResult<T> res = search(value);
        if(res != null && res.next.getKey().equals(value)) {
            return false;
        }

        Node<T> next = head.getReference();
        Node<T> new_node = new Node<T>(value, next);

        while(!head.compareAndSet(next, new_node, Node.UNMARKED, Node.UNMARKED)) {
            next = head.getReference();
            new_node.setNext(next);
        }

        return true;
    }

    public boolean remove(T value) {
        SearchResult<T> res;
        Node<T> next;

        do {
            res = search(value);
            if (res == null || !res.next.getKey().equals(value)) {
                return false;
            }

            next = res.next.getNext();
            if(!next.isMarked()) {
                if (res.next.getAtomicNext().attemptMark(next, Node.MARKED)) {
                    break;
                }
            }
        } while(true);

        if(!res.prev.compareAndSet(res.next, next, Node.UNMARKED, Node.UNMARKED)) {
            search(res.next.getKey());
        }

        return true;
    }

    public boolean contains(T value) {
        Node<T> next = head.getReference();
        while(!next.equals(tail)) {
            if(!next.isMarked() && next.getKey().equals(value)) {
                    return true;
            }
            next = next.getNext();
        }
        return false;
    }

    public boolean isEmpty() {
        return head.getReference().equals(tail);
    }

    private SearchResult<T> search(T value) {
        AtomicMarkableReference<Node<T>> prev = head;
        Node<T> next = head.getReference();
        while(!next.equals(tail)) {
            if(!next.isMarked()) {
                if (next.getKey().equals(value)) {
                    return new SearchResult<T>(prev, next);
                } else {
                    prev = next.getAtomicNext();
                    next = next.getNext();
                }
            } else if(prev.compareAndSet(next, next.getNext(), Node.UNMARKED, Node.UNMARKED)) {
                next = prev.getReference();
            } else {
                prev = head;
                next = head.getReference();
            }
        }
        return null;
    }
}
