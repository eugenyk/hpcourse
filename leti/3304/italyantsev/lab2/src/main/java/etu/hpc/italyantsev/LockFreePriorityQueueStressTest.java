package etu.hpc.italyantsev;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "1, 2, 3", expect = ACCEPTABLE)
@Outcome(id = "1, 2, 0", expect = ACCEPTABLE)
@Outcome(id = "1, 3, 0", expect = ACCEPTABLE)
@Outcome(id = "2, 3, 0", expect = ACCEPTABLE)
@Outcome(id = "1, 2, 0", expect = ACCEPTABLE)
@Outcome(id = "1, 0, 0", expect = ACCEPTABLE)
@Outcome(id = "2, 0, 0", expect = ACCEPTABLE)
@Outcome(id = "3, 0, 0", expect = ACCEPTABLE)
public class LockFreePriorityQueueStressTest {

    @State
    public static class QueueState {
        final PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    }

    @Actor
    public void actor1(QueueState state) {
        state.queue.offer(3);
        state.queue.offer(2);
        state.queue.offer(1);
    }

    @Actor
    public void actor2(QueueState state) {
        state.queue.poll();
        state.queue.poll();
    }

    @Arbiter
    public void arbiter(QueueState state, III_Result res) {
        Integer r1 = state.queue.poll();
        Integer r2 = state.queue.poll();
        Integer r3 = state.queue.poll();

        res.r1 = r1 != null ? r1 : 0;
        res.r2 = r2 != null ? r2 : 0;
        res.r3 = r3 != null ? r3 : 0;
    }
}
