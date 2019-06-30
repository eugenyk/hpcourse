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
@Outcome(id = "false, false", expect = ACCEPTABLE,  desc = "Both see value.")
@Outcome(id = "true, true", expect = FORBIDDEN,  desc = "Both failed.")
@State
public class LockFreeSetIsEmptyTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    public LockFreeSetIsEmptyTest() {
        set.add(1);
    }

    @Actor
    public void actor1(ZZ_Result r) {
        r.r1 = set.isEmpty();
        set.remove(1);
    }

    @Actor
    public void actor2(ZZ_Result r) {
        r.r2 = set.isEmpty();
        set.remove(1);
    }
}
