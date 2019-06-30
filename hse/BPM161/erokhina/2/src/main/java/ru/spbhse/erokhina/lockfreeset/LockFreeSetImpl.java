package ru.spbhse.erokhina.lockfreeset;

import ru.spbhse.erokhina.utils.Pair;

import java.util.Comparator;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {
    private static class Node<T> {
        private T value;
        private AtomicMarkableReference<Node<T>> nextAndMark;

        private Node(T value, AtomicMarkableReference<Node<T>> nextAndMark) {
            this.value = value;
            this.nextAndMark = nextAndMark;
        }

        T getValue() {
            return value;
        }

        AtomicMarkableReference<Node<T>> getNextAndMark() {
            return nextAndMark;
        }
    }

    private Node<T> head = new Node<>(null, new AtomicMarkableReference<>(null, false));

    @Override
    public boolean add(T value) {
        while(true) {
            Pair<Node<T>, Node<T>> pair = find(value);
            Node<T> pred = pair.getFirst();
            Node<T> curr = pair.getSecond();

            if (curr != null && curr.getValue() == value) {
                return false;
            } else {
                Node<T> node = new Node<>(value, new AtomicMarkableReference<>(curr, false));
                if (pred.getNextAndMark().compareAndSet(curr, node, false, false)) {
                    return true;
                }
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Pair<Node<T>, Node<T>> pair = find(value);
            Node<T> pred = pair.getFirst();
            Node<T> curr = pair.getSecond();

            if (curr == head || curr == null || curr.getValue().compareTo(value) != 0) {
                return false;
            } else {
                Node<T> succ = curr.getNextAndMark().getReference();
                if (!curr.getNextAndMark().compareAndSet(succ, succ, false, true)) {
                    continue;
                }
                pred.getNextAndMark().compareAndSet(curr, succ, false, false);
                return true;
            }
        }
    }

    private Pair<Node<T>, Node<T>> find(T value) {
        while (true) {
            Node<T> pred = head, curr = pred.getNextAndMark().getReference();

            while (true) {
                if (curr == null) {
                    return new Pair<>(pred, null);
                }

                if (curr.getNextAndMark().isMarked()) { // If curr was logically deleted
                    if (!pred.getNextAndMark().compareAndSet(curr, curr.getNextAndMark().getReference(),
                            false, false)) {
                        break;
                    }
                } else {
                    if (curr != head && (curr.getValue().compareTo(value) == 0)) {
                        return new Pair<>(pred, curr);
                    }
                    pred = curr;
                }
                curr = curr.getNextAndMark().getReference();
            }
        }
    }

    @Override
    public boolean contains(T value) {
        Node<T> curr = head;

        while (curr != null && (curr == head || curr.getValue().compareTo(value) != 0)) {
            curr = curr.getNextAndMark().getReference();
        }

        return curr != null && curr != head && curr.getValue().compareTo(value) == 0
                && !curr.getNextAndMark().isMarked();
    }

    @Override
    public boolean isEmpty() {
        return !iterator().hasNext();
    }

    private Pair<List<T>, Node<T>> getListAndLastNode() {
        List<T> list = new ArrayList<>();
        Node<T> last = head;

        for (Node<T> curr = head.getNextAndMark().getReference(); curr != null;
                                                        curr = curr.getNextAndMark().getReference()) {
            if (curr.getNextAndMark().isMarked()) {
                continue;
            }

            list.add(curr.getValue());
            last = curr;
        }

        return new Pair<>(list, last);
    }

    private List<T> getSnapshot() {
        while (true) {
            Pair<List<T>, Node<T>> resBefore = getListAndLastNode();
            Pair<List<T>, Node<T>> resAfter = getListAndLastNode();

            List<T> nodesBefore = resBefore.getFirst();
            List<T> nodesAfter = resAfter.getFirst();

            Node<T> lastBefore = resBefore.getSecond();
            Node<T> lastAfter = resAfter.getSecond();

            if (nodesBefore.equals(nodesAfter) && lastBefore.equals(lastAfter)) {
                return nodesBefore;
            }
        }
    }

    @Override
    public Iterator<T> iterator() {
        List<T> snapshot = getSnapshot();
        snapshot.sort(Comparable::compareTo);

        return snapshot.iterator();
    }



}
