package com.company;

import java.util.AbstractQueue;
import java.util.Collection;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class PriorityLFQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private AtomicMarkableReference<LFPNode<E>> head;
    private AtomicMarkableReference<LFPNode<E>> tail;
    private AtomicInteger lengthQueue;

    public PriorityLFQueue() {
        LFPNode<E> t = new LFPNode<>(null);
        LFPNode<E> h = new LFPNode<>(null, t);
        head = new AtomicMarkableReference<>(h, false);
        tail = new AtomicMarkableReference<>(t, false);
        lengthQueue = new AtomicInteger(0);
    }

    private PriorityArea<E> FindArea(E elem){
        LFPNode<E> previous = head.getReference();
        LFPNode<E> current = previous.next.getReference();

        boolean[] currentMayBeDeleted = {false};
        LFPNode<E> next = current.next.get(currentMayBeDeleted);

        while (true)
        {
            if (currentMayBeDeleted[0])
            {
                if (previous.next.compareAndSet(current, next, false, false))
                {
                    lengthQueue.decrementAndGet();
                }
            }
            else if (next == null || elem.compareTo(current.value) < 0)
            {
                return new PriorityArea<>(previous, current);
            }
            previous = current;
            current = previous.next.getReference();
            next = current.next.get(currentMayBeDeleted);
        }
    }

    @Override
    public boolean addAll(Collection<? extends E> c) {
        for (E elem : c)
        {
            this.offer(elem);
        }
        return true;
    }

    @Override
    public  boolean isEmpty(){
        LFPNode<E> first = null;
        LFPNode<E> curr = null;
        LFPNode<E> next = null;
        LFPNode<E> last = null;

        boolean[] currentMayBeDeleted = {false};
        first = head.getReference();
        curr = first.next.getReference();
        last = tail.getReference();

        if (curr == last){
            return true;
        } else {
            next = curr.next.get(currentMayBeDeleted);
            if (currentMayBeDeleted[0])
            {
                if (first.next.compareAndSet(curr, next, false, false))
                {
                    lengthQueue.decrementAndGet();
                }
            }
        }
        return false;
    }

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        return lengthQueue.get();
    }

    @Override
    public boolean offer(E elem) {
        LFPNode<E> newElem = new LFPNode<>(elem);
        while (true)
        {
            PriorityArea<E> area = FindArea(elem);
            LFPNode<E> previous = area.previous;
            LFPNode<E> current = area.current;
            newElem.next.set(current, false);
            if (previous.next.compareAndSet(current, newElem, false, false))
            {
                lengthQueue.incrementAndGet();
                return true;
            }
        }
    }

    @Override
    public E poll() {
        boolean[] currentMayBeDeleted = {false};
        LFPNode<E> first = null;
        LFPNode<E> curr = null;
        LFPNode<E> next = null;
        LFPNode<E> last = null;
        while (true)
        {
            first = head.getReference();
            curr = first.next.getReference();
            next = curr.next.get(currentMayBeDeleted);
            last = tail.getReference();

            if (currentMayBeDeleted[0])
            {
                if (first.next.compareAndSet(curr, next, false, false))
                {
                    lengthQueue.decrementAndGet();
                }
            }
            else if (curr != last)
            {
                E value = curr.value;
                if (curr.next.compareAndSet(next, next, false, true))
                {
                    if (first.next.compareAndSet(curr, next, false, false))
                    {
                        lengthQueue.decrementAndGet();
                    }
                    return value;
                }
            }
            else
            {
                return null;
            }
        }
    }

    @Override
    public E peek() {
        LFPNode<E> first;
        LFPNode<E> curr;
        LFPNode<E> next;
        LFPNode<E> last;

        boolean[] currentMayBeDeleted = {false};
        while (true)
        {
            first = head.getReference();
            curr = first.next.getReference();
            next = curr.next.get(currentMayBeDeleted);
            last = tail.getReference();

            if (currentMayBeDeleted[0])
            {
                if (first.next.compareAndSet(curr, next, false, false))
                {
                    lengthQueue.decrementAndGet();
                }
            }
            else if (curr != last)
            {
                return curr.value;
            }
            else {
                return null;
            }
        }
    }
}
