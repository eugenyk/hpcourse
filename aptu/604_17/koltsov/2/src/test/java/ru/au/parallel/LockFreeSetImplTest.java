package ru.au.parallel;

import org.junit.jupiter.api.Test;
import ru.au.parallel.util.RunnableWithNumber;

import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

public class LockFreeSetImplTest {

    @Test
    void testAdd() {
        LockFreeSet<String> lst = new LockFreeSetImpl<>();

        assertTrue(lst.add("1"));
        lst.add("2");
        lst.add("3");
        assertFalse(lst.add("1"));

        assertFalse(lst.isEmpty());
        assertTrue(lst.contains("1"));
        assertTrue(lst.contains("2"));
        assertTrue(lst.contains("3"));
    }

    @Test
    void testNullContainsRemove() {
        LockFreeSet<String> lst = new LockFreeSetImpl<>();

        assertFalse(lst.contains(null));
        assertFalse(lst.remove(null));
    }

    @Test
    void testRemove() {
        LockFreeSet<String> lst = new LockFreeSetImpl<>();

        lst.add("qwe");
        lst.add("wert");
        assertTrue(lst.remove("qwe"));
        assertFalse(lst.remove("nonexistant"));

        assertFalse(lst.contains("qwe"));
        assertTrue(lst.contains("wert"));
    }

    @Test
    void testContains() {
        LockFreeSet<Integer> lst = new LockFreeSetImpl<>();
        assertFalse(lst.contains(123));
        assertTrue(lst.add(123));
        assertTrue(lst.contains(123));
    }

    @Test
    void testIsEmpty() {
        LockFreeSet<Integer> lst = new LockFreeSetImpl<>();
        assertTrue(lst.isEmpty());

        lst.add(5);
        assertFalse(lst.isEmpty());
    }


    @Test
    void testSimultaneousAddThenDelete() throws InterruptedException {
        final int numThreads = 300;

        final LockFreeSet<Integer> lst = new LockFreeSetImpl<>();
        final CyclicBarrier barrier = new CyclicBarrier(numThreads);

        final Thread workers[] = new Thread[numThreads];

        for (int i = 0; i < numThreads; i++) {
            workers[i] = new Thread(new RunnableWithNumber(i) {
                public void run() {
                    try {
                        barrier.await();
                    } catch (InterruptedException | BrokenBarrierException e) {
                        e.printStackTrace();
                    }

                    lst.add(runnableNumber);
                }
            });
            workers[i].start();
        }

        for (int i = 0; i < numThreads; i++) {
            workers[i].join();
        }

        assertFalse(lst.isEmpty());
        for (int i = 0; i < numThreads; i++) {
            assertTrue(lst.contains(i));
        }

        barrier.reset();


        for (int i = 0; i < numThreads; i++) {
            workers[i] = new Thread(new RunnableWithNumber(i) {
                public void run() {
                    try {
                        barrier.await();
                    } catch (InterruptedException | BrokenBarrierException e) {
                        e.printStackTrace();
                    }

                    assertTrue(lst.remove(runnableNumber));
                }
            });

            workers[i].start();
        }

        for (int i = 0; i < numThreads; i++) {
            workers[i].join();
        }

        assertTrue(lst.isEmpty());
        for (int i = 0; i < numThreads; i++) {
            assertFalse(lst.contains(i));
        }
    }

    private void addAndThenDeleteSeveralThreads(boolean intersectedElems) throws InterruptedException {
        final int numThreads = 20;

        final LockFreeSet<Integer> lst = new LockFreeSetImpl<>();
        final CyclicBarrier barrier = new CyclicBarrier(numThreads);

        final Thread workers[] = new Thread[numThreads];

        for (int i = 0; i < numThreads; i++) {
            workers[i] = new Thread(new RunnableWithNumber(i) {
                public void run() {
                    try {
                        barrier.await();
                    } catch (InterruptedException | BrokenBarrierException e) {
                        e.printStackTrace();
                    }

                    int start = 100 * runnableNumber;
                    int end = 100 * (runnableNumber + 1);

                    if (intersectedElems) {
                        start = 0;
                        end = 100;
                    }

                    for (int i = start; i < end; i++) {
                        lst.add(i);
                        assertTrue(lst.remove(i));
                    }
                }
            });

            workers[i].start();
        }

        for (int i = 0; i < numThreads; i++) {
            workers[i].join();
        }

        assertTrue(lst.isEmpty());
    }

    @Test
    void testConcurrentaddDeleteSeparate() throws InterruptedException {
        addAndThenDeleteSeveralThreads(false);
    }

}
