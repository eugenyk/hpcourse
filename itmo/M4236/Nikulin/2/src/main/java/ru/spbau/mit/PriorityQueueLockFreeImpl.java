package ru.spbau.mit;


import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.AbstractQueue;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicReference;

public final class PriorityQueueLockFreeImpl<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueueLockFree<E> {
    private final AtomicReference<Node<E>> head = new AtomicReference<>(new Node<>(null, null));
    private final boolean debug;


    public PriorityQueueLockFreeImpl(boolean debug) {
        this.debug = debug;
    }


    @Override
    @NotNull
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }


    private ArrayList<E> elements() {
        Node<E> p = first();
        ArrayList<E> array = new ArrayList<>();
        while (p != null) {
            SubNode<E> subNode = p.subNode.get();
            if (subNode.item != null) {
                array.add(subNode.item);
            }
            p = subNode.next;
        }
        return array;
    }


    @Override
    public int size() {
        for (;;) {
            ArrayList<E> base_array = elements();
            ArrayList<E> check_array = elements();
            if (base_array.equals(check_array)) {
                return base_array.size();
            }
        }
    }

    @Override
    public boolean offer(@NotNull E e) {
        if (debug) System.out.println("try insert " + e);

        Node<E> newNode;
        for (;;) {
            Node<E> oldHead = this.head.get();
            Node<E> node = first();
            Node<E> prev = node;

            SubNode<E> subNode;
            subNode = prev.subNode.get();

            if (subNode.item == null && subNode.next == null) {
                if (debug) System.out.println("queue empty");
                final SubNode<E> newSubNode = new SubNode<>(e, null);
                if (prev.subNode.compareAndSet(subNode, newSubNode)) {
                    if (debug) System.out.println("set under null, el = " + e);
                    return true;
                } else {
                    continue;
                }
            }

            if (debug) System.out.println("start search: el = " + e);
            while (node != null) {
                subNode = node.subNode.get();
                if (debug) System.out.println(e + " :  el = " + subNode.item);
                if (subNode.item != null && subNode.item.compareTo(e) >= 0) {
                    break;
                }
                prev = node;
                node = succ(node);
            }

            newNode = new Node<>(e, node);
            if (prev != node) {
                final SubNode<E> prevSubNode = prev.subNode.get();
                final SubNode<E> newSubNode = new SubNode<>(prevSubNode.item, newNode);
                if (prevSubNode.next == node && prevSubNode.item != null && prev.subNode.compareAndSet(prevSubNode, newSubNode)) {
                    if (debug) System.out.println("insert " + e + " between [ " + prevSubNode.item + ", " + subNode.item + " ]");
                    return true;
                }
            } else {
                if (updateHead(oldHead, newNode)) {
                    if (debug) System.out.println("update head, el = " + e);
                    return true;
                } else {
                    if (debug) System.out.println("-- cant update head, el = " + e);
                }
            }
        }
    }


    @Override
    public E poll() {
        for (;;) {
            Node<E> p = first();
            SubNode<E> oldSubNode = p.subNode.get();
            if (oldSubNode.item == null && oldSubNode.next == null) {
                return null;
            }
            if (oldSubNode.item != null) {
                final SubNode<E> newSubNode = new SubNode<>(null, oldSubNode.next);
                if (p.subNode.compareAndSet(oldSubNode, newSubNode)) {
                    if (debug) System.out.println("    poll value = " + oldSubNode.item);
                    return oldSubNode.item;
                }
            }
        }
    }


    @Override
    public E peek() {
        Node<E> p = first();
        SubNode<E> subNode = p.subNode.get();
        return subNode.item;
    }


    private boolean updateHead(@NotNull Node<E> oldHead, @NotNull Node<E> newHead) {
        return oldHead != newHead && head.compareAndSet(oldHead, newHead);
    }


    @NotNull
    private Node<E> first() {
        Node<E> h, p;
        boolean hasItem;

        h = head.get();
        p = h;
        SubNode<E> subNode;
        while (true) {
            subNode = p.subNode.get();
            hasItem = (subNode.item != null);
            Node<E> q = subNode.next;
            if (hasItem || (q == null)) {
                break;
            }
            p = q;
        }

        this.updateHead(h, p);
        return p;
    }


    @Nullable
    private Node<E> succ(Node<E> node) {
        Node<E> next = node.subNode.get().next;
        return next;
    }


    private static class Node<E> {
        final AtomicReference<SubNode<E>> subNode;

        Node (E item, Node<E> next) {
            subNode = new AtomicReference<>(new SubNode<>(item, next));
        }
    }

    private static class SubNode<E> {
        SubNode (E item, Node<E> next) {
            this.item = item;
            this.next = next;
        }
        final E item;
        final Node<E> next;
    }

}

