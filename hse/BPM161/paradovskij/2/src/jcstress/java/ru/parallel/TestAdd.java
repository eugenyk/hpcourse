package ru.parallel;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.concurrent.atomic.AtomicInteger;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Add success")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Add fail")
@State
public class TestAdd {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private int numOfElements = 42;
    private AtomicInteger numOfSuccess = new AtomicInteger(0);

    private void addSome() {
        for (int i = 0; i < numOfElements; i++) {
            if (set.add(i)) {
                numOfSuccess.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor1() {
        addSome();
    }

    @Actor
    public void actor2() {
        addSome();
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = numOfSuccess.get() == numOfElements;
    }
}
