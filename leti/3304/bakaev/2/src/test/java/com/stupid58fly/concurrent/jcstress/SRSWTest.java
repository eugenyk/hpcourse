package com.stupid58fly.concurrent.jcstress;

import com.stupid58fly.concurrent.LockFreePriorityQueue;
import com.stupid58fly.concurrent.PriorityQueue;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
// expected value (0 + 100) * (100 - 0 + 1) = 5050
@Outcome(id = "5050", expect = Expect.ACCEPTABLE)
@State
public class SRSWTest {
    protected PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    protected int sequenceCount = 100;
    protected int sum = 0;

    @Actor
    public void writer() {
        for (int i = 0; i < sequenceCount; i++)
            queue.add(i);
    }

    @Actor
    public void reader() {
        for (int i = 0; i < sequenceCount; ) {
            Integer value = queue.poll();
            if (value != null) {
                sum += value;
                i++;
            }
        }
    }

    @Arbiter
    public void arbiter(I_Result result) {
        result.r1 = sum;
    }
}
