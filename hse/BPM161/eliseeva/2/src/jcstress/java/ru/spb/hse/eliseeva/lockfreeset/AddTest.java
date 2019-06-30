package ru.spb.hse.eliseeva.lockfreeset;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.IIIIII_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "1, 0, 1, 0, 1, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "1, 0, 1, 0, 0, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "1, 0, 1, 0, 0, 0", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "1, 0, 0, 1, 1, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "1, 0, 0, 1, 0, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "1, 0, 0, 1, 0, 0", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "0, 1, 1, 0, 1, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "0, 1, 1, 0, 0, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "0, 1, 1, 0, 0, 0", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "0, 1, 0, 1, 1, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "0, 1, 0, 1, 0, 1", expect = ACCEPTABLE, desc = "add correct")
@Outcome(id = "0, 1, 0, 1, 0, 0", expect = ACCEPTABLE, desc = "add correct")
@Outcome(expect = FORBIDDEN, desc = "add problem")
@State
public class AddTest {
    private final LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();

    @Actor
    public void actor1(IIIIII_Result result) {
        lockFreeSet.add(5);
        result.r1 = lockFreeSet.add(7) ? 1 : 0;
    }

    @Actor
    public void actor2(IIIIII_Result result) {
        lockFreeSet.add(5);
        result.r2 = lockFreeSet.add(7) ? 1 : 0;
    }

    @Actor
    public void actor3(IIIIII_Result result) {
        result.r3 = lockFreeSet.add(15) ? 1 : 0;
    }

    @Actor
    public void actor4(IIIIII_Result result) {
        result.r4 = lockFreeSet.add(15) ? 1 : 0;
    }
    
    @Actor
    public void actor5(IIIIII_Result result) {
        result.r5 = lockFreeSet.contains(7) ? 1 : 0;
        result.r6 = lockFreeSet.contains(5) ? 1 : 0;
    }
}
