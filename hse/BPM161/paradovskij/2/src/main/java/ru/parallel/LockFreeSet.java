package ru.parallel;

import com.sun.tools.javac.util.Pair;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class LockFreeSet<T extends Comparable<T>> implements LockFreeSetInterface<T> {
    private Node<T> head = new Node<>(null);

    public boolean add(T value) {
        if (value == null) {
            return false;
        }

        Node<T> newNode = new Node<>(value);
        while (true) {
            List<Node<T>> pair = standPair(value);
            Node<T> prev = pair.get(0);
            Node<T> cur = pair.get(1);

            if (cur != null) {
                return false;
            }

            int nodeVersion = prev.getNodeVersion();
            int nextNodeVersion = nodeVersion == 0 ? 0 : nodeVersion + 1;
            if (prev.getLink().compareAndSet(null, newNode, nodeVersion, nextNodeVersion)) {
                return true;
            }
        }
    }

    public boolean remove(T value) {
        if (value == null) {
            return false;
        }
        while (true) {
            List<Node<T>> pair = standPair(value);
            Node<T> prev = pair.get(0);
            Node<T> cur = pair.get(1);

            if (cur == null) {
                return false;
            }

            int nodeVersion = cur.getNodeVersion();
            Node<T> next = cur.getNextNode();
            if (nodeVersion != 0 && cur.getLink().compareAndSet(next, next, nodeVersion, 0)) {
                return true;
            }
        }
    }

    private void delete(Node<T> prev, Node<T> cur, Node<T> next) {
        while (next != null && next.getNodeVersion() == 0) {
            next = next.getNextNode();
        }

        int nodeVersion = prev.getNodeVersion();
        if (nodeVersion != 0) {
            prev.getLink().compareAndSet(cur, next, nodeVersion, nodeVersion + 1);
        }
    }

    public boolean contains(T value) {
        if (value == null) {
            return false;
        }

        Node<T> found = standPair(value).get(1);

        return found != null && found.getValue() == value;
    }


    private List<Node<T>> standPair(T value) {
        ArrayList<Node<T>> pair = new ArrayList<>();

        Node<T> prev = head;
        Node<T> cur = prev.getNextNode();

        while (cur != null) {
            if (cur.getNodeVersion() != 0 && cur.getValue() == value) {
                pair.add(prev);
                pair.add(cur);
                return pair;
            }

            prev = cur;
            cur = cur.getNextNode();
        }

        pair.add(prev);
        pair.add(null);
        return pair;
    }

    public boolean isEmpty() {
        // actually, we don't need the whole snapshot, but whatever
        return snapshot().isEmpty();
    }

    public Iterator<T> iterator() {
        return snapshot().iterator();
    }

    private List<T> snapshot() {
        while (true) {
            List<Pair<T, Integer>> l1 = naiveSnapshot();

            List<Pair<T, Integer>> l2 = naiveSnapshot();

            if (l1.equals(l2)) {
                List<T> result = new ArrayList<>();
                for (Pair<T, Integer> elem : l1) {
                    if (elem.fst != null) {
                        result.add(elem.fst);
                    }
                }
                return result;
            }
        }
    }

    private List<Pair<T, Integer>> naiveSnapshot() {
        ArrayList<Pair<T, Integer>> shot = new ArrayList<>();

        for (Node<T> currentNode = head; currentNode != null; currentNode = currentNode.getNextNode()) {
            if (currentNode.getNodeVersion() == 0) {
                continue;
            }

            shot.add(new Pair<>(currentNode.getValue(), currentNode.getNodeVersion()));
        }

        return shot;
    }

}
