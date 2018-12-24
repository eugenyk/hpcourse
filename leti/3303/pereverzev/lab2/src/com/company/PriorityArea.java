package com.company;

public class PriorityArea<E extends Comparable<E>> {
    public LFPNode<E> previous;
    public LFPNode<E> current;

    public PriorityArea(LFPNode<E> previous, LFPNode<E> current)
    {
        this.previous = previous;
        this.current = current;
    }
}
