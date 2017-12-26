package com.company;

import java.util.stream.IntStream;

public class Main {

    static void addTest() {
        LockFreeSet<Integer> lockFreeSet = new CustomLockFreeSet<>();
        IntStream.range(0, 10000).parallel().forEach(lockFreeSet::add);
        IntStream.range(0, 10000).parallel().forEach(i ->
        {
            if (!lockFreeSet.contains(i)) {
                throw new RuntimeException("Не содержит число " + i);
            }
        });
    }

    static void removeTest() {
        LockFreeSet<Integer> lockFreeSet = new CustomLockFreeSet<>();
        assertIsEmpty(lockFreeSet);
        IntStream.range(0, 1000).parallel().forEach(lockFreeSet::add);
        assertIsNotEmpty(lockFreeSet);
        IntStream.range(100, 200).parallel().forEach(lockFreeSet::remove);
        assertIsNotEmpty(lockFreeSet);
        IntStream.range(100, 200).parallel().forEach(i ->
        {
            if (lockFreeSet.contains(i)) {
                throw new RuntimeException("Не должно содержать число " + i);
            }
        });
        assertIsNotEmpty(lockFreeSet);
        IntStream.range(0, 99).parallel().forEach(i ->
        {
            if (!lockFreeSet.contains(i)) {
                throw new RuntimeException("Не должно содержать число " + i);
            }
        });
        IntStream.range(201, 1000).parallel().forEach(i ->
        {
            if (!lockFreeSet.contains(i)) {
                throw new RuntimeException("Не должно содержать число " + i);
            }
        });
        assertIsNotEmpty(lockFreeSet);
    }

    static void assertIsEmpty(LockFreeSet lockFreeSet){
        if(!lockFreeSet.isEmpty()){
            throw new RuntimeException("А должен быть пустым");
        }
    }

    static void assertIsNotEmpty(LockFreeSet lockFreeSet){
        if(lockFreeSet.isEmpty()){
            throw new RuntimeException("А должен быть не пустым");
        }
    }

    public static void main(String[] args) {
        addTest();
        removeTest();
    }
}
