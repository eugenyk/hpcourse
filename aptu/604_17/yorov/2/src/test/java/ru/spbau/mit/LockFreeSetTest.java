package ru.spbau.mit;

import org.junit.Assert;
import org.junit.Test;
import ru.spbau.mit.api.LockFreeSet;
import ru.spbau.mit.impl.LockFreeSetImpl;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.ConcurrentSkipListSet;

public class LockFreeSetTest {

    @FunctionalInterface
    private interface TripleFunction<F, S, T> {
        boolean apply(F first, S sec, T third);
    }

    private static void
    testRunner(List<TripleFunction<ConcurrentSkipListSet<Integer>, LockFreeSet<Integer>, Integer>> funcs) {
        final int count = 100;
        final ConcurrentSkipListSet<Integer> standardSet = new ConcurrentSkipListSet<>();
        final LockFreeSet<Integer> lockFreeSet = new LockFreeSetImpl<>();
        final ArrayList<Integer> list = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            Integer temp = new Integer(i);
            list.add(temp);
            Assert.assertEquals(standardSet.add(temp), lockFreeSet.add(temp));
        }

        Random rand = new Random();
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 2 * count; i++) {
            threads.add(new Thread(() -> {
                final int pos = rand.nextInt(count);
                final int funcPos = rand.nextInt(funcs.size());
                final Integer elem = list.get(pos);
                synchronized (list.get(pos)) {
                    Assert.assertTrue(funcs.get(funcPos).apply(standardSet, lockFreeSet, elem));
                }

            }));
        }

        threads.forEach(Thread::start);
        threads.forEach(t -> {
            try {
                t.join();
            } catch (InterruptedException e) { }
        });
    }

    @Test
    public void testAddContains() {
        List<TripleFunction<ConcurrentSkipListSet<Integer>, LockFreeSet<Integer>, Integer>> funcs =
                new ArrayList<>(2);
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.add(elem) == lockFreeSet.add(elem));
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.contains(elem) == lockFreeSet.contains(elem));
        testRunner(funcs);
    }

    @Test
    public void testAddRemove() {
        List<TripleFunction<ConcurrentSkipListSet<Integer>, LockFreeSet<Integer>, Integer>> funcs =
                new ArrayList<>(2);
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.add(elem) == lockFreeSet.add(elem));
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.remove(elem) == lockFreeSet.remove(elem));
        testRunner(funcs);
    }

    @Test
    public void testContainsRemove() {
        List<TripleFunction<ConcurrentSkipListSet<Integer>, LockFreeSet<Integer>, Integer>> funcs =
                new ArrayList<>(2);
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.remove(elem) == lockFreeSet.remove(elem));
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.contains(elem) == lockFreeSet.contains(elem));
        testRunner(funcs);
    }

    @Test
    public void testMix() {
        List<TripleFunction<ConcurrentSkipListSet<Integer>, LockFreeSet<Integer>, Integer>> funcs =
                new ArrayList<>(3);
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.add(elem) == lockFreeSet.add(elem));
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.contains(elem) == lockFreeSet.contains(elem));
        funcs.add((standardSet, lockFreeSet, elem) -> standardSet.remove(elem) == lockFreeSet.remove(elem));
        testRunner(funcs);
    }
}
