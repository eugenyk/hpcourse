package ru.homework;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class Node<E extends Comparable<E>> {

    public static class State {
        public static final boolean ACTIVE = false;
        public static final boolean DELETED = true;
    }

    private E key;
    private AtomicMarkableReference<Node<E>> next = new AtomicMarkableReference<>(null, State.ACTIVE);

    Node(E key) {
        this.key = key;
    }

    Node(E key, Node<E> next) {
        this.key = key;
        this.next = new AtomicMarkableReference<>(next, State.ACTIVE);
    }

    public void setNext(Node<E> next) {
        this.next.set(next, State.ACTIVE);
    }

    public Node<E> getNext() {
        return next.getReference();
    }

    public AtomicMarkableReference<Node<E>> getAtomicNext() {
        return next;
    }

    public E getKey() {
        return key;
    }

    public boolean isMarked() {
        return next.isMarked();
    }
}