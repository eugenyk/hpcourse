package ru.spbau.farutin.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import java.util.StringJoiner;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true, false", expect = ACCEPTABLE,  desc = "Only one removed successfully.")
@Outcome(id = "false, true", expect = ACCEPTABLE,  desc = "Only one removed successfully.")
@Outcome(id = "true, true", expect = FORBIDDEN,  desc = "Both removed successfully.")
@Outcome(id = "false, false", expect = FORBIDDEN,  desc = "Both failed.")
@State
public class LockFreeSetRemoveTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    public LockFreeSetRemoveTest() {
        set.add(1);
    }

    @Actor
    public void actor1(ZZ_Result r) {
        r.r1 = set.remove(1);
    }

    @Actor
    public void actor2(ZZ_Result r) {
        r.r2 = set.remove(1);
    }
}
