package ru.spbau.zhidkov;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.stream.IntStream;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetTest {

    @Test
    public void testNoChangesIsEmptyTrue() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.isEmpty());
    }

    @Test
    public void testNoChangesRemoveFalse() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertFalse(set.remove(-1));
        assertFalse(set.remove(0));
        assertFalse(set.remove(1));
    }

    @Test
    public void testAddOneElement() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.add(1));
        assertTrue(set.contains(1));
        assertFalse(set.contains(-1));
        assertFalse(set.contains(3));
        assertFalse(set.isEmpty());
    }

    @Test
    public void testAddAndRemove() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.add(1));
        assertTrue(set.add(2));
        assertTrue(set.remove(1));
        assertTrue(set.remove(2));
        assertTrue(set.isEmpty());
        assertFalse(set.contains(1));
        assertFalse(set.contains(2));
    }

    @Test
    public void testSecondAddFalse() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.add(1));
        assertFalse(set.add(1));
    }

    @Test
    public void testSecondRemoveFalse() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.add(1));
        assertTrue(set.remove(1));
        assertFalse(set.remove(1));
    }

    @Test
    public void testCase() {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        set.add(1);
        set.add(2);
        set.add(3);
        assertFalse(set.remove(0));
        assertFalse(set.remove(5));
        assertTrue(set.remove(2));
        assertTrue(set.contains(1));
        assertFalse(set.contains(2));
        assertTrue(set.contains(3));
        assertFalse(set.remove(2));
        assertTrue(set.remove(1));
        assertTrue(set.remove(3));
        assertTrue(set.isEmpty());
        assertFalse(set.contains(1));
        assertFalse(set.contains(2));
        assertFalse(set.contains(3));
    }

    @Test
    public void testMultipleThreads() throws InterruptedException {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        final List<Thread> threads = new ArrayList<>();
        IntStream.iterate(1, n -> n + 1)
                .limit(200)
                .peek(n -> threads.add(new Thread(() -> set.add(n))))
                .forEach(n -> threads.add(new Thread(() -> {
                    while (!set.remove(n)) ;
                })));
        Collections.shuffle(threads);
        threads.forEach(Thread::start);
        for (Thread t : threads) {
            t.join();
        }
        assertTrue(set.isEmpty());

    }



}
