package com.stupid58fly.concurrent.jcstress;

import com.stupid58fly.concurrent.LockFreePriorityQueue;
import com.stupid58fly.concurrent.PriorityQueue;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;

@JCStressTest
@Outcome(id = "1, 2, 3", expect = Expect.ACCEPTABLE)
@Outcome(id = "2, 3, 0", expect = Expect.ACCEPTABLE)
@Outcome(id = "3, 0, 0", expect = Expect.ACCEPTABLE)
@State
public class CorrectTest {
    protected PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    @Actor
    public void writer() {
        queue.add(1);
        queue.add(2);
        queue.add(3);
    }

    @Actor
    public void reader() {
        queue.poll();
        queue.poll();
    }

    @Arbiter
    public void arbiter(III_Result result) {
        result.r1 = queue.isEmpty() ? 0 : queue.poll();
        result.r2 = queue.isEmpty() ? 0 : queue.poll();
        result.r3 = queue.isEmpty() ? 0 : queue.poll();
    }
}
