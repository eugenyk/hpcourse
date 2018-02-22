package com.lockfreeset;

import java.util.Random;

public class Main {

    public static void main(String[] args) {

        LockFreeSetImplementation<Integer> testSet = new LockFreeSetImplementation<>();

        int count = 20;
        new Thread(() -> {
            for (int i=0; i < count*5; i++) {
                new Main().handleThread(testSet, "add");
            }
        }).start();

        new Thread(() -> {
            for (int i=0; i < count*4; i++) {
                new Main().handleThread(testSet, "rem");
            }
        }).start();

        new Thread(() -> {
            for (int i=0; i < count*3; i++) {
                new Main().handleThread(testSet, "rem");
            }
        }).start();

        try { Thread.sleep(6000); } catch (Exception e) { }

        testSet.printResult();
    }

    public void handleThread(LockFreeSetImplementation<Integer> testSet, String  command) {
        Random random = new Random();
        int a = random.nextInt(100);
        try { Thread.sleep(a); } catch (Exception e) { }
        if (command.equals("add")) {
            if (testSet.add(a)) {
                System.out.println("Added: " + a);
            } else System.out.println("Not added: " + a);
        } else {
            if (testSet.remove(a)) {
                System.out.println("Removed: " + a);
            } else System.out.println("Not removed: " + a);
        }
    }
}