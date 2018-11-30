package ru.ifmo.priorityqueue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.II_Result;

import java.util.concurrent.atomic.AtomicInteger;

@org.openjdk.jcstress.annotations.JCStressTest
@Outcome(id = "1, 0", expect = Expect.ACCEPTABLE, desc = "Factorial of 0")
@Outcome(id = "1, 1", expect = Expect.ACCEPTABLE, desc = "Factorial of 1")
@Outcome(id = "2, 2", expect = Expect.ACCEPTABLE, desc = "Factorial of 2")
@Outcome(id = "6, 3", expect = Expect.ACCEPTABLE, desc = "Factorial of 3")
@Outcome(id = "24, 4", expect = Expect.ACCEPTABLE, desc = "Factorial of 4")
@Outcome(id = "120, 5", expect = Expect.ACCEPTABLE, desc = "Factorial of 5")
@Outcome(id = "720, 6", expect = Expect.ACCEPTABLE, desc = "Factorial of 6")
@Outcome(id = "5040, 7", expect = Expect.ACCEPTABLE, desc = "Factorial of 7")
@State
public class JCStressTest {

    private PriorityQueue<Integer> LFPQ = new LockFreePriorityQueue<>();
    private AtomicInteger result = new AtomicInteger(1);
    private AtomicInteger count = new AtomicInteger(0);

    @Actor
    public void producer() {
        for (int i = 1; i <= 7; i++) {
            LFPQ.offer(i);
        }
    }

    @Actor
    public void consumer() {
        for (int i = 0; i < 15; i++) {
            Integer value = LFPQ.poll();
            if (value != null) {
                result.set(result.get() * value);
                count.incrementAndGet();
            }
        }
    }

    @Arbiter
    public void arbiter(II_Result r) {
        r.r1 = result.get();
        r.r2 = count.get();
    }
}
