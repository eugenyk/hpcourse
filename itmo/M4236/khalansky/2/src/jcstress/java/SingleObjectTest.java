package ru.ifmo.ct.khalansky.hpcourse.queue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.II_Result;

@JCStressTest
@Outcome(id = "1, 5", expect = Expect.ACCEPTABLE, desc = "Queue was emptied.")
@Outcome(id = "0, 0", expect = Expect.ACCEPTABLE, desc = "Queue was not used.")
@Outcome(id = "1, 0", expect = Expect.ACCEPTABLE, desc = "Race.")
@State
public class SingleObjectTest {

    PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    volatile int result = 0;

    @Actor
    public void actor1() {
        queue.offer(5);
    }

    @Actor
    public void actor2(II_Result r) {
        Integer v = queue.peek();
        r.r1 = queue.size();
        r.r2 = v == null ? 0 : v;
    }

}
