package priority_q;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private final Node<E> head_; // top element stands just after head_
    private final Node<E> tail_;
    AtomicInteger size_ = new AtomicInteger(0);

    public LockFreePriorityQueue() {
        head_ = new Node<>(null, null, false);
        tail_ = new Node<>(null, null, false);
        head_.setNext(tail_, false);
    }

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public boolean offer(E key) {
//        System.out.println("-----offer " + key + " -------");

        final Node<E> node = new Node<>(key, null, false);

        while (true) {
            final PrevCurr<E> p = find(key);
            final Node<E> prev = p.prev;
            final Node<E> curr = p.curr;

            node.setNext(curr, false);
            if (prev.casNext(curr, node, false, false)) {
                increment_size();
                break;
            }
        }

        return true;
    }

    @Override
    public E poll() { // return top and delete
        final E result;
        while (true) {
            final Node<E> curr = internalPeek();

            if (curr == tail_) {
                result = null;
                break;
            }

            final Node<E> succ = curr.getNext();
            if (!curr.casNext(succ, succ, false, true)) {
                continue;
            }

            head_.casNext(curr, succ, false, false); // physical delete (It does not have to be successful)

            result = curr.data;
            decrement_size();
            break;
        }
//        System.out.println("----- " + size() + " poll " + result + " -------");

        return result;

    }

    @Override
    public E peek() { // return top
        Node<E> topNode = internalPeek();
        return topNode.data;
    }

    @Override
    public int size() {
        return size_.get();
    }

    @Override
    public boolean isEmpty() {
        return size() == 0;
    }

    private Node<E> internalPeek() {
        final Node<E> prev = head_;
        Node<E> curr = null;
        Node<E> succ = null;
        boolean marked[] = {false};

        while (true) {
            curr = head_.getNext();
            if (curr == tail_) {
                return tail_;
            }

            succ = curr.getNext(marked);
            if (marked[0]) {
                prev.casNext(curr, succ, false, false);
                continue;
            }
            return curr;
        }
    }

    private PrevCurr<E> find(E key) {
        Node<E> prev = null;
        Node<E> curr = null;
        Node<E> succ = null;

        retry:
        while (true) {
            if (head_.getNext() == tail_) {
                return new PrevCurr<>(head_, tail_);
            }

            prev = head_;
            boolean marked[] = {false};

            while (true) {
                curr = prev.getNext();
                succ = curr.getNext(marked);

                if (marked[0]) {
                    if (!prev.casNext(curr, succ, false, false)) {
                        continue retry;
                    }
                    continue;
                }

                if (curr == tail_ || curr.data.compareTo(key) >= 0) {
                    return new PrevCurr<>(prev, curr);
                }
                prev = curr;
            }
        }
    }

    private void decrement_size() {
        boolean success;
        do {
            int tmp = size_.get();
            success = size_.compareAndSet(tmp, tmp - 1);
        } while (!success);
    }

    private void increment_size() {
        boolean success;
        do {
            int tmp = size_.get();
            success = size_.compareAndSet(tmp, tmp + 1);
        } while (!success);
    }
}

class PrevCurr<E extends Comparable<E>> {
    final Node<E> prev;
    final Node<E> curr;

    PrevCurr(final Node<E> prev, final Node<E> curr) {
        this.prev = prev;
        this.curr = curr;
    }
}

class Node<E extends Comparable<E>> extends AtomicMarkableReference<Node<E>> {
    final E data;

    public Node(E curData, Node<E> next, boolean curMark) {
        super(next, curMark); // trick: we store next as AMR reference, but its mark corresponds to current node
        this.data = curData;
    }

    public void setNext(Node<E> next, boolean curMark) {
        set(next, curMark);
    }

    public Node<E> getNext() {
        return getReference();
    }

    // getting nextNode and currentMark at the same time
    public Node<E> getNext(boolean curMarked[]) {
        return get(curMarked);
    }

    public boolean casNext(Node<E> nextOld, Node<E> nextNew, boolean curMarkOld, boolean curMarkNew) {
        return compareAndSet(nextOld, nextNew, curMarkOld, curMarkNew);
    }

    public boolean isCurMarked() {
        return isMarked();
    }
}