package ru.spbau.gbarto;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true, false", expect = ACCEPTABLE)
@State
public class JCStressContains {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(8);
        set.add(800);
    }

    @Actor
    public void actor2() {
        set.add(800);
        set.add(555);
    }

    @Arbiter
    public void arbiter(ZZZZ_Result r) {
        r.r1 = set.contains(8);
        r.r2 = set.contains(800);
        r.r3 = set.contains(555);
        r.r4 = set.contains(3535);
    }
}