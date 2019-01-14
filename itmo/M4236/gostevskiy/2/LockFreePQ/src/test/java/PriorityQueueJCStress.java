import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

import java.util.Queue;

@JCStressTest
@Outcome(id = "0",  expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "1",  expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "2",  expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "3",  expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "4", expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "5", expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "6", expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "7", expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "8", expect = Expect.ACCEPTABLE, desc = "OK")
@Outcome(id = "9", expect = Expect.ACCEPTABLE, desc = "OK")
@State

public class PriorityQueueJCStress {
    private Queue<Integer> queue = new LockFreePriorityQueue<>();

    @Actor
    public void actor1() {
        for (int i = 0; i <= 10; i += 2) {
            queue.offer(i);
        }
    }

    @Actor
    public void actor2() {
        for (int i = 0; i < 10; i += 2) {
            queue.offer(i);
        }
    }

    @Arbiter
    public void arbiter(I_Result r) {
        Integer pred = queue.poll();
        while (pred == null) pred = queue.poll();
        for (int i = 0; i < 9; i++) {
            Integer cur = queue.poll();
            if (cur == null) continue;
            assert(cur - pred % 2 == 0 && pred > cur);
            r.r1 = cur;
            pred = cur;
        }
    }
}
