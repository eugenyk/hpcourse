package com.lockfree;

import java.util.List;

public class Tester {

    public static void main(String[] args) {

        LockFreeSetImpl<Integer> testSet = new LockFreeSetImpl<>();

        int count = 100000;


        new Thread(() -> {
            for (int i=0; i<count; i++) {testSet.add(i);}
        }).start();
        System.out.println("First thread is running");
        new Thread(() -> {
            for (int i=0; i<count; i+=2) {testSet.remove(i);}
        }).start();
        System.out.println("Second thread is running");
        new Thread(() -> {
            for (int i=100; i>count; i--) {testSet.remove(i);}
        }).start();
        System.out.println("Third thread is running");
        new Thread(() -> {
            for (int i=100; i>count; i-=2) {testSet.add(i);}
        }).start();
        System.out.println("Fourth thread is running");

        try { Thread.sleep(count / 100); } catch (Exception e) { }

        for (Integer n : testSet.getAll()) {
            System.out.println(n);
        }
        System.out.println("Test is finished");
    }
}
