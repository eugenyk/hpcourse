package ru.hse.spb.sharkova.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

@JCStressTest
@Description("Two threads try to add elements to the set simultaneously.")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "Correct execution for two threads.")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Incorrect execution for two threads.")
@State
public class LockFreeSetAddTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private AtomicInteger successes = new AtomicInteger(0);
    private AtomicInteger failures = new AtomicInteger(0);

    private static final int size = 50;

    private void act() {
        List<Integer> values = new ArrayList<>();

        for (int i = 0; i < size; i++) {
            values.add(i);
        }

        Collections.shuffle(values);

        for (int value : values) {
            (set.add(value) ? successes : failures).incrementAndGet();
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
        r.r1 = successes.get() == failures.get() && successes.get() == size;
    }
}
