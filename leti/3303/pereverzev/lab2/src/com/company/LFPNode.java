package com.company;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LFPNode<E extends Comparable<E>> {

    public E value;
    public AtomicMarkableReference<LFPNode<E>> next;

    public LFPNode(E value, LFPNode<E> next)
    {
        this.value = value;
        this.next = new AtomicMarkableReference<>(next, false);
    }

    public LFPNode(E value)
    {
        this(value, null);
    }
}

