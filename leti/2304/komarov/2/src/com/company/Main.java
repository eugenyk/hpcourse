package com.company;

public class Main {

    public static void main(String[] args) {
        LockFreeSet<Integer> lockFreeContainer = new LockFreeSetImpl<>();
        lockFreeContainer.add(1);
        lockFreeContainer.add(2);

        assert !lockFreeContainer.isEmpty();
        assert !lockFreeContainer.add(2);
        assert lockFreeContainer.contains(1);
        assert lockFreeContainer.contains(2);

        lockFreeContainer.add(3);
        lockFreeContainer.add(4);
        lockFreeContainer.add(5);

        assert lockFreeContainer.contains(5);
        lockFreeContainer.remove(3);
        assert !lockFreeContainer.contains(3);

        lockFreeContainer.remove(5);
        lockFreeContainer.remove(4);
        assert !lockFreeContainer.remove(3);
        lockFreeContainer.remove(2);
        lockFreeContainer.remove(1);
        assert lockFreeContainer.isEmpty();

        final LockFreeSet<Integer> containerForThread = new LockFreeSetImpl<>();
        final Thread thread1 = new Thread(() -> {
            int i = 0;
            while (i < 1000) {
                containerForThread.add(i);
                i++;
            }
        });
        final Thread thread2 = new Thread(() -> {
            int i = 0;
            while (i < 1000) {
                containerForThread.add(i);
                i++;
            }
        });
        final Thread thread3 = new Thread(() -> {
            int i = 0;
            while (i < 1000) {
                containerForThread.remove(i);
                i++;
            }
        });

        thread1.start();
        thread2.start();
        thread3.start();

        try {
            thread1.join();
            thread2.join();
            thread3.join();
        } catch (InterruptedException ignored) {

        }

        assert !containerForThread.isEmpty();

        System.out.println("Tests passed");
    }
}
