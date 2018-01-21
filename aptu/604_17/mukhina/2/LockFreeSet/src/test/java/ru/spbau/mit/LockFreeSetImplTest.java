package ru.spbau.mit;

import org.junit.Test;

import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CyclicBarrier;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class LockFreeSetImplTest {

    private final int THREADS_NUMBER = 1000;

    private static LockFreeSet<Integer> fillWithSerialIntegers(int maxNumber) {
        LockFreeSet<Integer> intList = new LockFreeSetImpl<>();
        for (int i = 0; i < maxNumber; ++i) {
            intList.add(i);
        }
        return intList;
    }

    @Test
    public void add() throws Exception {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        ArrayList<Thread> threadsList = new ArrayList<>();

        CyclicBarrier barrier = new CyclicBarrier(THREADS_NUMBER);
        for (int i = 0; i < THREADS_NUMBER; ++i) {
            int number = i;
            threadsList.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable e) {
                }
                set.add(number);
            }));
        }
        for (Thread thread : threadsList) {
            thread.start();
        }
        for (Thread thread : threadsList) {
            try {
                thread.join();
            } catch (InterruptedException e) {
            }
        }

        for (int i = 0; i < THREADS_NUMBER; i++) {
            assertTrue(set.contains(i));
        }
    }

    @Test
    public void remove() throws Exception {
        LockFreeSet<Integer> set = fillWithSerialIntegers(THREADS_NUMBER);
        ArrayList<Thread> threadsList = new ArrayList<>();
        CyclicBarrier barrier = new CyclicBarrier(THREADS_NUMBER);

        for (int i = 0; i < THREADS_NUMBER; ++i) {
            int number = i;
            threadsList.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable t) {
                }
                set.remove(number);
            }));
        }

        for (Thread thread : threadsList) {
            thread.start();
        }
        for (Thread thread : threadsList) {
            try {
                thread.join();
            } catch (InterruptedException e) {
            }
        }
        assertTrue(set.isEmpty());
    }

    @Test
    public void removeOneNumberTest() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        int numberInSet = 1;
        set.add(numberInSet);
        ArrayList<Thread> threadsList = new ArrayList<>();
        CopyOnWriteArrayList<Boolean> results = new CopyOnWriteArrayList<>();
        CyclicBarrier barrier = new CyclicBarrier(THREADS_NUMBER);
        for (int i = 0; i < THREADS_NUMBER; ++i) {
            threadsList.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable t) {
                }
                results.add(set.remove(numberInSet));
            }));
        }
        for (Thread thread : threadsList) {
            thread.start();
        }
        for (Thread thread : threadsList) {
            try {
                thread.join();
            } catch (InterruptedException e) {
            }
        }
        int amountOfTrues = 0;
        int amountOfFalses = 0;
        for (boolean res : results) {
            if (res) amountOfTrues++;
            else amountOfFalses++;
        }

        assertEquals(1, amountOfTrues);
        assertEquals(THREADS_NUMBER - 1, amountOfFalses);
    }

    @Test
    public void doubleAddTest() throws Exception {
        LockFreeSet<Integer> set = fillWithSerialIntegers(THREADS_NUMBER);
        ArrayList<Thread> threadsList = new ArrayList<>();
        CyclicBarrier barrier = new CyclicBarrier(THREADS_NUMBER);
        CopyOnWriteArrayList<Boolean> results = new CopyOnWriteArrayList<>();
        for (int i = 0; i < THREADS_NUMBER; ++i) {
            int number = i;
            threadsList.add(new Thread(() -> {
                try {
                    barrier.await();
                } catch (Throwable t) {
                }
                results.add(set.add(number));
            }));
        }
        for (Thread thread : threadsList) {
            thread.start();
        }
        for (Thread thread : threadsList) {
            try {
                thread.join();
            } catch (InterruptedException e) {
            }
        }

        int amountOfTrues = 0;
        int amountOfFalses = 0;
        for (boolean res : results) {
            if (res) amountOfTrues++;
            else amountOfFalses++;
        }

        assertEquals(0, amountOfTrues);
        assertEquals(THREADS_NUMBER, amountOfFalses);
    }
}