package lockfree_set;

import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.*;

public class MySetTest {

    @Test
    public void simpleTest() {
        for (int t = 0; t < 10; t++) {
            final MySet<Integer> mySet = new MySet<Integer>();

            List<Integer> nums = new ArrayList<Integer>();
            for (int i = 0; i < 1000; i++) {
                nums.add(i);
            }
            Collections.shuffle(nums);

            assertTrue(mySet.isEmpty());
            for (int i : nums) {
                assertTrue(mySet.add(i));
            }
            for (int i : nums) {
                assertTrue(mySet.contains(i));
            }
            assertFalse(mySet.isEmpty());
            for (int i : nums) {
                assertTrue(mySet.remove(i));
            }
            assertFalse(mySet.remove(123));
            assertTrue(mySet.isEmpty());
        }
    }

    @Test
    public void multiThreadTest() {
        for (int test = 0; test < 100; test++) {
            final MySet<Integer> mySet = new MySet<Integer>();

            List<Integer> nums = new ArrayList<Integer>();
            for (int i = 0; i < 100; i++) {
                nums.add(i);
            }
            Collections.shuffle(nums);

            List<Thread> threads = new ArrayList<Thread>();

            for (final int i : nums) {
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        assertFalse(mySet.contains(i));
                        assertTrue(mySet.add(i));
                        assertTrue(mySet.contains(i));
                        assertFalse(mySet.add(i));
                    }
                });
                threads.add(t);
                t.start();
            }

            joinThreads(threads);
            Collections.shuffle(nums);

            for (final int i : nums) {
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        assertTrue(mySet.contains(i));
                        assertFalse(mySet.add(i));
                        assertTrue(mySet.remove(i));
                        assertFalse(mySet.contains(i));
                    }
                });
                threads.add(t);
                t.start();
            }

            joinThreads(threads);

            assertTrue(mySet.isEmpty());
        }
    }

    private void joinThreads(List<Thread> threads) {
        for (Thread th : threads) {
            try {
                th.join();
            } catch (Exception e) {
                // blah-blah
            }
        }
    }
}