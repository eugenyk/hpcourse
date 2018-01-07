package lab2;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T> {

    private Node<T> head;
    private Node<T> tail;
    
    static class Node<T> {
        T item;
        AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null, false);

        Node(T item) {
            this.item = item;
        }
    }
    
    static class Pair<T> {
        Node<T> left;
        Node<T> right;

        Pair(Node<T> left, Node<T> right) {
            this.left = left;
            this.right = right;
        }
    }
    
    LockFreeSetImpl() {
        tail = new Node<>(null);
        head = new Node<>(null);
        head.next.set(tail, false);
    }
    
    private Pair<T> search(T item) {
        Node<T> prev;
        Node<T> curr;
        Node<T> next;
        boolean[] marked = {false};

        if (isEmpty()) {
            return new Pair<>(head, tail);
        }

        retry:
        while (true) {
            prev = head;
            curr = prev.next.getReference();
            while (true) {
                next = curr.next.get(marked);
                while (marked[0]) {
                    //physical remove
                    if (!prev.next.compareAndSet(curr, next, false, false)) {
                        continue retry;
                    }
                    curr = next;
                    next = curr.next.get(marked);
                }
                //when we stop searching
                if (curr == tail || item.compareTo(curr.item) <= 0) {
                    return new Pair<>(prev, curr);
                }
                prev = curr;
                curr = next;
            }
        }
    }
    
    public boolean add(T item) {
        Node<T> newNode = new Node<>(item);
        while (true) {
            Pair<T> pair = search(item);
            Node<T> left = pair.left;
            Node<T> right = pair.right;

            if (right.item == item) {
                return false;
            } 
            else {
                newNode.next.set(right, false);
                if (left.next.compareAndSet(right, newNode, false, false)) {
                    return true;
                }
            }
        }
    }

    public boolean remove(T item) {
        while (true) {
            Pair<T> pair = search(item);
            Node<T> curr = pair.right;
            if (curr.item != item) {
                return false;
            }
            Node<T> next = curr.next.getReference();
            //logical remove
            if (curr.next.compareAndSet(next, next, false, true)) {
                return true;
            }
        }
    }

    public boolean contains(T item) {
        Pair<T> pair = search(item);
        Node<T> right = pair.right;
        return (right.item == item && !right.next.isMarked());
    }

    public boolean isEmpty() {
        boolean[] marked = {false};
        while(true){
            
            if(head.next.getReference() == tail){
                return true;
            }
            // test if all elements are marked to remove
            else{
                Node<T> head_next = head.next.getReference();
                Node<T> head_next_next = head_next.next.get(marked);
                if(marked[0] && head_next != tail){
                    head.next.compareAndSet(head_next,head_next_next,false,false);
                }
                else{
                    return false;
                }
            }
        }
            
        
    }
    public void printResult() {
        
        String result = "";
        Node<T> curr = this.head.next.getReference();

        while (curr != tail) {
            if (contains(curr.item)) {
                result = result + curr.item.toString() + " "; 
            }
            curr = curr.next.getReference();
        }
        System.out.println("Result: " + result);
        
    }
}
