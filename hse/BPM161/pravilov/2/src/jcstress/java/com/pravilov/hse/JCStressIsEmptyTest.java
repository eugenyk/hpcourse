package com.pravilov.hse;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE,  desc = "Empty.")
@Outcome(id = "false", expect = FORBIDDEN,  desc = "Not empty.")
@State
public class JCStressIsEmptyTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
        set.remove(1);
    }

    @Actor
    public void actor2() {
        set.add(1);
        set.remove(1);
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        r.r1 = set.isEmpty();
    }
}