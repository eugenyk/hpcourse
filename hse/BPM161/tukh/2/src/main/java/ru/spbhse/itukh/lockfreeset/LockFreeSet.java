package ru.spbhse.itukh.lockfreeset;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.atomic.AtomicMarkableReference;

/***
 * Implementation of a LockFreeSetInterface. Based on a linked list.
 * @param <T> is type of value to store.
 */
public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    /**
     * Node of list. Next is a reference to the next node in a list with a mark, which indicates that element
     * is to be deleted.
     */
    private class ListNode {
        private T value;
        AtomicMarkableReference<ListNode> next = new AtomicMarkableReference<>(null, false);

        ListNode(T value) {
            this.value = value;
        }
    }

    private class ListNodePair {
        ListNode first;
        ListNode second;

        ListNodePair(ListNode first, ListNode second) {
            this.first = first;
            this.second = second;
        }

        void toNext() {
            this.first = this.second;
            this.second = this.second.next.getReference();
        }
    }

    /**
     * Head.next refers to a first ListNode of out list (set), tail is a fictive last element.
     */
    private ListNode head;
    private ListNode tail;

    LockFreeSet() {
        head = new ListNode(null);
        tail = new ListNode(null);
        head.next.set(tail, false);
    }

    @Override
    public boolean add(T value) {
        ListNode newNode = new ListNode(value);

        while (true) {
            ListNodePair nodesPair = lookUp(value);

            if (nodesPair.second.value == value && !nodesPair.second.next.isMarked()) {
                return false;
            }

            if (nodesPair.second != tail) {
                if (nodesPair.second.value == value) {
                    ListNode nextNode = nodesPair.second.next.getReference();
                    ListNode first = nodesPair.first;
                    boolean mark = first.next.isMarked();
                    first.next.compareAndSet(nodesPair.second, nextNode, mark, mark);
                }
                continue;
            }

            ListNode first = nodesPair.first;
            ListNode second = nodesPair.second;
            newNode.next.set(second, false);
            if (first.next.compareAndSet(second, newNode, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            ListNodePair nodesPair = lookUp(value);
            ListNode nextNode = nodesPair.second.next.getReference();

            if (nodesPair.second != tail) {
                ListNode second = nodesPair.second;
                if (second.next.compareAndSet(nextNode, nextNode, false, true)) {
                    tryToRemove(nodesPair.first, second);
                    return true;
                }
                continue;
            }
            return false;
        }
    }

    @Override
    public boolean contains(T value) {
        ListNode current = head.next.getReference();
        ListNode previous = head;
        while (current != tail) {
            T curretValue = current.value;

            if (curretValue == value && !current.next.isMarked()) {
                return true;
            } else if (curretValue == value) {
                tryToRemove(previous, current);
                return false;
            }

            previous = current;
            current = current.next.getReference();
        }

        return false;
    }

    @Override
    public boolean isEmpty() {
        while (head.next.getReference() != tail) {
            ListNode currentFirst = head.next.getReference();
            if (!currentFirst.next.isMarked()) {
                return false;
            }

            head.next.compareAndSet(currentFirst, currentFirst.next.getReference(), false, false);
        }

        return true;
    }

    @Override
    public Iterator<T> iterator() {
        while (true) {
            List<T> content = getListContent();
            List<T> contentLater = getListContent();

            if (content.equals(contentLater)) {
                return content.iterator();
            }
        }
    }

    private List<T> getListContent() {
        ListNode current = head.next.getReference();
        ArrayList<T> content = new ArrayList<>();

        for (; current != tail; current = current.next.getReference()) {
            if (!current.next.isMarked()) {
                content.add(current.value);
            }
        }

        return content;
    }

    /**
     * Searches value in a list. If finds, returns (previous, found node) and (..., tail) otherwise.
     *
     * @param value is a value to look up.
     */
    private ListNodePair lookUp(T value) {
        ListNodePair current = new ListNodePair(head, head.next.getReference());

        while (current.second != tail) {
            ListNode nextNode = current.second.next.getReference();

            if (current.second.next.isMarked()) {
                if (current.first.next.compareAndSet(current.second, nextNode, false, false)) {
                    current.second = nextNode;
                    continue;
                }
                break;
            }

            if (current.second.value == value) {
                return current;
            }

            current.toNext();
        }

        return current;
    }

    private void tryToRemove(ListNode previousNode, ListNode nodeToRemove) {
        ListNode nextNode = nodeToRemove.next.getReference();
        while (nextNode != tail && nextNode.next.isMarked()) {
            nextNode = nextNode.next.getReference();
        }
        boolean mark = previousNode.next.isMarked();
        previousNode.next.compareAndSet(nodeToRemove, nextNode, mark, mark);
    }
}
