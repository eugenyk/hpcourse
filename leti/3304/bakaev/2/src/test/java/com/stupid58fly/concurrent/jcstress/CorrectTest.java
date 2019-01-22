package com.stupid58fly.concurrent.jcstress;

import com.stupid58fly.concurrent.LockFreePriorityQueue;
import com.stupid58fly.concurrent.PriorityQueue;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;

@JCStressTest
@Outcome(id = "1, 2, 3", expect = Expect.ACCEPTABLE)
@Outcome(id = "1, 2, 0", expect = Expect.ACCEPTABLE)
@Outcome(id = "1, 3, 0", expect = Expect.ACCEPTABLE)
@Outcome(id = "2, 3, 0", expect = Expect.ACCEPTABLE)
@Outcome(id = "1, 0, 0", expect = Expect.ACCEPTABLE)
@Outcome(id = "2, 0, 0", expect = Expect.ACCEPTABLE)
@Outcome(id = "3, 0, 0", expect = Expect.ACCEPTABLE)
public class CorrectTest {
    @State
    public static class PriorityQueueState {
        public final PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    }

    @Actor
    public void writer(PriorityQueueState state) {
        state.queue.add(1);
        state.queue.add(2);
        state.queue.add(3);
    }

    @Actor
    public void reader(PriorityQueueState state) {
        state.queue.poll();
        state.queue.poll();
    }

    @Arbiter
    public void arbiter(PriorityQueueState state, III_Result result) {
        result.r1 = state.queue.isEmpty() ? 0 : state.queue.poll();
        result.r2 = state.queue.isEmpty() ? 0 : state.queue.poll();
        result.r3 = state.queue.isEmpty() ? 0 : state.queue.poll();
    }
}
