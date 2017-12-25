/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package hw2;
/**
 *
 * @author Kiril
 */
public class ListBasedSet<T extends Comparable<T>> implements LockFreeSet<T>{
    
    private class ValidationData{
        public Node<T> prev;
        public long pVer;
        public Node<T> cur;
        
        public boolean validationOK;
        
        public ValidationData(Node<T> prev, long pVer, Node<T> cur){
            this.prev = prev;
            this.pVer = pVer;
            this.cur = cur;
            this.validationOK = true;
        }
        
        public ValidationData(){
            this.validationOK = false;
        }
    }
    
    private class Node<T>{
        public T value;
        public Node next;
        public boolean deleted;
        public VersionLock vlock;
        
        public Node(T value, Node next)
        {
            this.value = value;
            this.next = next;
            this.deleted = false;
            this.vlock = new VersionLock();
        }
    };
    
    Node<T> head;
    Node<T> tail;
    
    public ListBasedSet(T rangeStartValue, T rangeEndValue)
    {
        tail = new Node<T>(rangeEndValue, null);
        head = new Node<T>(rangeStartValue, tail);
    }
    
    private Node<T> waitfreeTraversal(T value){
        Node<T> prev = head;
        Node<T> cur = head;
        while (cur.value.compareTo(value) < 0){
            prev = cur;
            cur = cur.next;
        }
        return prev;
    }
    
    private ValidationData validate(T value, Node<T> prev){
        Node<T> cur;
        long pVer;
        validate: do {
            pVer = prev.vlock.getVersion();
            if (prev.deleted) 
                return new ValidationData();

            cur = prev.next;
            while (cur.value.compareTo(value) < 0){
                pVer = cur.vlock.getVersion();
                if (cur.deleted)
                {
                    continue validate;
                }
                prev = cur;
                cur = cur.next;
            }
            break validate;
        }  while(true);
        
        return new ValidationData(prev, pVer, cur);
    }
    
    public boolean add(T value){
        Node<T> prev;
        ValidationData validData;
        Node<T> newNode = new Node<T>(value, null);
        
        adding:
        do {
            prev = waitfreeTraversal(value);
            validation: do {
                validData = validate(value, prev);
                prev = validData.prev;
                
                if (!validData.validationOK)
                    continue adding;
       
                if (validData.cur.deleted)
                    continue validation;
                
                if (validData.cur.value.compareTo(value) == 0)
                    return false;
                
                newNode.next = validData.cur;
                if (!prev.vlock.tryLockAtVersion(validData.pVer))
                    continue validation;
                
                break validation;
                } while(true);
            
            prev.next = newNode;
            prev.vlock.unlockAndIncrementVersion();
            
            break adding;
        } while(true);
        
        System.out.println("add ended");
        return true;
    };
    
    public boolean remove(T value){
        Node<T> prev;
        ValidationData validData;
        
        removing: do{
            prev = waitfreeTraversal(value);
            validation: do {
                validData = validate(value, prev);
                prev = validData.prev;
                
                if (!validData.validationOK)
                    continue removing;
       
                if (validData.cur.deleted || validData.cur.value.compareTo(value) != 0 )
                    return false;
                
                if (!prev.vlock.tryLockAtVersion(validData.pVer))
                    continue validation;
                
                break validation;
                } while(true);
            
            validData.cur.vlock.lockAtCurrentVersion();
            validData.cur.deleted = true;
            prev.next = validData.cur.next;
            validData.cur.vlock.unlockAndIncrementVersion();
            prev.vlock.unlockAndIncrementVersion();
            
            break removing;
        } while(true);
        
        return true;
    };
    
    public boolean contains(T value){
        Node<T> cur = head;
        while (cur.value.compareTo(value) < 0)
            cur = cur.next;
        
        return (cur.value.compareTo(value) == 0 && !cur.deleted);
    };
    
    public boolean isEmpty(){
        return head.next == tail;
    };
}
