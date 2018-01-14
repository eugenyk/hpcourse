/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package lb2;
import java.util.concurrent.atomic.AtomicMarkableReference;
/**
 *
 * @author andre
 */
public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T>{

    private final Node<T> head;
    private final Node<T> tail;
    
     LockFreeSetImpl(){
        head = new Node<>(null);
        tail = new Node<>(null);
        head.next.set(tail,false);
    }
     
    static class Node<T>{
        final T key;
        AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null,false);

        Node(T key){
            this.key = key;
        }
    }

    static class Pair<T>{
        Node<T> left;
        Node<T> right;

        Pair(Node<T> left, Node<T> right){
            this.left = left;
            this.right = right;
        }
    }

    private Pair<T> search(T value){
        Node<T> right;
        Node<T> left;
        Node<T> right_next;
        boolean[] mark = {false};

        if(isEmpty()){
            return new Pair<>(head,tail);
        }

        search_again:
        do{
           left = head;
           right = head.next.getReference();

           do{
                right_next = right.next.get(mark);

                while (mark[0]){
                    if(!left.next.compareAndSet(right,right_next,false,false)){
                        continue search_again;
                    }

                    right = right_next;
                    right_next = right.next.get(mark);
                }

                if(right == tail || value.compareTo(right.key) <= 0){
                    return new Pair<>(left, right);
                }

                left = right;
                right = right_next;

           }while(true);

        }while (true);

    }

    @Override
    public boolean add(T value) {
        Node<T> newNode = new Node<>(value);

        do{
            Pair<T> LR = search(value);
            Node<T> left = LR.left;
            Node<T> right = LR.right;

            if(right.key == value){
                return false;
            }
            else{
                newNode.next.set(right,false);
                if(left.next.compareAndSet(right,newNode,false,false)){
                    return true;
                }
            }

        }while(true);
    }

    @Override
    public boolean remove(T value) {
        do {
            Pair<T> LR = search(value);
            Node<T> right = LR.right;

            if(right.key != value){
                return false;
            }

            Node<T> right_next = right.next.getReference();

            if(right.next.compareAndSet(right_next,right_next,false,true)){
                return true;
            }
        }while(true);
    }

    @Override
    public boolean contains(T value) {
        Pair<T> LR = search(value);
        Node<T> right = LR.right;

        return right.key == value && !right.next.isMarked();
    }

    @Override
    public boolean isEmpty() {
        boolean[] mark = {false};
        do{
            if(head.next.getReference() == tail){
                return true;
            }
            else{
                Node<T> first = head.next.getReference();
                Node<T> second = first.next.get(mark);
                if(mark[0] && first != tail) {
                    head.next.compareAndSet(first, second, false, false);
                }
                else{
                    return false;
                }
            }
        }while(true);
    }


}