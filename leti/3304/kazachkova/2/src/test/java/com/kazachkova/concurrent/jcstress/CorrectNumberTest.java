package com.kazachkova.concurrent.jcstress;

import com.kazachkova.concurrent.*;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
@Outcome(id = "2550", expect = Expect.ACCEPTABLE)
@State
public class CorrectNumberTest {
    protected PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    protected int sequenceCount = 50;
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
