package LockFreeSet;
import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * Created by jakutenshi on 1/14/18.
 */
public class LockFreeSetContainer<T extends Comparable<T>> implements LockFreeSet<T> {
    public boolean add(T value) {
        while (true) {
            Pair<T> prevcur = find(value);
            if (prevcur.cur == null) {
                Node<T> node = new Node<T>(value);
                node.next.set(null, false); // set?
                if (prevcur.prev.next.compareAndSet(null, node, false, false)) {
                    return true;
                }
            }

            if (prevcur.cur.value.equals(value)) {
                return false;
            } else {
                Node<T> node = new Node<T>(value);
                node.next.set(prevcur.cur, false); // set?
                if (prevcur.prev.next.compareAndSet(prevcur.cur, node, false, false)) {
                    return true;
                }
            }
        }
    }

    public boolean remove(T value) {
        while (true) {
            Pair<T> prevcur = find(value);

            if (!prevcur.cur.value.equals(value)) {
                return false;
            } else {
                Node succ = prevcur.cur.next.getReference();
                if (prevcur.cur.next.compareAndSet(succ, succ, false, true)) {
                    if (prevcur.prev != null) {
                        prevcur.prev.next.compareAndSet(prevcur.cur, succ, false, false);
                    }
                    return true;
                }
            }
        }
    }

    public boolean contains(T value) {
        Node<T> curr = head;
        while (curr.value.compareTo(value) < 0) {
            curr = curr.getNext();
        }
        return value.equals(curr.value) && !curr.next.isMarked();
    }

    public boolean isEmpty() {
        return head.next.isMarked();
    }

    private Pair<T> find(T value) {
        while (true) {
            Node<T> prev;
            Node<T> curr;
            Node<T> succ;
            if (head.getNext() == null) {
                prev = null;
                curr = head;
            } else {
                prev = head;
                curr = head.getNext();
            }
            while (true) {
                if (curr == null) {
                    return new Pair<T>(prev, curr);
                }
                succ = curr.next.getReference();
                if (curr.next.isMarked()) {
                    if (prev.next.compareAndSet(curr, succ, false, false)) {
                        curr = succ;
                    }
                } else {
                    if (curr.value.compareTo(value) >= 0) {
                        return new Pair<T>(prev, curr);
                    }
                    prev = curr;
                    curr = succ;
                }

            }
        }
    }

    private class Node<T> {
	    T value;
	    AtomicMarkableReference<Node<T>> next;
      	Node(T value) {
      	    this.value = value;
      	    next = new AtomicMarkableReference<Node<T>>(null, false);
        }

        Node<T> getNext() {
      	    return next.getReference();
        }
    }

    private class Pair<T> {
 		Node<T> prev;
 		Node<T> cur;

        Pair(Node<T> prev, Node<T> cur) {
            this.prev = prev;
            this.cur = cur;
        }
 	}

    private final Node<T> head;

    public LockFreeSetContainer(T value) {
        head = new Node<T>(value);
    }

}
