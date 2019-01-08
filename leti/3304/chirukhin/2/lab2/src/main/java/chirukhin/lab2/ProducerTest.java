package chirukhin.lab2;

import chirukhin.lab2.priorityqueue.LockFreePriorityQueue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.*;

@JCStressTest
@Outcome(id = "true, true", expect = ACCEPTABLE, desc = "All ok.")
@Outcome(id = "true, false", expect = ACCEPTABLE_INTERESTING, desc = "Size is broken.")
@Outcome(id = "false, true", expect = ACCEPTABLE_INTERESTING, desc = "Offer broken.")
@Outcome(id = "false, false", expect = FORBIDDEN, desc = "Nothing ran.")
@State
public class ProducerTest {
    private final LockFreePriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    @Actor
    public void actor1(ZZ_Result result) {
        result.r1 = queue.offer(1);
    }

    @Arbiter
    public void arbiter1(ZZ_Result result) {
        result.r2 = !queue.isEmpty();
    }
}
