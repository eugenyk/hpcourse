package ru.spbau.farutin.lockfreeset;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true, false", expect = ACCEPTABLE,  desc = "First removed.")
@Outcome(id = "false, true", expect = ACCEPTABLE,  desc = "Second removed.")
@Outcome(id = "true, true", expect = ACCEPTABLE,  desc = "Both see value.")
@Outcome(id = "false, false", expect = FORBIDDEN,  desc = "Both failed.")
@State
public class LockFreeSetContainsTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    public LockFreeSetContainsTest() {
        set.add(1);
        set.add(2);
    }

    @Actor
    public void actor1(ZZ_Result r) {
        r.r1 = set.contains(1);
        set.remove(2);
    }

    @Actor
    public void actor2(ZZ_Result r) {
        r.r2 = set.contains(2);
        set.remove(1);
    }
}
