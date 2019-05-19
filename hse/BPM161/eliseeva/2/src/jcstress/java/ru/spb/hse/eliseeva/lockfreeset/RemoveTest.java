package ru.spb.hse.eliseeva.lockfreeset;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.IIIII_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "1, 0, 1, 1, 0", expect = ACCEPTABLE, desc = "remove correct")
@Outcome(id = "0, 1, 1, 1, 0", expect = ACCEPTABLE, desc = "remove correct")
@Outcome(id = "1, 0, 1, 1, 1", expect = ACCEPTABLE, desc = "remove correct")
@Outcome(id = "0, 1, 1, 1, 1", expect = ACCEPTABLE, desc = "remove correct")
@Outcome(expect = FORBIDDEN, desc = "remove problem")
@State
public class RemoveTest {
    private final LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();
    public RemoveTest() {
        lockFreeSet.add(1);
        lockFreeSet.add(2);
        lockFreeSet.add(10);
        lockFreeSet.add(9);
        lockFreeSet.add(8);
    }

    @Actor
    public void actor1(IIIII_Result result) {
        result.r1 = lockFreeSet.remove(10) ? 1 : 0;
    }

    @Actor
    public void actor2(IIIII_Result result) {
        result.r2 = lockFreeSet.remove(10) ? 1 : 0;
    }

    @Actor
    public void actor3(IIIII_Result result) {
        lockFreeSet.remove(9);
        lockFreeSet.remove(8);
        result.r3 = lockFreeSet.remove(1) ? 1 : 0;
    }

    @Actor
    public void actor4(IIIII_Result result) {
        result.r4 = lockFreeSet.remove(2) ? 1 : 0;
    }

    @Actor
    public void actor5(IIIII_Result result) {
        result.r5 = lockFreeSet.remove(8) ? 1 : 0;
    }
}

