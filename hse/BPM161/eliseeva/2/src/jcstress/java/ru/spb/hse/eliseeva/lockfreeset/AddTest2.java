package ru.spb.hse.eliseeva.lockfreeset;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.III_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


@JCStressTest
@Outcome(id = "1, 1, 0", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "1, 1, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(expect = FORBIDDEN, desc = "add problem")
@State
public class AddTest2 {
    final LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();

    @Actor
    public void actor1(III_Result result) {
        lockFreeSet.add(1);
        lockFreeSet.add(2);
        result.r1 = lockFreeSet.add(3) ? 1 : 0;
    }

    @Actor
    public void actor2(III_Result result) {
        lockFreeSet.add(4);
        lockFreeSet.add(5);
        result.r2 = lockFreeSet.add(6) ? 1 : 0;
    }

    @Actor
    public void actor3(III_Result result) {
        lockFreeSet.add(7);
        lockFreeSet.add(8);
        result.r3 = lockFreeSet.add(1) ? 1 : 0;
    }
}