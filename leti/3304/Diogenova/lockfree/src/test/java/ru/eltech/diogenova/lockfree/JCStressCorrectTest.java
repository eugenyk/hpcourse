package ru.eltech.diogenova.lockfree;

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
@State
public class JCStressCorrectTest {
    final PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    @Actor
    public void actor1() {
        queue.offer(3);
        queue.offer(2);
        queue.offer(1);
    }

    @Actor
    public void actor2() {
        queue.poll();
        queue.poll();
    }

    @Arbiter
    public void arbiter(III_Result res) {
        Integer r1 = queue.poll();
        Integer r2 = queue.poll();
        Integer r3 = queue.poll();

        res.r1 = r1 != null ? r1 : 0;
        res.r2 = r2 != null ? r2 : 0;
        res.r3 = r3 != null ? r3 : 0;
    }
}