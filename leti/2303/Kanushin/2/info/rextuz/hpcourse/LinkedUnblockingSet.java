package info.rextuz.hpcourse;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LinkedUnblockingSet<T extends Comparable<T>> implements LockFreeSet<T> {
    private Node head;
    private Node tail;

    /**
     * A common constructor.
     */
    public LinkedUnblockingSet() {
        /* init tail with the biggest key */
        tail = new Node();
        tail.key = Integer.MAX_VALUE;

        /* init head with the smallest key */
        head = new Node();
        head.key = Integer.MIN_VALUE;
        head.next = new AtomicMarkableReference<>(tail, false);
    }

    /**
     * Searches for the exact place for the key
     * in the list. Returns two nodes
     * - neighbours, in between which a node
     * with the given key should be located.
     */
    private PCNodes find(int key) {
        retry:
        while (true) {
            Node pred = head;
            Node curr = pred.next.getReference();

            if (curr == tail)
                return new PCNodes(pred, tail);

            while (true) {
                if (curr == tail)
                    return new PCNodes(pred, tail);

                boolean[] marked = new boolean[1];
                Node succ = curr.next.get(marked);
                while (marked[0]) {
                    boolean deleted = pred.next.compareAndSet(curr, succ, false, false);
                    if (!deleted)
                        continue retry;
                    curr = succ;
                    succ = curr.next.get(marked);
                }
                if (curr.key >= key)
                    return new PCNodes(pred, curr);

                pred = curr;
                curr = succ;
            }
        }
    }

    /**
     * Adds item to the list.
     * Tries to insert the node into found place.
     * If the key exists, returns False. Otherwise
     * continues until is able to insert.
     */
    @Override
    public boolean add(T value) {
        int key = value.hashCode();
        while (true) {
            PCNodes PCNodes = find(key);
            Node pred = PCNodes.pred;
            Node curr = PCNodes.curr;

            if (curr.key == key)
                return false;

            Node node = new Node(value);
            node.next = new AtomicMarkableReference<>(curr, false);
            if (pred.next.compareAndSet(curr, node, false, false))
                return true;
        }
    }

    /**
     * Removes item from the list.
     * If the key is not found returns false.
     * Otherwise continues until is able to
     * mark the node.
     */
    @Override
    public boolean remove(T value) {
        int key = value.hashCode();

        while (true) {
            PCNodes PCNodes = find(key);
            Node pred = PCNodes.pred;
            Node curr = PCNodes.curr;

            if (curr.key != key)
                return false;

            Node succ = curr.next.getReference();
            boolean marked = curr.next.attemptMark(succ, true);
            if (!marked)
                continue;

            pred.next.compareAndSet(curr, succ, false, false);
            return true;
        }
    }

    /**
     * Walks through the elements of the set while
     * the given key is reached.
     * Returns True if the found element's key matches
     * and if it is not marked as logically deleted.
     * Otherwise returns False.
     */
    @Override
    public boolean contains(T value) {
        boolean[] marked = new boolean[1];
        int key = value.hashCode();
        Node curr = head;

        while (curr.key < key) {
            curr = curr.next.getReference();

            if (curr == tail)
                return false;

            curr.next.get(marked);
        }

        return (curr.key == key && !marked[0]);
    }

    /**
     * Walks through the elements of the set and deletes
     * ones that are marked as logically deleted.
     * If an element is found that is not marked as
     * deleted then returns False, otherwise returns True;
     */
    @Override
    public boolean isEmpty() {
        while (true) {
            Node pred = head;
            Node curr = head.next.getReference();
            if (curr == tail)
                return true;

            boolean[] marked = new boolean[1];
            Node succ = curr.next.get(marked);
            if (!marked[0])
                return false;

            pred.next.compareAndSet(curr, succ, true, false);
        }
    }

    /**
     * Node of the list.
     */
    private class Node {
        T item;     // value
        int key;    // key
        AtomicMarkableReference<Node> next; // atomic next ptr

        /* Common constructor */
        Node(T item) {
            this.item = item;
            this.key = item.hashCode();
        }

        /* head/tail constructor */
        private Node() {
        }
    }

    /**
     * A class that stores two nodes.
     * Used to return a value from find().
     */
    private class PCNodes {
        Node pred, curr;

        PCNodes(Node pred, Node curr) {
            this.pred = pred;
            this.curr = curr;
        }
    }

}
