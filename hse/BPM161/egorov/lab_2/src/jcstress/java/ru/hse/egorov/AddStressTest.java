package ru.hse.egorov;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

@JCStressTest
@Outcome(id = "true", expect = Expect.ACCEPTABLE)
@Outcome(id = "false", expect = Expect.FORBIDDEN)
@State
public class AddStressTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private AtomicInteger trueCount = new AtomicInteger(0);
    private AtomicInteger falseCount = new AtomicInteger(0);

    private static final int ITERATIONS = 100;

    private void act() {
        List<Integer> elems = new ArrayList<>();

        for (int j = 0; j < ITERATIONS; ++j) {
            elems.add(j);
        }

        Collections.shuffle(elems);

        for (Integer elem : elems) {
            if (set.add(elem)) {
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
        r.r1 = trueCount.get() == falseCount.get() && trueCount.get() == ITERATIONS;
    }
}