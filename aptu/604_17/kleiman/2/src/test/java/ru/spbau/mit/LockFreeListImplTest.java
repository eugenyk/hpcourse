package ru.spbau.mit;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.locks.Lock;

import static org.junit.Assert.*;

public class LockFreeListImplTest {
    private final int COUNT_THREAD = 500;
    @Test
    public void appendTest() {
        LockFreeList<Integer> lockFreeList = new LockFreeListImpl<>();
        for (int i = 0; i < 100; i++) {
            assertTrue(lockFreeList.add(i));
        }
        for (int i = 0; i < 100; i++) {
            assertTrue(lockFreeList.contains(i));
        }
        for (int i = 0; i < 100; i++) {
            assertFalse(lockFreeList.add(i));
        }
    }

    @Test
    public void emptyTest() {
        LockFreeList<Integer> lockFreeList = new LockFreeListImpl<>();
        assertTrue(lockFreeList.isEmpty());
        lockFreeList.add(0);
        assertFalse(lockFreeList.isEmpty());
    }

    @Test
    public void removeTest() {
        LockFreeList<Integer> lockFreeList = new LockFreeListImpl<>();
        for (int i = 0; i < 100; i++) {
            assertTrue(lockFreeList.add(i));
        }
        assertFalse(lockFreeList.isEmpty());
        for (int i = 0; i < 100; i++) {
            assertTrue(lockFreeList.contains(i));
        }
        for (int i = 0; i < 100; i++) {
            assertTrue(lockFreeList.remove(i));
        }
        assertTrue(lockFreeList.isEmpty());
        for (int i = 0; i < 100; i++) {
            assertFalse(lockFreeList.remove(i));
        }
        for (int i = 0; i < 100; i++) {
            assertFalse(lockFreeList.contains(i));
        }
    }

    @Test
    public void appendDuplicate() {
        LockFreeList<Integer> lockFreeList = new LockFreeListImpl<>();
        for (int i = 0; i < 100; i++) {
            assertTrue(lockFreeList.add(i));
        }
        for (int i = 0; i < 100; i++) {
            assertFalse(lockFreeList.add(i));
        }
    }

    @Test
    public void appendMultiThreadTest() {
        LockFreeList<Integer> lockFreeList = new LockFreeListImpl<>();
        CyclicBarrier barier = new CyclicBarrier(COUNT_THREAD);
        List<Thread> threadList = new ArrayList<>();
        init(threadList, barier, lockFreeList);
        exec(threadList);
        assertFalse(lockFreeList.isEmpty());
        for (int i = 0; i < COUNT_THREAD; i++) {
            assertTrue(lockFreeList.contains(i));
        }
    }

    @Test
    public void removeMultiThreadTest() {
        LockFreeList<Integer> lockFreeList = new LockFreeListImpl<>();
        CyclicBarrier barier = new CyclicBarrier(COUNT_THREAD);
        List<Thread> threadList = new ArrayList<>();
        init(threadList, barier, lockFreeList);
        exec(threadList);
        threadList.clear();
        for(int i = 0; i < COUNT_THREAD; i++) {
            Integer t = i;
            threadList.add(new Thread(() -> {
                try {
                    barier.await();
                } catch (Throwable ignored) {}
                lockFreeList.remove(t);
            }));
        }
        exec(threadList);
        assertTrue(lockFreeList.isEmpty());
        for(int i = 0; i < COUNT_THREAD; i++) {
            assertFalse(lockFreeList.contains(i));
        }
    }

    private void init(List<Thread> threadList, CyclicBarrier barier, LockFreeList<Integer> lockFreeList) {
        for (int i = 0; i < COUNT_THREAD; i++) {
            Integer t = i;
            threadList.add(new Thread(() -> {
                try {
                    barier.await();
                } catch (Throwable ignored) {}
                lockFreeList.add(t);
            }));
        }
    }

    private void exec(List<Thread> threadList) {
        threadList.forEach(Thread::start);
        threadList.forEach(th -> {
            try {
                th.join();
            } catch (InterruptedException ignored) {}
        });
    }
}