package ru.sofysmo.priorityqueue;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>>
        extends AbstractQueue<E> implements PriorityQueue<E> {

    private final ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> tail = new ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<>(null, null);
    private final ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> head = new ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<>(null, tail);
    private static final int maxAttempt = 100;


    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> currenct = head;
        int i = 0;
        while (currenct.getNext() != tail){
            if (!currenct.nextIsMarked()){
                i++;
            }
            currenct = currenct.getNext();
        }
        return i;
    }

    @Override
    public boolean offer(E e) {
        int i = 0;
        while (i < maxAttempt) {
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Position<E> pair = findPosition(e);
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> previous = pair.getLeft();
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> next = pair.getRight();
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> newNode = new ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<>(e, next);
            if(previous.compareAndSetNext(next, newNode)) {
                return true;
            }
            i++;
        }
        return false;
    }
    @Override
    public E poll() {
        int i = 0;
        while (i < maxAttempt) {
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> position = getFirst();
            if(position == tail) {
                return null;
            }
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> nextMin = position.getNext();
            if(position.markDeleted(nextMin, nextMin)) {
                head.compareAndSetNext(position, nextMin);
                return position.getValue();
            }
            i++;
        }
        return null;
    }

    @Override
    public E peek() {
        return getFirst().getValue();
    }

    @Override
    public boolean isEmpty() {
        return peek() == null;
    }

    private ru.sofysmo.priorityqueue.LockFreePriorityQueue.Position<E> findPosition(E e) {
        while(true) {
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> left = head;
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> right;
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> current = head.getNext();
            boolean[] flagDeletion = {false};

            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> tmpNode = head;
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> tmpNodeNext = head.getNext(flagDeletion);
            do {
                if (!flagDeletion[0]) {
                    left = tmpNode;
                    current = tmpNodeNext;
                }
                tmpNode = tmpNodeNext;
                tmpNodeNext = tmpNode.getNext(flagDeletion);
            } while (tmpNode != tail && (flagDeletion[0] || tmpNode.getValue().compareTo(e) <= 0));

            right = tmpNode;
            if(left.compareAndSetNext(current, right)) {
                if(right == tail || !right.nextIsMarked()) {
                    return new ru.sofysmo.priorityqueue.LockFreePriorityQueue.Position<>(left, right);
                }
            }
        }
    }

    private ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> getFirst() {
        ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> left = head;
        boolean[] flagDeletion = {false};
        while (true) {
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> current = left.getNext();
            ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> next = current.getNext(flagDeletion);
            if(flagDeletion[0]) {
                left.compareAndSetNext(current, next);
                continue;
            }
            return current;
        }
    }

    private static class Node<E extends Comparable<E>> {
        private final E value;
        private AtomicMarkableReference<ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E>> next;

        Node(E value, ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> next) {
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }

        E getValue() {
            return value;
        }

        boolean markDeleted(ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> nextOld, ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> nextNew) {
            return next.compareAndSet(nextOld, nextNew, false, true);
        }

        boolean compareAndSetNext(ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> nextOld, ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> nextNew) {
            return next.compareAndSet(nextOld, nextNew, false, false);
        }

        ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> getNext() {
            return next.getReference();
        }

        ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> getNext(boolean flagDeletion[]) {
            return next.get(flagDeletion);
        }

        boolean nextIsMarked() {
            return next.isMarked();
        }
    }

    private static class Position<E extends Comparable<E>> {
        private final ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> left;
        private final ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> right;

        Position(ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> leftNode, ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> rightNode) {
            left = leftNode;
            right = rightNode;
        }

        ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> getLeft() {
            return left;
        }

        ru.sofysmo.priorityqueue.LockFreePriorityQueue.Node<E> getRight() {
            return right;
        }
    }
}