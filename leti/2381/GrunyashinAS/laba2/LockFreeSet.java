package com.lockfreeset;

public interface LockFreeSet<T extends Comparable<T>> {
   
    boolean add(T value);
    boolean remove(T value);
    boolean contains(T value);
    boolean isEmpty();
}