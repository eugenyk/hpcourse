package com.lockfreeset;

import java.util.concurrent.atomic.AtomicMarkableReference;

class Node<T> {

    public T get_value() {
        return _value;
    }

    private final T _value;

    public AtomicMarkableReference<Node<T>> get_next() {
        return _next;
    }

    private final AtomicMarkableReference<Node<T>> _next;

    Node(T value) {
        _value = value;
        _next =  new AtomicMarkableReference<>(null, false);
    }
}