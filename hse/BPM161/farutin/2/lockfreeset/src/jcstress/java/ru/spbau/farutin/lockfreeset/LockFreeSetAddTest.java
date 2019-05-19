package ru.spbau.farutin.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.StringJoiner;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "1, 2", expect = ACCEPTABLE,  desc = "Both added successfully.")
@Outcome(id = "2, 1", expect = ACCEPTABLE,  desc = "Both added successfully.")
@Outcome(id = "1", expect = FORBIDDEN,  desc = "One failed.")
@Outcome(id = "2", expect = FORBIDDEN,  desc = "One failed.")
@Outcome(id = "", expect = FORBIDDEN,  desc = "Both failed.")
@State
public class LockFreeSetAddTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
    }

    @Actor
    public void actor2() {
        set.add(2);
    }

    @Arbiter
    public void arbiter(L_Result r) {
        StringJoiner joiner = new StringJoiner(", ");
        set.iterator().forEachRemaining(value -> joiner.add(value.toString()));
        r.r1 = joiner.toString();
    }
}
