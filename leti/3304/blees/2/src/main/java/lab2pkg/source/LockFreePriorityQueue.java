/*
    Algorithm from:
    Sundell H., Tsigas P. Fast and lock-free concurrent priority queues for multi-thread systems
    //Journal of Parallel and Distributed Computing. – 2005. – Т. 65. – №. 5. – С. 609-627.

    https://web.archive.org/web/20170811000106/http://www.non-blocking.com/download/SunT03_PQueue_TR.pdf
*/

package lab2pkg.source;

import java.util.*;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {
    private static final int MAX_LEVEL = 10;
    private static final Random RANDOM = new Random();

    private static class Node<E> {
        AtomicMarkableReference<E> value;
        Node<E> prev;
        ArrayList<AtomicMarkableReference<Node<E>>> next;
        int level, validLevel;

        Node() {
            this.value = new AtomicMarkableReference<>(null, false);
            this.prev = null;
            this.next = new ArrayList<>(MAX_LEVEL);
            this.validLevel = -1;
        }

        Node(int l, E d) {
            this.value = new AtomicMarkableReference<>(d, false);
            this.next = new ArrayList<>(MAX_LEVEL);
            this.level = l;
        }
    }

    private static class NodeTuple<E> {
        Node<E> n1;
        Node<E> n2;

        NodeTuple(Node<E> _n1, Node<E> _n2) {
            n1 = _n1;
            n2 = _n2;
        }
    }

    private class PQueueIterator implements Iterator<E> {
        Node<E> cursor = head.next.get(0).getReference();

        public E next() {
            if (cursor == tail)
                throw new NoSuchElementException();

            E result = cursor.value.getReference();
            cursor = cursor.next.get(0).getReference();
            return result;
        }

        public boolean hasNext() {
            return cursor != tail;
        }
    }

    private Node<E> head = new Node<>();
    private Node<E> tail = new Node<>();

    public LockFreePriorityQueue() {
        for (int i = 0; i < MAX_LEVEL; i++) {
            head.next.add(new AtomicMarkableReference<>(tail, false));
        }
    }

    @Override
    public boolean add(E e) {
        int level, i;

        Node<E> newNode;
        Node<E> n1, n2;
        NodeTuple<E> t1, t2, t3;
        ArrayList<Node<E>> savedNode = new ArrayList<>();

        level = randomLevel();
        newNode = new Node<>(level, e);
        n1 = head;

        for (int j = 0; j < MAX_LEVEL; j++) {
            savedNode.add(new Node<>());
        }

        for (i = MAX_LEVEL - 1; i >= 1; i--) {
            t1 = scanKey(n1, i, newNode);
            n2 = t1.n2;
            n1 = t1.n1;
            if (i < level)
                savedNode.set(i, n1);
        }

        int k = 0;
        while (true) {
            t2 = scanKey(n1, 0, newNode);
            n2 = t2.n2;
            n1 = t2.n1;

            if ((compare(e, n2.value.getReference()) == 0) && (!n2.value.isMarked())) {
                if (n2.value.compareAndSet(n2.value.getReference(), e, false, false))
                    return true;
                else
                    continue;
            }

            if (k == 0) {
                newNode.next.add(new AtomicMarkableReference<>(n2, false));
                k++;
                newNode.validLevel = 0;
            } else
                newNode.next.set(0, new AtomicMarkableReference<>(n2, false));

            if (n1.next.get(0).compareAndSet(n2, newNode, false, false))
                break;
        }

        for (i = 1; i <= level - 1; i++) {
            newNode.validLevel = i;
            n1 = savedNode.get(i);
            k = 0;
            while (true) {
                t3 = scanKey(n1, i, newNode);
                n2 = t3.n2;
                n1 = t3.n1;

                if (k == 0) {
                    newNode.next.add(new AtomicMarkableReference<>(n2, false));
                    k++;
                } else
                    newNode.next.set(i, new AtomicMarkableReference<>(n2, false));

                if (newNode.value.isMarked())
                    break;
                if (n1.next.get(i).compareAndSet(n2, newNode, false, false))
                    break;
            }
        }

        newNode.validLevel = level;

        if (newNode.value.isMarked())
            newNode = helpDelete(newNode, 0);

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
        return head.next.get(0).getReference().value.getReference();
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
        Iterator<E> it = iterator();
        while (it.hasNext()) {
            it.next();
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
            if (element.equals(o))
                return true;
        }
        return false;
    }

    @Override
    public Iterator<E> iterator() {
        return new PQueueIterator();
    }

    private int randomLevel() {
        int v = 1;
        while ((RANDOM.nextDouble() < 0.50) && (v < MAX_LEVEL - 1)) {
            v = v + 1;
        }
        return v;
    }

    private E deleteMin() {
        Node<E> n1 = null;
        Node<E> n2, last, prev;
        NodeTuple<E> tn1, tn2;
        E val;
        int i = 0;
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

            val = n1.value.getReference();
            if (!n1.value.isMarked()) {
                if (n1.value.compareAndSet(n1.value.getReference(), n1.value.getReference(), false, true)) {
                    n1.prev = prev;
                    break;
                } else {
                    iflag = 1;
                    continue retry;
                }
            } else if (n1.value.isMarked())
                n1 = helpDelete(n1, 0);
            prev = n1;
        }
        for (i = 0; i <= n1.validLevel - 1; i++) {
            do {
                tempn1 = n1.next.get(i);
                n2 = tempn1.getReference();
            } while (!tempn1.compareAndSet(n2, n2, false, true) && !tempn1.isMarked());
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

                if (!tmpN.value.isMarked()) {
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
        NodeTuple<E> tn1, tn2;
        AtomicMarkableReference<Node<E>> tempn1;

        // Mark all the next pointers for delete
        for (int i = ll; i <= n.validLevel - 1; i++) {
            do {
                tempn1 = n.next.get(i);
                n2 = tempn1.getReference();
            } while (!tempn1.compareAndSet(n2, n2, false, true) && !tempn1.isMarked());
        }

        // Get the previous pointer
        prev = n.prev;
        if ((prev == null) || (ll >= prev.validLevel)) {
            prev = head;
            for (int i = MAX_LEVEL - 1; i >= ll; i--) {
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

            if (!tmpN.value.isMarked()) {
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

    private NodeTuple<E> readNext(Node<E> n1, int ll) {
        Node<E> n2, nn2;

        if (n1.value.isMarked()) {
            nn2 = helpDelete(n1, ll);
            n1 = nn2;
        }

        n2 = readNode(n1, ll);
        while (n2 == null) {
            nn2 = helpDelete(n1, ll);
            n1 = nn2;
            n2 = readNode(n1, ll);
        }

        return new NodeTuple<>(n1, n2);
    }

    private NodeTuple<E> scanKey(Node<E> n1, int ll, Node<E> nk) {
        Node<E> n2;
        NodeTuple<E> tn1, tn2;

        tn1 = readNext(n1, ll);
        n1 = tn1.n1;
        n2 = tn1.n2;

        while ((compare(n2.value.getReference(), nk.value.getReference()) < 0) && (n2 != tail)) {
            n1 = n2;
            tn2 = readNext(n1, ll);
            n1 = tn2.n1;
            n2 = tn2.n2;
        }

        return new NodeTuple<>(n1, n2);
    }

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
