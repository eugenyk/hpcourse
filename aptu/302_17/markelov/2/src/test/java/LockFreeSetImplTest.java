import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Random;
import java.util.concurrent.*;

@RunWith(Parameterized.class)
public class LockFreeSetImplTest {
    private final int threads;
    private final int values;

    public LockFreeSetImplTest(int threads, int values) {
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
    public void concurentSmoke() throws ExecutionException, InterruptedException {
        ExecutorService pool = Executors.newFixedThreadPool(threads);
        final int STEPS = 1000;

        final LockFreeSet<Integer> s = constructSet();
        ArrayList<Future<Boolean>> tasks = new ArrayList<Future<Boolean> >();
        for (int i = 0; i < threads; i++) {
            tasks.add(pool.submit(new Callable<Boolean> (){
                @Override
                public Boolean call() {
                    Random rnd = new Random();
                    for (int i = 0; i < STEPS; i++) {
                        int x = rnd.nextInt(values);
                        if (s.add(x)) {
                            if (!s.remove(x))
                                return false;
                        }
                    }
                    return true;
                }

            }));
        }

        for(Future<Boolean> fut : tasks) {
            assertTrue(fut.get());
        }

        pool.shutdown();
        pool.awaitTermination(Long.MAX_VALUE, TimeUnit.DAYS);
    }

    @Test
    public void smokeTest() {
        final LockFreeSet<Integer> s = constructSet();
        for (int i = 0; i < 2; i++) {
            assertTrue(s.add(3));
            assertTrue(s.add(5));
            assertFalse(s.add(3));
            assertFalse(s.add(5));
            assertTrue(s.remove(3));
            assertTrue(s.remove(5));
            assertFalse(s.remove(3));
            assertFalse(s.remove(5));
        }
    }
}