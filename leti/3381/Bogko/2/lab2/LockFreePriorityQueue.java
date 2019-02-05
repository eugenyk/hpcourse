package lab2;

import java.util.AbstractQueue;
import java.util.Collection;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E> {

    private AtomicMarkableReference<LockFreePriorityQueueNode<E>> head;
    private AtomicMarkableReference<LockFreePriorityQueueNode<E>> tail;
    private AtomicInteger count;

    private class Window {
        public LockFreePriorityQueueNode<E> prev;
        public LockFreePriorityQueueNode<E> cur;
        public Window(LockFreePriorityQueueNode<E> previous, LockFreePriorityQueueNode<E> current)
        {
            this.prev = previous;
            this.cur = current;
        }
    }

    public LockFreePriorityQueue() {
        LockFreePriorityQueueNode<E> t = new LockFreePriorityQueueNode<>(null);
        LockFreePriorityQueueNode<E> h = new LockFreePriorityQueueNode<>(null, t);
        head = new AtomicMarkableReference<>(h, false);
        tail = new AtomicMarkableReference<>(t, false);
        count = new AtomicInteger(0);
    }


    private Window findInterval(E value){
        LockFreePriorityQueueNode<E> previous = head.getReference();
        LockFreePriorityQueueNode<E> current = previous.next.getReference();

        boolean[] currentMayBeDeleted = {false};
        LockFreePriorityQueueNode<E> next = current.next.get(currentMayBeDeleted);

        while (true)
        {
            if (currentMayBeDeleted[0])
            {
                if (previous.next.compareAndSet(current, next, false, false))
                {
                    count.decrementAndGet();
                }
            }
            else if (next == null || value.compareTo(current.value) < 0)
            {
                return new Window(previous, current);
            }
            previous = current;
            current = current.next.getReference();
            next = current.next.get(currentMayBeDeleted);
        }
    }

    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public int size() {
        return count.get();
    }

    public boolean add(E value){
        LockFreePriorityQueueNode<E> newNode = new LockFreePriorityQueueNode<>(value);
        while (true)
        {
            Window w = findInterval(value);
            LockFreePriorityQueueNode<E> previous = w.prev;
            LockFreePriorityQueueNode<E> current = w.cur;
            newNode.next.set(current, false);
            if (previous.next.compareAndSet(current, newNode, false, false))
            {
                count.incrementAndGet();
                return true;
            }
        }
    }

    @Override
    public boolean addAll(Collection<? extends E> c) {
        for (E elem : c)
        {
            this.add(elem);
        }
        return true;
    }

    @Override
    public boolean offer(E value) {
        return add(value);
    }

    public E remove() {
        boolean[] currentMayBeDeleted = {false};
        LockFreePriorityQueueNode<E> first = null;
        LockFreePriorityQueueNode<E> curr = null;
        LockFreePriorityQueueNode<E> next = null;
        LockFreePriorityQueueNode<E> last = null;
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
                    count.decrementAndGet();
                }
            }
            else if (curr != last)
            {
                E value = curr.value;
                if (curr.next.compareAndSet(next, next, false, true))
                {
                    if (first.next.compareAndSet(curr, next, false, false))
                    {
                        count.decrementAndGet();
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
    public E poll() {
        return this.remove();
    }

    @Override
    public E peek(){
        LockFreePriorityQueueNode<E> first;
        LockFreePriorityQueueNode<E> curr;
        LockFreePriorityQueueNode<E> next;
        LockFreePriorityQueueNode<E> last;

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
                    count.decrementAndGet();
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
    @Override
    public  boolean isEmpty(){
        LockFreePriorityQueueNode<E> first = null;
        LockFreePriorityQueueNode<E> curr = null;
        LockFreePriorityQueueNode<E> next = null;
        LockFreePriorityQueueNode<E> last = null;
         while(true) {
             boolean[] currentMayBeDeleted = {false};
             first = head.getReference();
             curr = first.next.getReference();
             last = tail.getReference();

             if (curr == last) {
                 return true;
             } else {
                 next = curr.next.get(currentMayBeDeleted);
                 if (currentMayBeDeleted[0]) {
                     if (first.next.compareAndSet(curr, next, false, false)) {
                         count.decrementAndGet();
                         continue;
                     }
                 }
             }

             return false;
         }
    }
}