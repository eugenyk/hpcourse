package chirukhin.lab2;

import chirukhin.lab2.priorityqueue.LockFreePriorityQueue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.III_Result;

import static org.openjdk.jcstress.annotations.Expect.*;

@JCStressTest
@Outcome(id = "1, 2, 1", expect = ACCEPTABLE, desc = "All ok.")
@Outcome(expect = FORBIDDEN)
@State
public class ConsumerTest {
    private final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    public ConsumerTest() {
        queue.offer(1);
        queue.offer(2);
        queue.offer(3);
    }

    @Actor
    public void actor1(III_Result result) {
        result.r1 = queue.poll();
        result.r2 = queue.poll();
    }

    @Arbiter
    public void arbiter1(III_Result result) {
        result.r3 = queue.size();
    }
}
