package ru.spbau.mit;

import java.util.concurrent.atomic.AtomicMarkableReference;

class Node<U extends Comparable<U>> {
    private final U value;
    private final AtomicMarkableReference<Node<U>> next;
    private NodeType nodeType;

    Node(U value, Node<U> next) {
        this.value = value;
        this.next = new AtomicMarkableReference<>(next, false);
        this.nodeType = NodeType.Usual;
    }

    public static <T extends Comparable<T>> Node<T> tailNode() {
        Node<T> node = new Node<>(null, null);
        node.nodeType = NodeType.Tail;
        return node;
    }

    public static <T extends Comparable<T>> Node<T> headNode(Node<T> tailNode) {
        Node<T> node = new Node<>(null, tailNode);
        node.nodeType = NodeType.Head;
        return node;
    }

    public boolean isHead() {
        return nodeType == NodeType.Head;
    }

    public boolean isTail() {
        return nodeType == NodeType.Tail;
    }

    // this.value <= value.
    public int compareToValue(U value) {
        if (isHead()) {
            return -1;
        }
        if (isTail()) {
            return 1;
        }
        return this.value.compareTo(value);
    }

    public AtomicMarkableReference<Node<U>> getNext() {
        return next;
    }

    public U getValue() {
        return value;
    }

    private enum NodeType {
        Usual,
        Head,
        Tail
    }
}
