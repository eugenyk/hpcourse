package ru.spbau.svidchenko.hpcourse.hw002;

import java.util.concurrent.atomic.AtomicMarkableReference;

/**
 * @author ArgentumWalker
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private final SetNode tail = new SetNode(null);
    private final SetNode head = new SetNode(null, tail);


    public boolean add(T value) {
        SetNode previousNode = head;
        SetNode currentNode = head.getNext().getReference();

        SetNode newNode = new SetNode(value);
        while (true) {
            if (currentNode != tail && currentNode.getNext().isMarked()) {
                //If previous.next node marked, then previous node already deleted and we should start again
                //If previous.next node is not current node, then it already changed and we should start again
                previousNode.getNext().compareAndSet(currentNode, currentNode.getNext().getReference(),
                        false, false);
                previousNode = head;
                currentNode = head.getNext().getReference();
                continue;
            }
            if (currentNode != tail) {
                switch (currentNode.getValue().compareTo(value)) {
                    case 0:
                        return false;
                    case -1: {
                        //Make step
                        previousNode = currentNode;
                        currentNode = currentNode.getNext().getReference();
                        continue;
                    }
                }
            }
            newNode.setNext(currentNode);

            //If previous.next node marked, then previous node already deleted and we should start again
            //If previous.next node is not current node, then it already changed and we should start again
            if (previousNode.getNext().compareAndSet(currentNode, newNode, false, false)) {
                return true;
            }
            previousNode = head;
            currentNode = head.getNext().getReference();
        }
    }

    public boolean remove(T value) {
        SetNode previousNode = head;
        SetNode currentNode = head.getNext().getReference();

        while (true) {
            if (currentNode != tail && currentNode.getNext().isMarked()) {
                //If previous.next node marked, then previous node already deleted and we should start again
                //If previous.next node is not current node, then it already changed and we should start again
                previousNode.getNext().compareAndSet(currentNode, currentNode.getNext().getReference(),
                        false, false);
                previousNode = head;
                currentNode = head.getNext().getReference();
                continue;
            }
            if (currentNode != tail) {
                switch (currentNode.getValue().compareTo(value)) {
                    case 1:
                        return false;
                    case -1: {
                        //Make step
                        previousNode = currentNode;
                        currentNode = currentNode.getNext().getReference();
                        continue;
                    }
                }
                //If marked successfully then we can return
                if (currentNode.getNext().attemptMark(currentNode.getNext().getReference(), true)) {
                    return true;
                } else {
                    previousNode = head;
                    currentNode = head.getNext().getReference();
                    continue;
                }
            }
            return false;
        }
    }

    public boolean contains(T value) {
        SetNode previousNode = head;
        SetNode currentNode = head.getNext().getReference();
        while (true) {
            if (currentNode != tail && currentNode.getNext().isMarked()) {
                //If previous.next node marked, then previous node already deleted and we should start again
                //If previous.next node is not current node, then it already changed and we should start again
                previousNode.getNext().compareAndSet(currentNode, currentNode.getNext().getReference(),
                        false, false);
                previousNode = head;
                currentNode = head.getNext().getReference();
                continue;
            }
            if (currentNode != tail) {
                switch (currentNode.getValue().compareTo(value)) {
                    case 0:
                        return true;
                    case -1: {
                        //Make step
                        previousNode = currentNode;
                        currentNode = currentNode.getNext().getReference();
                        continue;
                    }
                }
            }
            return false;
        }
    }

    public boolean isEmpty() {
        SetNode previousNode = head;
        SetNode currentNode = head.getNext().getReference();
        while (true) {
            if (currentNode != tail && currentNode.getNext().isMarked()) {
                //If previous.next node marked, then previous node already deleted and we should start again
                //If previous.next node is not current node, then it already changed and we should start again
                previousNode.getNext().compareAndSet(currentNode, currentNode.getNext().getReference(),
                        false, false);
                previousNode = head;
                currentNode = head.getNext().getReference();
                continue;
            }
            return currentNode == tail;
        }
    }

    private class SetNode {
        private T value;
        private final AtomicMarkableReference<SetNode> next = new AtomicMarkableReference<SetNode>(this, false);

        public SetNode(T value) {
            this.value = value;
        }

        public SetNode(T value, SetNode next) {
            this.value = value;
            this.next.set(next, false);
        }

        public T getValue() {
            return value;
        }

        public AtomicMarkableReference<SetNode> getNext() {
            return next;
        }

        public void setNext(SetNode node) {
            next.set(node, false);
        }
    }
}
