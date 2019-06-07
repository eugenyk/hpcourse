package ru.spbau.gbarto;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE)
@State
public class JCStressIsEmpty {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(8);
        set.remove(8);
    }

    @Actor
    public void actor2() {
        set.add(800);
        set.remove(800);
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = set.isEmpty();
    }
}