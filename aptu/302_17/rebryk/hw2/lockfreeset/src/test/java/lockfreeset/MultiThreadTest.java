package lockfreeset;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;

public class MultiThreadTest {
    private static final int THREADS_COUNT = 1000;
    private static final int TESTS_COUNT = 100;

    private Runnable getTask(final Set<Integer> set, final Integer number) {
        return () -> {
            assertFalse(set.contains(number));
            assertTrue(set.add(number));
            assertFalse(set.isEmpty());
            assertFalse(set.add(number));

            assertFalse(set.contains(-number));
            assertTrue(set.add(-number));
            assertFalse(set.isEmpty());
            assertFalse(set.add(-number));

            assertTrue(set.remove(number));
            assertFalse(set.remove(number));
            assertFalse(set.contains(number));

            assertTrue(set.remove(-number));
            assertFalse(set.remove(-number));
            assertFalse(set.contains(-number));
        };
    }

    private void singleTest() {
        final Set<Integer> set = new Set<>();

        final List<Integer> data = new ArrayList<>();
        for (int i = 1; i <= THREADS_COUNT; ++i) {
            data.add(i);
        }

        final List<Thread> threads = new ArrayList<>();

        for (int x : data) {
            Thread thread = new Thread(getTask(set, x));
            threads.add(thread);
        }

        threads.stream().forEach(thread -> thread.start());
        threads.stream().forEach(thread -> {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });

        assertTrue(set.isEmpty());
    }

    @Test
    public void fullTest() {
        for (int i = 0; i < TESTS_COUNT; ++i) {
            singleTest();
        }
    }
}
