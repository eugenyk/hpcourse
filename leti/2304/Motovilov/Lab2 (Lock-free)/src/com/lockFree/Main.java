package com.lockFree;

public class Main {
    public static void main(String[] args) {
        LockFreeSetImpl<Integer> set = new LockFreeSetImpl<>();

        new Thread(() -> {
            for (int i = 0; i < 100; i++) {set.add(i);}
        }).start();

        new Thread(() -> {
            for (int i = 0; i < 100; i += 3) {set.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i=100; i > 0; i--) {set.remove(i);}
        }).start();

        new Thread(() -> {
            for (int i = 100; i > 0; i -= 3) {set.add(i);}
        }).start();

        try { Thread.sleep(1000); } catch (Exception e) { }
        System.out.print(set);
    }
}
