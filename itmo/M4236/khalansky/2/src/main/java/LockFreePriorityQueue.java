package ru.ifmo.ct.khalansky.hpcourse.queue;
import java.util.concurrent.atomic.*;
import java.util.*;

public class LockFreePriorityQueue<E extends Comparable<E>>
extends AbstractQueue<E> implements PriorityQueue<E> {

    private static class Node<E> {
        AtomicReference<Node<E>> next = new AtomicReference<>();
        AtomicBoolean isTaken = new AtomicBoolean(false);
        E value;

        boolean wasConsumed() {
            return isTaken.get();
        }
    }

    private Node<E> headNode = new Node<E>();

    static private<T> boolean setsEqual(Set<T> set1, Set<T> set2) {
        if (set1.size() != set2.size()) {
            return false;
        }
        for (T el : set1) {
            if (!set2.contains(el)) {
                return false;
            }
            set2.remove(el);
        }
        return true;
    }

    public boolean offer(E e) {
        if (e == null) {
            return false;
        }

        Node<E> node = new Node<>();
        node.value = e;
        while (true) {
            Node<E> curr = headNode;

            /* The invariant is that we know for sure that the node can't be
               inserted before `curr`.

               We leave this loop either by returning from the function, which
               indicates that we inserted successfully, or by breaking from it,
               which indicates that an attempt at inserting the value at some
               position failed and we need to start anew.
            */
            while (true) {
                Node<E> next = curr.next.get();
                /* Clean up bad nodes that we encounter. */
                while (next != null && next.wasConsumed()) {
                    /* We try throwing away the nodes that have been consumed.
                       It doesn't matter if we succeed: if we don't, it means
                       that either someone has thrown the value away already
                       or a new value was inserted. Either way, we keep cleaning
                       up while we can. */
                    curr.next.compareAndSet(next, next.next.get());
                    next = curr.next.get();
                }
                if (next == null || e.compareTo(next.value) <= 0) {
                    /* This is the place where we want to insert the new node:
                       either we're at the end of the list and, by the loop
                       invariant, are bigger than any of the existing values,
                       or we're less than or equal to the next value.
                    */
                    node.next.set(next);

                    /* Try inserting the value. */
                    if (curr.next.compareAndSet(next, node)) {
                        /* It succeeded. But what if the value that we attached
                           to had been removed from the queue? We need to check
                           whether the value ever got into the list. For this,
                           we traverse through the list, looking for our node.
                           We also stop the check if it turns out that our value
                           has been consumed.
                        */
                        Node<E> checker = headNode;
                        while (checker != null) {
                            if (checker == node || node.wasConsumed()) {
                                return true;
                            }
                            if (checker.value != null &&
                                e.compareTo(checker.value) < 0
                            ) {
                                break;
                            }
                            checker = checker.next.get();
                        }
                    }
                    break;
                }
                curr = next;
            }
        }
    }

    /* Return a node for which it is true that at some point of time since
       calling this method the node wasn't taken by anyone and was the first in
       the queue.
    */
    private static<T> Node<T> nextForTaking(Node<T> head) {
        Node<T> next = head.next.get();
        /* We remove values that have been taken while we can. When we can't, it
           means that either we found a value that hasn't been taken or that the
           end of the queue has been reached. */
        while (next != null && next.wasConsumed()) {
            /* See a similar loop in `offer` for an explanation why we don't
               need to know whether throwing a value away succeeded.
            */
            head.next.compareAndSet(next, next.next.get());
            next = head.next.get();
        }
        return next;
    }

    public E peek() {
        /* At some point of time since starting this method, this value--or
           lack thereof--was at the beginning of the queue.
        */
        Node<E> toTake = nextForTaking(headNode);
        return toTake == null ? null : toTake.value;
    }

    public E poll() {
        while (true) {
            Node<E> toTake = nextForTaking(headNode);
            if (toTake == null) {
                return null;
            }
            if (toTake.isTaken.compareAndSet(false, true)) {
                return toTake.value;
            }
        }
    }

    /* This is a method that will most probably just hang. */
    public int size() {
        /* Length will be output if two passes through the full list go through
           the same nodes. */
        List<HashSet<Node<E>>> seen = new ArrayList<HashSet<Node<E>>>();
        seen.add(new HashSet<Node<E>>());
        seen.add(new HashSet<Node<E>>());
        while (true) {
            for (int i = 0; i < 2; ++i) {
                seen.get(i).clear();
                Node<E> curr = headNode;
                while (curr != null) {
                    Node<E> next = curr.next.get();
                    while (next != null && next.wasConsumed()) {
                        curr.next.compareAndSet(next, next.next.get());
                        next = curr.next.get();
                    }
                    seen.get(i).add(curr);
                    curr = next;
                }
            }
            if (setsEqual(seen.get(0), seen.get(1))) {
                return seen.get(0).size()-1;
            }
        }
    }

    public boolean isEmpty() {
        return nextForTaking(headNode) == null;
    }

    public Iterator<E> iterator() {
        return new Iterator<E>() {

            Node<E> last = headNode;

            public boolean hasNext() {
                return last != null && nextForTaking(last) != null;
            }

            public E next() {
                last = nextForTaking(last);
                if (last == null) {
                    throw new NoSuchElementException();
                }
                return last.value;
            }

        };
    }

}
