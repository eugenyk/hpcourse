/**
 * Created by yeputons.
 */

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import static org.junit.Assert.*;
import static org.hamcrest.CoreMatchers.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Random;
import java.util.concurrent.*;

@RunWith(Parameterized.class)
public class MultiThreadSetConflictingStressTest {
    private final int threads;
    private final int values;

    public MultiThreadSetConflictingStressTest(int threads, int values) {
        this.threads = threads;
        this.values = values;
    }

    LockFreeSet<Integer> constructSet() {
        return new LockFreeSetImpl<Integer>();
    }


    @Parameterized.Parameters(name = "{index}: threads={0}, values={1}, ops={2}")
    public static Collection<Object[]> data() {
        return Arrays.asList(new Object[][]{
                { 1, 1000 },
                { 2, 1 },
                { 2, 2 },
                { 10, 1 },
                { 10, 2 },
                { 10, 5 },
                { 10, 10 },
                { 10, 1000 },
        });
    }


    @Test
    public void testConflictingStress() throws ExecutionException, InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(threads);
        final int STEPS = 5;

        final LockFreeSet<Integer> s = constructSet();
        ArrayList<Future<ArrayList<Integer>>> tasks = new ArrayList<Future<ArrayList<Integer>>>();
        for (int i = 0; i < threads; i++) {
            final int id = i;
            tasks.add(pool.submit(new Callable<ArrayList<Integer>>() {
                @Override
                public ArrayList<Integer> call() throws Exception {
                    Random rnd = new Random();
                    ArrayList<Integer> bal = new ArrayList<Integer>();
                    for (int i = 0; i < values; i++) {
                        bal.add(0);
                    }
                    for (int i = 0; i < STEPS; i++) {
                        int x = rnd.nextInt(values);
                        if (id % 2 == 0) {
                            if (s.add(x)) {
                                bal.set(x, bal.get(x) + 1);
                            }
                        } else {
                            if (s.remove(x)) {
                                bal.set(x, bal.get(x) - 1);
                            }
                        }
                    }
                    return bal;
                }
            }));
        }
        ArrayList<Integer> bal = new ArrayList<Integer>();
        for (int i = 0; i < values; i++) {
            bal.add(0);
        }
        for (Future<ArrayList<Integer>> f : tasks) {
            ArrayList<Integer> cbal = f.get();
            for (int i = 0; i < bal.size(); i++) {
                bal.set(i, bal.get(i) + cbal.get(i));
            }
        }
        for (int i = 0; i < bal.size(); i++) {
            assertThat(bal.get(i), anyOf(equalTo(0), equalTo(1)));
            assertEquals(bal.get(i) != 0, s.contains(i));
        }

        pool.shutdown();
        pool.awaitTermination(Long.MAX_VALUE, TimeUnit.DAYS);
    }
}