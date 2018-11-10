package priority_q;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    // top element stand just after root
    private final Node<E> head_;
    private final Node<E> tail_;
    AtomicInteger size_ = new AtomicInteger(0);

    public LockFreePriorityQueue() {
        head_ = new Node<>(null);
        tail_ = new Node<>(null);
        head_.next.set(tail_, false);
    }

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public boolean offer(E key) {
        System.out.println("-----offer " + key + " -------");

        final Node<E> node = new Node<>(key);

        while (true) {
            final PrevCurr<E> p = find(key);
            final Node<E> prev = p.prev;
            final Node<E> curr = p.curr;

            node.next.set(curr, false);
            if (prev.next.compareAndSet(curr, node, false, false)) {
                increment_size();
                break;
            }
        }
//        printPipe();

        return true;
    }

//    public void printPipe() {
//        AtomicMarkableReference<Node<E>> tmp = head_;
//        for (int i = 0; i < size(); i++) {
//            tmp = tmp.getReference().next;
//            System.out.print(tmp.getReference() + " " + tmp.getReference().data + " | ");
//            System.out.print(tmp.getReference().data + " | ");
//        }
//        System.out.println();
//    }

    @Override
    public E poll() { // return top and delete
        final E result;
        while (true) {
            final Node<E> curr = internalPeek();

            if (curr == tail_) {
                result = null;
                break;
            }

            final Node<E> succ = curr.next.getReference();
            if (!curr.next.compareAndSet(succ, succ, false, true)) {
                continue;
            }

            if (head_.next.compareAndSet(curr, succ, false, false)) { //
                System.out.println("main phys delete");
            } else {
                System.out.println("main phys delete fail");
            }

            result = curr.data;
            decrement_size();
            break;
        }
        System.out.println("----- " + size() + " poll " + result + " -------");

        return result;

    }

    @Override
    public E peek() { // return top
        System.out.println("----- peek -------");

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
            curr = head_.next.getReference();
            if (curr == tail_) {
                return tail_;
            }

            succ = curr.next.get(marked);
            if (marked[0]) {
                prev.next.compareAndSet(curr, succ, false, false);
                System.out.println("intPeek_fail");
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
            if (head_.next.getReference() == tail_) {
                return new PrevCurr<>(head_, tail_);
            }

            prev = head_;
            boolean marked[] = {false};
            while (true) {
                curr = prev.next.getReference();
                succ = curr.next.get(marked);
                if (marked[0]) {
                    if (!prev.next.compareAndSet(curr, succ, false, false)) {
                        System.out.println("find physically delete fail prev=" + prev + " " + prev.next.isMarked() + " curr=" + curr + " " + curr.next.isMarked() + " succ=" + succ + " " + succ.next.isMarked());
                        continue retry; //TODO maybe should be another
                    }
                    System.out.println("find physically delete prev=" + prev + " " + prev.next.isMarked() + " curr=" + curr + " " + curr.next.isMarked() + " succ=" + succ + " " + succ.next.isMarked());

                }

                if (curr == tail_ || curr.data.compareTo(key) >= 0) {
                    return new PrevCurr<>(prev, curr);
                }
//                prev.set(curr.getReference(), curr.isMarked());
                prev = curr;
//                assert prev.getReference().data != null;
//                assert curr.getReference().data != null;
//                assert prev.getReference() == curr.getReference();
//                assert prev.getReference().data == curr.getReference().data;
//                assert prev.getReference().next == curr.getReference().next;
//
//                assert prev.getReference().data.compareTo(curr.getReference().data) <= 0;

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

class Node<E extends Comparable<E>> {
    final E data;
    final AtomicMarkableReference<Node<E>> next;
//    private static final AtomicReferenceFieldUpdater<Node, Object> dataUpdater = AtomicReferenceFieldUpdater.newUpdater(Node.class, Object.class, "data");
//    private static final AtomicReferenceFieldUpdater<Node, Node> nextUpdater = AtomicReferenceFieldUpdater.newUpdater(Node.class, Node.class, "next");

    public Node(E data) {
        next = new AtomicMarkableReference<>(null, false);
        this.data = data;
    }

}