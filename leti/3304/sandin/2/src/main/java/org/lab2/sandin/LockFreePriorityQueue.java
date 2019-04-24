package org.lab2.sandin;

import java.util.AbstractQueue;
import java.util.Iterator;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreePriorityQueue<E extends Comparable<E>> extends AbstractQueue<E> implements PriorityQueue<E>{

    private AtomicMarkableReference<Node<E>> head;
    private AtomicMarkableReference<Node<E>> tail;
    private AtomicInteger numOfNodes;

    LockFreePriorityQueue(){
        Node<E> nodeTail = new Node<>(null);
        Node<E> nodeHead = new Node<>(null,nodeTail);
        head = new AtomicMarkableReference<>(nodeHead, false);
        tail = new AtomicMarkableReference<>(nodeTail, false);
        numOfNodes = new AtomicInteger(0);
    }
    
    @Override
    public Iterator<E> iterator() {
        return null;
    }

    @Override
    public boolean offer(E e) {
        Node<E> newNode = new Node<>(e);
        
        while (true)
        {

            Node<E> prev = head.getReference();
            Node<E> current = prev.getNext();
            Node<E> next = current.getNext();
            
            while (true)
            {
                if (next == null || e.compareTo(current.value) < 0)
                {
                    break;
                }
                else if (current.isMarked())
                {
                    if (prev.next.compareAndSet(current, next, false, false))
                    {
                        numOfNodes.decrementAndGet();
                    }
                }
                
                prev = current;
                current = current.getNext();
                next = current.getNext();
            }
            
            newNode.next.set(current, false);
            if (prev.next.compareAndSet(current, newNode, false, false))
            {
                numOfNodes.incrementAndGet();
                return true;
            }         
        }
    }

    @Override
    public E poll() {
        Node<E> first = null;
        Node<E> current = null;
        Node<E> next = null;
        Node<E> last = null;
        
        while (true)
        {
            first = head.getReference();
            current = first.getNext();
            next = current.getNext();
            last = tail.getReference();
            
            if (current == last)
            {
                return null;
            }
            
            if (current.isMarked())
	            {
	                if (first.next.compareAndSet(current, next, false, false))
	                {
	                    numOfNodes.decrementAndGet();
	                }
	            }
	            else
	            {
	                E data = current.value;
	                if (current.next.compareAndSet(next, next, false, true))
	                {
	                    if (first.next.compareAndSet(current, next, false, false))
	                    {
	                        numOfNodes.decrementAndGet();
	                    }
	                    
	                    return data;
	                }
	            }
            }
    }

    @Override
    public E peek() {
        Node<E> first;
        Node<E> current;
        Node<E> next;
        Node<E> last;
        
        while (true)
        {
            first = head.getReference();
            current = first.getNext();
            next = current.getNext();
            last = tail.getReference();

            if (current.isMarked())
            {
                if (first.next.compareAndSet(current, next, false, false))
                {
                    numOfNodes.decrementAndGet();
                }
            }
            else if (current != last)
            {
                return current.value;
            }
            else 
            {
                return null;
            }
        }
    }

    @Override
    public  boolean isEmpty(){
        Node<E> first = null;
        Node<E> current = null;
        Node<E> last = null;

        while(true)
        {
            first = head.getReference();
            current = first.getNext();
            last = tail.getReference();
            
            if (current == last)
            {
                return true;
            } 

            if (current.isMarked())
            {
                if (first.next.compareAndSet(current, current.getNext(), false, false))
                {
                    numOfNodes.decrementAndGet();
                }
                
                continue;
            }
  
            return false;
        }
    }
    
    @Override
    public int size() {
        return numOfNodes.get();
    }
    
    protected class Node <E extends Comparable<E>> {
        public E value;
        public AtomicMarkableReference<Node<E>> next;

        Node(E value){
            this.value = value;
            this.next = new AtomicMarkableReference<>(null, false);
        }

        Node (E value, Node<E> next){
            this.value = value;
            this.next = new AtomicMarkableReference<>(next, false);
        }
        
        boolean isMarked() {
            return next.isMarked();
        }

        Node<E> getNext() {
            return next.getReference();
        }
    }
}