package ru.spbhse.karvozavr.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

@JCStressTest
@Description("Test remove correctness.")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "Remove is probably correct.")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Remove is incorrect.")
@State
public class RemoveTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private AtomicInteger trueCount = new AtomicInteger(0);
    private AtomicInteger falseCount = new AtomicInteger(0);

    private static final int ITERATIONS = 50;

    public RemoveTest() {
        for (int j = 0; j < ITERATIONS; ++j) {
            set.add(j);
        }
    }

    private void act() {
        List<Integer> elems = new ArrayList<>();

        for (int j = 0; j < ITERATIONS; ++j) {
            elems.add(j);
        }

        Collections.shuffle(elems);

        for (Integer elem : elems) {
            if (set.remove(elem)) {
                trueCount.incrementAndGet();
            } else {
                falseCount.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor1() {
        act();
    }

    @Actor
    public void actor2() {
        act();
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = falseCount.get() == trueCount.get() && trueCount.get() == ITERATIONS && set.isEmpty();
    }
}
