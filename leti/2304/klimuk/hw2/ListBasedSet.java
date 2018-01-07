/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package hw2;
import java.util.concurrent.atomic.AtomicMarkableReference;
/**
 *
 * @author Kiril
 */
public class ListBasedSet<T extends Comparable<T>> implements LockFreeSet<T>{
    
    final private Node head;
    final private Node tail;
    
    private class Node{
        public final T value;
        public volatile AtomicMarkableReference<Node> next = new AtomicMarkableReference<>(null, false);
        
        public Node(T value, Node next)
        {
            this.value = value;
            this.next.set(next, false);
        }
    };
    
    private class Window{
        public Node prev = head; 
        public Node curr;
        
        public Window(){}
        
        public Window(Node prev, Node curr){
            this.prev = prev;
            this.curr = curr;
        }
        
        public void setValues(Node prev, Node curr){
            this.prev = prev;
            this.curr = curr;
        }
    }
    
    public ListBasedSet()
    {
        tail = new Node(null, null);
        head = new Node(null, tail);
    }
    
    public ListBasedSet(T min, T max)
    {
        tail = new Node(max, null);
        head = new Node(min, tail);
    }
    
    private Window traverse(T value){
        Node prev = null;
        Node curr = null;
        Node next = null;
        boolean[] marked = {false};
        
        retry: 
        while(true){
            prev = head;
            curr = prev.next.getReference();
            while(curr != null){
                next = curr.next.get(marked);
                //delete marked nodes
                while(marked[0]){
                    if(prev.next.compareAndSet(curr, next, false, false))
                        continue retry;
                    
                    curr = next;
                    next = curr.next.get(marked);
                }
                
                if (curr.value.compareTo(value) > 0)
                    return new Window(prev, curr);
                
                prev = curr;
                curr = next;
            }
        }
    }
    
    
    public boolean add(T value){
        while(true){
            Window win = traverse(value);
            Node prev = win.prev;
            Node curr = win.curr;
            
            if (curr.value.equals(value))
                return false;
            else {
                Node newNode = new Node(value, curr);
                if (prev.next.compareAndSet(curr, newNode, false, false))
                    return true;
            }
       }
   };
    
    
    public boolean remove(T value){
       while(true){
           Window win = traverse(value);
           Node prev = win.prev;
           Node curr = win.curr;
           
           if (curr.value.compareTo(value) != 0)
               return false;
           else {
               Node next = curr.next.getReference();
               if (!curr.next.attemptMark(next, true))
                   continue;
               
               prev.next.compareAndSet(curr, next, false, false);
               return true;
           }
              
       }
    };
    
    public boolean contains(T value){
        Window win = traverse(value);
        return (win.curr.value.equals(value) && !win.curr.next.isMarked());
    };
    
    public boolean isEmpty(){
		while (true){
				Node next = head.next.getReference();
				if (next == tail)
					return true;
				
				if (!curr.next.isMarked())
					return false;
				
				if (!head.next.compareAndSet(next, next.next, false, false))
					continue;
			}
        }
    };
}
