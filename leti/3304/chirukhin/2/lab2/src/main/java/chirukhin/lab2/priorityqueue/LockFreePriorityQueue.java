/*
    Implementation of
    'Fast and lock-free concurrent priority queues for multi-thread systems'
    by Hakan Sundell and Philippas Tsigas

    http://www.cse.chalmers.se/~tsigas/papers/JPDC-Lock-free-skip-lists-and-Queues.pdf
*/

package chirukhin.lab2.priorityqueue;

import java.util.*;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private static class Node<E> {
        int level, validLevel;
        AtomicMarkableReference<E> data;
        Node<E> prev;
        ArrayList<AtomicMarkableReference<Node<E>>> next;

        Node() {
            this.data = new AtomicMarkableReference<>(null, false);
            this.prev = null;
            this.next = new ArrayList<>(MAXLEVEL);
            this.validLevel = -1;
        }

        Node(int l, E d) {
            this.data = new AtomicMarkableReference<>(d, false);
            this.next = new ArrayList<>(MAXLEVEL);
            this.level = l;
        }
    }

    private static class NodePair<E> {
        Node<E> n1;
        Node<E> n2;

        NodePair(Node<E> nn1, Node<E> nn2) {
            n1 = nn1;
            n2 = nn2;
        }
    }

    private class PQueueIterator implements Iterator<E> {

        Node<E> cursor = head.next.get(0).getReference();

        public boolean hasNext() {
            return cursor != tail;
        }

        public E next() {
            if (cursor == tail)
                throw new NoSuchElementException();

            E result = cursor.data.getReference();
            cursor = cursor.next.get(0).getReference();
            return result;
        }
    }

    private static final int MAXLEVEL = 10;

    private Node<E> head = new Node<>();

    private Node<E> tail = new Node<>();

    private static final Random RAND_GEN = new Random();

    public LockFreePriorityQueue() {
        for (int i = 0; i < MAXLEVEL; i++) {
            head.next.add(new AtomicMarkableReference<>(tail, false));
        }
    }

    @Override
    public boolean add(E e) {
        int level, i;

        Node<E> newN;
        Node<E> n1, n2;
        ArrayList<Node<E>> savedNode = new ArrayList<>();
        NodePair<E> tn1, tn2, tn3;

        for (int ii = 0; ii < MAXLEVEL; ii++) {
            savedNode.add(new Node<>());
        }

        level = randomLevel();
        newN = new Node<>(level, e);

        n1 = head;

        for (i = MAXLEVEL - 1; i >= 1; i--) {
            tn1 = scanKey(n1, i, newN);
            n2 = tn1.n2;
            n1 = tn1.n1;
            if (i < level) {
                savedNode.set(i, n1);
            }
        }

        int kk = 0;
        while (true) {
            tn2 = scanKey(n1, 0, newN);
            n2 = tn2.n2;
            n1 = tn2.n1;

            if (kk == 0) {
                newN.next.add(new AtomicMarkableReference<>(n2, false));
                kk++;
                newN.validLevel = 0;
            } else {
                newN.next.set(0,
                        new AtomicMarkableReference<>(n2, false));
            }
            if (n1.next.get(0).compareAndSet(n2, newN, false, false)) {
                break;
            }
        }

        for (i = 1; i <= level - 1; i++) {
            newN.validLevel = i;
            n1 = savedNode.get(i);
            kk = 0;
            while (true) {

                tn3 = scanKey(n1, i, newN);
                n2 = tn3.n2;
                n1 = tn3.n1;

                if (kk == 0) {
                    newN.next.add(new AtomicMarkableReference<>(n2,
                            false));
                    kk++;
                } else {
                    newN.next.set(i, new AtomicMarkableReference<>(n2,
                            false));
                }
                if (newN.data.isMarked())
                    break;
                if (n1.next.get(i).compareAndSet(n2, newN, false, false)) {
                    break;
                }
            }
        }

        newN.validLevel = level;

        if (newN.data.isMarked())
            newN = helpDelete(newN, 0);

        return true;
    }

    @Override
    public boolean offer(E e) {
        return add(e);
    }

    @Override
    public E poll() {
        return deleteMin();
    }

    @Override
    public E remove() {
        E x = poll();
        if (x != null)
            return x;
        else
            throw new NoSuchElementException();
    }

    @Override
    public E peek() {
        return head.next.get(0).getReference().data.getReference();
    }

    @Override
    public E element() {
        E x = peek();
        if (x != null)
            return x;
        else
            throw new NoSuchElementException();
    }

    @Override
    public boolean isEmpty() {
        return head.next.get(0).getReference() == tail;
    }

    @Override
    public int size() {
        int size = 0;
        Iterator<E> itr = iterator();
        while (itr.hasNext()) {
            itr.next();
            size++;
        }
        return size;
    }

    @Override
    public boolean contains(Object o) {
        E element;
        Iterator<E> itr = new PQueueIterator();
        while (itr.hasNext()) {
            element = itr.next();
            if (element.equals(o)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public Iterator<E> iterator() {
        return new PQueueIterator();
    }


    // ========== helpers ==========

    private int randomLevel() {
        int v = 1;

        while ((RAND_GEN.nextDouble() < 0.50) && (v < MAXLEVEL - 1)) {
            v = v + 1;
        }

        return v;
    }

    private E deleteMin() {
        Node<E> n2, last, prev;
        Node<E> n1 = null;
        E val;
        int i = 0;
        NodePair<E> tn1, tn2;
        AtomicMarkableReference<Node<E>> tempn1;
        int iflag = 0;

        prev = head;

        retry: while (true) {
            if (iflag != 1) {
                tn1 = readNext(prev, 0);
                n1 = tn1.n2;
                prev = tn1.n1;
                if (n1 == tail)
                    return null;
            }
            iflag = 0;

            val = n1.data.getReference();
            if (!n1.data.isMarked()) {
                if (n1.data.compareAndSet(n1.data.getReference(), n1.data
                        .getReference(), false, true)) {
                    n1.prev = prev;
                    break;
                } else {
                    iflag = 1;
                    continue retry;
                }
            } else if (n1.data.isMarked()) {
                n1 = helpDelete(n1, 0);
            }
            prev = n1;
        }
        for (i = 0; i <= n1.validLevel - 1; i++) {
            do {
                tempn1 = n1.next.get(i);
                n2 = tempn1.getReference();
            } while (!tempn1.compareAndSet(n2, n2, false, true)
                    && !tempn1.isMarked());
        }

        prev = head;

        for (i = n1.validLevel - 1; i >= 0; i--) {
            Node<E> tmpN = n1.next.get(i).getReference();
            while (true) {
                if (n1.next.get(i).getReference() == null)
                    break;
                tn2 = scanKey(prev, i, n1);
                last = tn2.n2;
                prev = tn2.n1;

                if (((last != n1) || (n1.next.get(i).getReference() == null)))
                    break;

                if (!tmpN.data.isMarked()) {
                    if (prev.next.get(i).compareAndSet(n1, tmpN, false, false)) {
                        n1.next.get(i).set(null, true);
                        break;
                    }
                } else
                    tmpN = tmpN.next.get(i).getReference();

                if (n1.next.get(i).getReference() == null)
                    break;
            }
        }

        return val;
    }

    private Node<E> helpDelete(Node<E> n, int ll) {
        Node<E> prev, last, n2;
        NodePair<E> tn1, tn2;
        AtomicMarkableReference<Node<E>> tempn1;

        // Mark all the next pointers of the node to be deleted
        for (int i = ll; i <= n.validLevel - 1; i++) {
            do {
                tempn1 = n.next.get(i);
                n2 = tempn1.getReference();
            } while (!tempn1.compareAndSet(n2, n2, false, true)
                    && !tempn1.isMarked());
        }

        // Get the previous pointer
        prev = n.prev;
        if ((prev == null) || (ll >= prev.validLevel)) {
            prev = head;
            for (int i = MAXLEVEL - 1; i >= ll; i--) {
                tn1 = scanKey(prev, i, n);
                n2 = tn1.n2;
                prev = tn1.n1;
            }
        }

        Node<E> tmpN = n.next.get(ll).getReference();

        while (true) {
            if (n.next.get(ll).getReference() == null)
                break;
            tn2 = scanKey(prev, ll, n);
            last = tn2.n2;
            prev = tn2.n1;
            if (((last != n) || (n.next.get(ll).getReference() == null)))
                break;

            if (!tmpN.data.isMarked()) {
                if (prev.next.get(ll).compareAndSet(n, tmpN, false, false)) {
                    n.next.get(ll).set(null, true);
                    break;
                }
            } else
                tmpN = tmpN.next.get(ll).getReference();

            if (n.next.get(ll).getReference() == null)
                break;
        }

        return prev;
    }

    private Node<E> readNode(Node<E> n, int ll) {
        if (n.next.get(ll).isMarked())
            return null;
        else
            return n.next.get(ll).getReference();
    }

    private NodePair<E> readNext(Node<E> n1, int ll) {
        Node<E> n2, nn2;

        if (n1.data.isMarked()) {
            nn2 = helpDelete(n1, ll);
            n1 = nn2;
        }

        n2 = readNode(n1, ll);
        while (n2 == null) {
            nn2 = helpDelete(n1, ll);
            n1 = nn2;
            n2 = readNode(n1, ll);
        }

        return new NodePair<>(n1, n2);
    }

    private NodePair<E> scanKey(Node<E> n1, int ll, Node<E> nk) {
        Node<E> n2;
        NodePair<E> tn1, tn2;

        tn1 = readNext(n1, ll);
        n1 = tn1.n1;
        n2 = tn1.n2;

        while ((compare(n2.data.getReference(), nk.data.getReference()) < 0)
                && (n2 != tail)) {
            n1 = n2;
            tn2 = readNext(n1, ll);
            n1 = tn2.n1;
            n2 = tn2.n2;
        }

        return new NodePair<>(n1, n2);
    }

    /**
     * @return -1 if k1 is less than k2; 0 if k1 equals k2; otherwise 1
     */
    private int compare(E k1, E k2) {
        if ((k1 == null) && (k2 == null))
            return 0;
        if (k1 == null)
            return -1;
        else if (k2 == null)
            return 1;
        else {
            return k1.compareTo(k2);
        }
    }
}
