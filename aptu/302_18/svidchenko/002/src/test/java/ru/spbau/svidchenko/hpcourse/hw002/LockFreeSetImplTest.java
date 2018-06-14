package ru.spbau.svidchenko.hpcourse.hw002;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.*;

public class LockFreeSetImplTest {
    @Test
    public void add_simple() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.add(1));
        assertTrue(set.add(2));
        assertFalse(set.add(2));
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));
    }

    @Test
    public void add_async() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                set.add(finalI);
            }));
        }
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
        for (int i = 0; i < 10; i++) {
            assertTrue(set.contains(i));
        }
    }

    @Test
    public void add_asyncBig() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                for (int j = 0; j < 100; j++) {
                    set.add(finalI * 100 + j);
                }
            }));
        }
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
        for (int i = 0; i < 1000; i++) {
            assertTrue(set.contains(i));
        }
    }

    @Test
    public void remove_simple() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(1);
        set.add(2);
        assertTrue(set.remove(1));
        assertTrue(set.remove(2));
        assertFalse(set.remove(3));
        assertFalse(set.contains(1));
        assertFalse(set.contains(2));
        set.add(1);
        set.add(2);
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));
    }

    @Test
    public void remove_async() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                set.add(finalI);
            }));
        }
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
        threads.clear();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                set.remove(finalI);
            }));
        }
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
        for (int i = 0; i < 10; i++) {
            assertFalse(set.contains(i));
        }
    }

    @Test
    public void remove_asyncBig() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                for (int j = 0; j < 100; j++) {
                    set.add(finalI * 100 + j);
                }
            }));
        }
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
        threads.clear();
        for (int i = 0; i < 10; i++) {
            int finalI = i;
            threads.add(new Thread(() -> {
                try {
                    TimeUnit.MILLISECONDS.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                for (int j = 0; j < 100; j++) {
                    set.remove(finalI * 100 + j);
                }
            }));
        }
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
        for (int i = 0; i < 1000; i++) {
            assertFalse(set.contains(i));
        }
    }

    @Test
    public void contains_simple() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(1);
        set.add(2);
        set.add(3);
        assertTrue(set.contains(1));
        assertTrue(set.contains(2));
        assertTrue(set.contains(3));
        assertFalse(set.contains(4));
        set.remove(3);
        assertFalse(set.contains(3));
    }

    @Test
    public void isEmpty_simple() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.isEmpty());
        set.add(1);
        assertFalse(set.isEmpty());
        set.remove(1);
        assertTrue(set.isEmpty());
    }

}