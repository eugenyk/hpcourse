import org.junit.Test;
import org.junit.Assert;

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.*;

public class LockFreeListSetTest extends Assert {
    @Test
    public void basicFunctionalityTest() {
        LockFreeSet<Integer> set = new LockFreeListSet<>();
        assertTrue(set.isEmpty());
        assertTrue(set.add(10));
        assertTrue(set.add(15));
        assertTrue(set.contains(10));
        assertTrue(set.contains(15));
        assertFalse(set.contains(9));
        assertFalse(set.add(10));
        assertTrue(set.remove(10));
        assertFalse(set.contains(10));
        assertFalse(set.isEmpty());
        assertTrue(set.contains(15));
        assertFalse(set.remove(10));
        assertTrue(set.remove(15));
        assertTrue(set.isEmpty());
        assertFalse(set.remove(15));
    }

    @Test
    public void x1ThreadTest() throws ExecutionException, InterruptedException {
        concurrencyTest(1);
    }

    @Test
    public void x4ThreadTest() throws ExecutionException, InterruptedException {
        concurrencyTest(4);
    }

    @Test
    public void x16ThreadTest() throws ExecutionException, InterruptedException {
        concurrencyTest(16);
    }

    @Test
    public void x32sThreadTest() throws ExecutionException, InterruptedException {
        concurrencyTest(32);
    }


    private void concurrencyTest(final int threadCount) throws InterruptedException, ExecutionException {
        final LockFreeSet<Integer> set = new LockFreeListSet<>();
        List<Callable<Boolean>> tasks = new LinkedList<>();
        for (int i = 0; i < threadCount; i++) {
            tasks.add(createTask(i, set));
        }
        ExecutorService executorService = Executors.newFixedThreadPool(threadCount);
        List<Future<Boolean>> futures = executorService.invokeAll(tasks);
        List<Boolean> results = new LinkedList<>();
        for (Future<Boolean> future : futures) {
            results.add(future.get());
        }
        assertEquals(threadCount, results.size());
        for (boolean res : results) {
            assertTrue(res);
        }
        assertTrue(set.isEmpty());
    }

    private Callable<Boolean> createTask(final int id, final LockFreeSet<Integer> set) {
        return new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                boolean result = true;
                for (int num = id * 1000; num < id * 1000 + 900; num++) {
                    result &= set.add(num);
                    result &= !set.add(num);
                }
                result &= !set.isEmpty();
                for (int num = 0; num < 1000; num++) {
                    if (num < 900) {
                        result &= set.contains(id * 1000 + num);
                    } else {
                        result &= !set.contains(id * 1000 + num);
                    }
                }
                for (int num = 0; num < 1000; num++) {
                    if (num < 900) {
                        result &= set.remove(id * 1000 + num);
                        result &= !set.remove(id * 1000 + num);
                    } else {
                        result &= !set.remove(id * 1000 + num);
                    }
                }
                return result;
            }
        };
    }
}