package com.company;

public class Main {

    public static void main(String[] args) {
        LockFreePriorityLFQueue<Double> intQueue = new LockFreePriorityLFQueue<>();
        for(int i = 0; i < 10; i++) {
            intQueue.offer(Math.random());
        }

        for(int i = 0; i < 10; i++) {
            System.out.println(intQueue.poll());
        }
    }
}
