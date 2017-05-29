package task;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class ThreadSafeSetRemoveTests {

    @Test
    public void testSmallRemove() {
        final ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();
        int a[] = { 3, 1, 0, 4, 2 };

        for (int i = 0; i < a.length; ++i) {
            assertTrue(set.add(a[i]));
        }

        for (int i = 0; i < a.length; ++i) {
            assertTrue(set.remove(i));
        }
    }

    @Test
    public void testSerialRemove() {
        final ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();
        final int N = 100;

        for (int i = 0; i < N; ++i) {
            assertFalse(set.remove(i));
        }

        for (int i = 0; i < N; ++i) {
            assertTrue(set.add(i));
        }

        for (int i = N; i < N + N; ++i) {
            assertFalse(set.remove(i));
        }

        for (int i = 0; i < N; ++i) {
            assertTrue(set.remove(i));
        }
    }

    @Test
    public void testRandomSerialRemove() {
        final ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();
        final int N = 100;
        ArrayList<Integer> a = new ArrayList<Integer>();

        for (int i = 0; i < N; ++i) {
            a.add(i);
        }

        Collections.shuffle(a);

        for (int i = 0; i < N; ++i) {
            assertTrue(set.add(a.get(i)));
        }

        for (int i = 0; i < N; ++i) {
            assertTrue(set.remove(i));
        }
    }

    @Test
    public void testMultithreadSerialRemove() throws InterruptedException {
        final ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();
        final int N = 4;
        final boolean res[][] = new boolean[2][N];
        ArrayList<Integer> a = new ArrayList<Integer>();

        for (int i = 0; i < N; ++i) {
            a.add(i);
        }

        Collections.shuffle(a);
        for (int i = 0; i < N; ++i) {
            assertTrue(set.add(a.get(i)));
        }

        Runnable adder = new Runnable() {
            public void run() {
                try {
                    for (int i = 0; i < N; ++i) {
                        res[Integer.decode(Thread.currentThread().getName())][i] = set.remove(i);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };

        Thread t0 = new Thread(adder);
        Thread t1 = new Thread(adder);

        t0.setName("0");
        t1.setName("1");

        t0.start();
        t1.start();

        t0.join();
        t1.join();

        for (int i = 0; i < N; ++i) {
            assertTrue(res[0][i] ^ res[1][i]);
        }
    }

    @Test
    public void testMultithreadRandomRemove() throws InterruptedException {
        final ThreadSafeSet<Integer> set = new ThreadSafeSet<Integer>();
        final int N = 10000;
        final boolean res[][] = new boolean[2][N];

        final ArrayList<Integer> elements = new ArrayList<Integer>();

        for (int i = 0; i < N; ++i) {
            elements.add(i);
        }

        Collections.shuffle(elements);
        for (int i = 0; i < N; ++i) {
            assertTrue(set.add(elements.get(i)));
        }

        final ArrayList<ArrayList<Integer>> remElements = new ArrayList<ArrayList<Integer>>();

        remElements.add(new ArrayList<Integer>(N));
        remElements.add(new ArrayList<Integer>(N));

        for (int i = 0; i < N; ++i) {
            remElements.get(0).add(i);
            remElements.get(1).add(i);
        }

        Collections.shuffle(remElements.get(0));
        Collections.shuffle(remElements.get(1));

        Runnable adder = new Runnable() {
            public void run() {
                try {
                    for (int i = 0; i < N; ++i) {
                        int tn = Integer.decode(Thread.currentThread().getName());
                        int e = remElements.get(tn).get(i);
                        res[tn][e] = set.remove(e);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };

        Thread t0 = new Thread(adder);
        Thread t1 = new Thread(adder);

        t0.setName("0");
        t1.setName("1");

        t0.start();
        t1.start();

        t0.join();
        t1.join();

        for (int i = 0; i < N; ++i) {
            assertTrue(res[0][i] ^ res[1][i]);
        }
    }
}