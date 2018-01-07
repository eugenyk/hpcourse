package Lab2;

import java.util.concurrent.atomic.AtomicMarkableReference;

public class LockFreeSetImpl<T extends Comparable<T>> implements LockFreeSet<T>{

    static class Node<T>{
        final T key;
        final AtomicMarkableReference<Node<T>> next = new AtomicMarkableReference<>(null,false);

        Node(T key){
            this.key = key;
        }
    }

    static class Pair<T>{
        final Node<T> left;
        final Node<T> right;

        Pair(Node<T> left, Node<T> right){
            this.left = left;
            this.right = right;
        }
    }

    private final Node<T> head;
    private final Node<T> tail;

    LockFreeSetImpl(){
        head = new Node<>(null);
        tail = new Node<>(null);
        head.next.set(tail,false);
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
        final Node<T> newNode = new Node<>(value);

        do{
            Pair<T> LR = search(value);
            final Node<T> left = LR.left;
            final Node<T> right = LR.right;

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
            final Node<T> right = LR.right;

            if(right.key != value){
                return false;
            }

            final Node<T> right_next = right.next.getReference();

            if(right.next.compareAndSet(right_next,right_next,false,true)){
                return true;
            }
        }while(true);
    }

    @Override
    public boolean contains(T value) {
        final Pair<T> LR = search(value);
        final Node<T> right = LR.right;

        if(right.key == value && !right.next.isMarked()){
            return true;
        }
        else{
            return false;
        }
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
