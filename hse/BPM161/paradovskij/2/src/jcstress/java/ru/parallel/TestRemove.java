package ru.parallel;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicInteger;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Remove success")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Remove fail")
@State
public class TestRemove {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    private int numOfElements = 42;
    private AtomicInteger numOfSuccess = new AtomicInteger(0);

    public TestRemove() {
        for (int i = 0; i < numOfElements; i++) {
            set.add(i);
        }
    }

    private void removeSome() {
        ArrayList<Integer> range = new ArrayList<>();

        for (int i = 0; i < numOfElements; i++) {
            range.add(i);
        }

        Collections.shuffle(range);

        for (int i = 0; i < numOfElements; i++) {
            if (set.remove(i)) {
                numOfSuccess.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor1() {
        removeSome();
    }

    @Actor
    public void actor2() {
        removeSome();
    }

    @Actor
    public void actor3() {
        removeSome();
    }

    @Actor
    public void actor4() {
        removeSome();
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = numOfSuccess.get() == numOfElements && set.isEmpty();
    }
}
