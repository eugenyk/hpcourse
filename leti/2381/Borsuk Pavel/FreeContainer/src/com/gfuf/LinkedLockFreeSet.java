package com.gfuf;

import java.util.Objects;
import java.util.concurrent.atomic.AtomicMarkableReference;
import java.util.concurrent.atomic.AtomicReference;

public class LinkedLockFreeSet<T extends Comparable<T>> implements LockFreeSet<T>{						
	
	private final AtomicReference<Node> head = new AtomicReference<>(null);
	
	@Override
	public boolean add(Comparable value) {
		Node prev;
		Node curr;
		while(true) {
            Pair window = findWindow(value);
            prev = window.first;
            curr = window.second;
            
            if (curr != null && Objects.equals(curr.value, value)) 
            {
                return false;
            }

            Node node = new Node(value, new AtomicMarkableReference<>(curr, false));
            
            if (prev == null) 
            {
                if (this.head.compareAndSet(curr, node)) 
                {
                    return true;
                }
            } 
            else if (prev.next.compareAndSet(curr, node, false, false)) 
            {
                    return true;
            }
		}
	}

	@Override
	public boolean remove(Comparable value) {
		Node prev;
        Node curr;
        boolean casSuccess;
        Node next;
        do 
        {
            Pair window = this.findWindow(value);
            prev = window.first;
            curr = window.second;
            
            if (curr == null || !Objects.equals(curr.value, value)) 
            {
                return false;
            }

            next = curr.next.getReference();
            casSuccess = curr.next.compareAndSet(next, next, false, true);
        } while(!casSuccess);

        if (prev == null) 
        {
            this.head.compareAndSet(curr, next);
        } 
        else 
        {
            prev.next.compareAndSet(curr, next, false, false);
        }

        return true;
	}

	@Override
	public boolean contains(Comparable value) 
	{
		 Node curr = this.head.get();
		 
		 while(curr != null && curr.value.compareTo(value) < 0)
		 {
			 curr = curr.next.getReference();
		 }

	     return curr != null && 
	    		 Objects.equals(curr.value, value) && 
	    		 !curr.next.isMarked();
	}
	
	@Override
	public boolean isEmpty() 
	{
		return this.head.get() == null;
	}
	
	private Pair findWindow(Comparable value) 
	{
        while(true) 
        {
            Node prev = head.get();
            Node prevNext;

            if (prev != null) 
            {
                prevNext = prev.next.getReference();
            } 
            else 
            {
                return new Pair(null, null);
            }

            if (prev.next.isMarked()) 
            {
                head.compareAndSet(prev, prev.next.getReference());
                continue;
            }

            if (prev.value.compareTo(value) >= 0) 
            {
                return new Pair(null, prev);
            }


            Node curr = prevNext;
            boolean retry = false;
            while(curr != null) 
            {
                Node succ = curr.next.getReference();
                if (curr.next.isMarked()) 
                {
                    if (prev.next.compareAndSet(curr, succ, false, false)) 
                    {
                        retry = true;
                        break;
                    }
                } 
                else 
                {
                    if (curr.value.compareTo(value) >= 0) 
                    {
                        return new Pair(prev, curr);
                    }

                    prev = curr;
                }
                curr = succ;
            }

            if (retry) 
            {
                continue;
            }

            return new Pair(prev, null);

        }
    }
	
	private final class Node 
	{
		private Comparable value;
		private AtomicMarkableReference<Node> next;
		 
		Node(Comparable value, AtomicMarkableReference<Node> nextIsRemoved) 
		{
			this.value = value;
			this.next = nextIsRemoved;
		}
	}
	    
	private class Pair
	{
		final Node first;
		final Node second;
		Pair(Node first, Node second) 
		{
			this.first = first;
			this.second = second;
		}	 
	}
	
}
