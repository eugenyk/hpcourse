import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.*;

@RunWith(Parameterized.class)
public class MultiThreadSetTest {
    private final int STEPS = 100000;
    private final int threads;
    private final int valsPerThread;
    private final LockFreeSet<Integer> s;
    private final StressTestUtil util;

    public MultiThreadSetTest(int threads, int valsPerThread, int operations, boolean fillAll) {
        this.s = new LockFreeSetImpl<>();
        this.threads = threads;
        this.valsPerThread = valsPerThread;
        this.util = new StressTestUtil(operations, false);
        if (fillAll) {
            for (int i = 0; i < threads * valsPerThread; i++) {
                s.add(i);
            }
        }
    }

    @Parameterized.Parameters(name = "{index}: threads={0}, vals_per_test={1}, ops={2}")
    public static Collection<Object[]> data() {
        return Arrays.asList(new Object[][]{
                { 1, 1000, StressTestUtil.DO_ALL, false },
                { 2, 1, StressTestUtil.DO_ADD, false },
                { 2, 1, StressTestUtil.DO_ADD | StressTestUtil.DO_CONTAINS, false },
                { 2, 1, StressTestUtil.DO_REMOVE, true },
                { 2, 1, StressTestUtil.DO_REMOVE | StressTestUtil.DO_CONTAINS, true },
        });
    }

    @Test
    public void testDisjointStress() throws Throwable {
        ExecutorService pool = Executors.newFixedThreadPool(threads);
        final int STEPS = 10000;

        ArrayList<Future<Void>> tasks = new ArrayList<>();
        for (int i = 0; i < threads; i++) {
            final int id = i;
            tasks.add(pool.submit(new Callable<Void>() {
                @Override
                public Void call() throws Exception {
                    util.stressTest(s, valsPerThread * id, valsPerThread,  STEPS,123 + id);
                    return null;
                }
            }));
        }
        for (Future<Void> f : tasks) {
            try {
                f.get();
            } catch (ExecutionException e) {
                throw e.getCause();
            }
        }
        pool.shutdown();
        pool.awaitTermination(Long.MAX_VALUE, TimeUnit.DAYS);
    }
}
