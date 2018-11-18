package ru.spbau.mit;


import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

public final class PriorityQueueLockFreeImpl<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueueLockFree<E> {
    private final AtomicReference<Node<E>> head = new AtomicReference<>(new Node<>(null, null));
    private final boolean debug;
    private final AtomicInteger amount;


    public PriorityQueueLockFreeImpl(boolean debug) {
        this.debug = debug;
        this.amount = new AtomicInteger();
    }


    @Override
    @NotNull
    public Iterator<E> iterator() {
        throw new UnsupportedOperationException();
    }


    private ArrayList<SubNode<E>> nodes() {
        Node<E> p = first();
        ArrayList<SubNode<E>> array = new ArrayList<>();
        while (p != null) {
            SubNode<E> subNode = p.subNode.get();
            array.add(subNode);
            p = subNode.next;
        }
        return array;
    }


    private List<E> elements() {
        return nodes().stream()
                .map(sn -> sn.item)
                .filter(Objects::nonNull)
                .collect(Collectors.toList());
    }


    @Override
    public int size() {
        outerLoop:
        for (;;) {
            Node<E> oldHead = head.get();
            int oldAmount = amount.get();
            ArrayList<SubNode<E>> base_array = nodes();
            ArrayList<ArrayList<SubNode<E>>> check_arrays = new ArrayList<>();
            for (int i = 0; i < 10; i++) {
                Thread.yield();
                check_arrays.add(nodes());
            }
            if (oldHead == head.get()
                    && oldHead.subNode == head.get().subNode
                    && oldAmount == amount.get()) {
                for (ArrayList<SubNode<E>> check_array : check_arrays) {
                    if (!base_array.equals(check_array)) {
                        continue outerLoop;
                    }
                }
                List<E> elemArray = base_array.stream().map(sn -> sn.item).filter(Objects::nonNull).collect(Collectors.toList());
                int localAmount = elemArray.size();
                if (oldAmount == localAmount) {
                    return localAmount;
                }
            }
        }
    }

    @Override
    public boolean offer(@NotNull E e) {
        if (debug) System.out.println("try insert " + e);

        Node<E> newNode;
        outerLoop:
        for (;;) {
            Node<E> oldHead = this.head.get();
            Node<E> node = first();
            Node<E> prev = node;

            SubNode<E> subNode, prevWalkSubNode;
            subNode = prev.subNode.get();
            prevWalkSubNode = subNode;

            if (subNode.item == null && subNode.next == null) {
                if (debug) System.out.println("queue empty");
                final Node<E> newHeadNode = new Node<>(e, null);
                final SubNode<E> newSubNode = new SubNode<>(null, newHeadNode);
                if (prev.subNode.compareAndSet(subNode, newSubNode)) {
                    if (debug) System.out.println("set under null, el = " + e);
                    amount.incrementAndGet();
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
                if (subNode.item == null && prevWalkSubNode.next == node) {
                    if (subNode.next != null)
                    {
                        final SubNode<E> skipSubNode = new SubNode<>(prevWalkSubNode.item, subNode.next);
                        prev.subNode.compareAndSet(prevWalkSubNode, skipSubNode);
                        continue outerLoop;
                    }
                }
                prevWalkSubNode = subNode;
                prev = node;
                node = succ(node);
            }

            newNode = new Node<>(e, node);
            final SubNode<E> prevSubNode = prev.subNode.get();
            if (node == null && prevSubNode.item == null && prevSubNode.next == null) {
                subNode = new SubNode<>(e, null);
                if (prev.subNode.compareAndSet(prevSubNode, subNode)) {
                    if (debug) System.out.println("after = " + e);
                    amount.incrementAndGet();
                    return true;
                }
            } else
            if (prev != node) {
                final SubNode<E> newSubNode = new SubNode<>(prevSubNode.item, newNode);
                if (prevSubNode.next == node
                        && prevSubNode.item != null
                        && prevSubNode.item.compareTo(e) <= 0
                        && prev.subNode.compareAndSet(prevSubNode, newSubNode)) {
                    if (debug) System.out.println("insert " + e + " between [ " + prevSubNode.item + ", " + subNode.item + " ]");
                    amount.incrementAndGet();
                    return true;
                }
            } else {
                final SubNode<E> headSubNode = oldHead.subNode.get();
                if (oldHead == node
                        && headSubNode.item != null
                        && e.compareTo(headSubNode.item) <= 0
                        && (node.subNode.get() == subNode)
                        && updateHead(oldHead, newNode)) {
                    if (debug) System.out.println("update head, el = " + e);
                    if (debug) System.out.println(elements());
                    amount.incrementAndGet();
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
            final Node<E> oldHead = head.get();
            Node<E> p = first();
            SubNode<E> oldSubNode = p.subNode.get();
            if (oldSubNode.item == null && oldSubNode.next == null) {
                return null;
            }
            if (oldSubNode.item != null && oldHead == head.get() && oldHead.subNode == head.get().subNode) {
                final SubNode<E> newSubNode = new SubNode<>(null, oldSubNode.next);
                if (p.subNode.compareAndSet(oldSubNode, newSubNode)) {
                    if (debug) System.out.println("    poll value = " + oldSubNode.item);
                    amount.decrementAndGet();
                    return oldSubNode.item;
                }
            }
        }
    }


    @Override
    public E peek() {
        for (;;) {
            final Node<E> oldHead = head.get();
            Node<E> p = first();
            SubNode<E> subNode = p.subNode.get();
            if (subNode.item == null && subNode.next == null) {
                return null;
            }
            if (subNode.item != null && oldHead == head.get() && oldHead.subNode == head.get().subNode) {
                return subNode.item;
            }
        }
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

