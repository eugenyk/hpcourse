package com.pravilov.hse;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;
import org.openjdk.jcstress.infra.results.ZZZ_Result;
import org.openjdk.jcstress.infra.results.ZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "false, true", expect = ACCEPTABLE,  desc = "First was removed.")
@Outcome(id = "true, true", expect = ACCEPTABLE,  desc = "First wasn't removed.")
@Outcome(id = "true, false", expect = ACCEPTABLE,  desc = "Second cannot be removed.")
@Outcome(id = "false, false", expect = ACCEPTABLE,  desc = "Second cannot be removed.")
@State
public class JCStressIsContainsTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
        set.add(1);
    }

    @Actor
    public void actor2() {
        set.remove(1);
        set.add(2);
    }

    @Arbiter
    public void arbiter(ZZ_Result r) {
        r.r1 = set.contains(1);
        r.r2 = set.contains(2);
    }
}