package com.lockfree;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private final Node<T> head;
    private final Node<T> tail;

    LockFreeSetImpl() {
        tail = new Node<>(null);
        head = new Node<>(null);
        head.next.set(tail, false);
    }

    static class Node<T> {
        final T value;
        //mark stores state for current node, not for node to which it refer
        final AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);

        Node(T value) {
            this.value = value;
        }
    }

    //class for returning two nodes together
    static class NodesPair<T> {
        Node<T> prev;
        Node<T> cur;

        NodesPair(Node<T> prev, Node<T> cur) {
            this.prev = prev;
            this.cur = cur;
        }
    }

    public boolean add(T value) {
        final Node<T> newNode = new Node<>(value);
        while (true) {
            //search for value in sorted list
            final NodesPair<T> nodesPair = search(value);
            final Node<T> prev = nodesPair.prev;
            final Node<T> cur = nodesPair.cur;

            //if it's already in list then return
            if (cur.value == value) {
                return false;
            } else {
                //set reference from new node to current
                newNode.next.set(cur, false);
                //try to set reference from previous node to new if now it refer to current node and not marked
                if (prev.next.compareAndSet(cur, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    public boolean remove(T value) {
        while (true) {
            //search for value in sorted list
            final NodesPair<T> nodesPair = search(value);
            //final Node<T> prev = nodesPair.prev;
            final Node<T> cur = nodesPair.cur;
            //if found value is different then return
            if (cur.value != value) {
                return false;
            }
            final Node<T> next = cur.next.getReference();
            //if next not changed then mark current, else - retry
            if (cur.next.compareAndSet(next, next, false, true)) {
                //remove current - make previous refer to next, if now it refer to current
                //it's optional instruction because removal anyway will be done during search
                //prev.next.compareAndSet(cur, next, false, false);
                return true;
            }
        }
    }

    //returns pair of neighbour nodes
    private NodesPair<T> search(T value) {
        Node<T> prev;
        Node<T> cur;
        Node<T> next;
        //when trying to get next node, mark will be here
        //mark means that current element should be deleted
        boolean[] curMark = {false};

        // If set is empty
        if (head.next.getReference() == tail) {
            return new NodesPair<>(head, tail);
        }

        retry:
        while (true) {
            prev = head;
            cur = prev.next.getReference();
            //loop through all nodes in list
            while (true) {
                next = cur.next.get(curMark);

                //remove marked nodes
                while (curMark[0]) {
                    //try to remove, if can't remove then retry
                    if (!prev.next.compareAndSet(cur, next, false, false)) {
                        continue retry;
                    }
                    //else go to next node and remove it if marked too
                    cur = next;
                    next = cur.next.get(curMark);
                }

                //if value found or place for insertion found or list tail reached
                if (cur == tail || value.compareTo(cur.value) <= 0) {
                    return new NodesPair<>(prev, cur);
                }
                prev = cur;
                cur = next;
            }
        }
    }

    public boolean contains(T value) {
        final NodesPair<T> nodesPair = search(value);
        final Node<T> cur = nodesPair.cur;

        //true if value found and not marked
        return (cur.value == value && !cur.next.isMarked());

/*      boolean[] marked = {false};
        Node<T> cur = head.next.getReference();
        cur.next.get(marked);
        //loop through all nodes fo sorted list to place of value
        while (cur != tail && value.compareTo(cur.value) > 0) {
            //get next node and its mark
            cur = cur.next.getReference();
            cur.next.get(marked);
        }
        //true if value found and not marked
        return (cur.value == value && !marked[0]);*/
    }

    public boolean isEmpty() {
        boolean[] prevMark = {false};
        Node<T> cur = head.next.getReference();

        //loop through all nodes
        while (cur != tail) {
            cur = cur.next.get(prevMark);
            //if found not removed element than set is not empty
            if (!prevMark[0]) return false;
        }

        //set is empty in other cases
        return true;
    }

    //just for test
    public java.util.ArrayList<T> getAll() {
        java.util.ArrayList<T> result = new java.util.ArrayList<>();
        Node<T> cur = head.next.getReference();
        //loop through all nodes
        while (cur != tail) {
            if (contains(cur.value)) result.add(cur.value);
            cur = cur.next.getReference();
        }
        return result;
    }
}
