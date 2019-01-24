package chirukhin.lab2;

import chirukhin.lab2.priorityqueue.LockFreePriorityQueue;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.II_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "2, 1", expect = ACCEPTABLE, desc = "All ok.")
@Outcome(id = "3, 1", expect = ACCEPTABLE, desc = "All ok.")
@Outcome(expect = FORBIDDEN)
@State
public class ProducerConsumerTest {
    private final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    public ProducerConsumerTest() {
        queue.offer(2);
    }

    @Actor
    public void actor1(II_Result result) {
        result.r1 = queue.poll();
    }

    @Actor
    public void actor2() {
        queue.offer(3);
    }

    @Arbiter
    public void arbiter1(II_Result result) {
        result.r2 = queue.size();
    }
}
