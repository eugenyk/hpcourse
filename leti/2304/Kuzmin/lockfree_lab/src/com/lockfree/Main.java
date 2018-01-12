package com.lockfree;

public class Main {

    public static void main(String[] args) {

        LockFreeSetImpl<Integer> testSet = new LockFreeSetImpl<>();

        //test
        new Thread(() -> {
            for (int i=0; i<100; i++) {testSet.add(i);}
        }).start();

        new Thread(() -> {
            for (int i=0; i<100; i+=2) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=100; i>0; i--) {testSet.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=100; i>0; i-=2) {testSet.add(i);}
        }).start();


        try { Thread.sleep(1000); } catch (Exception e) { }
        System.out.print(testSet.getAll().toString() + "\n");
        System.out.print(testSet.isEmpty());
    }
}
