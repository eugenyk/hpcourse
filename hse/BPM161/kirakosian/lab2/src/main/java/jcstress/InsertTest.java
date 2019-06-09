package jcstress;

import hse.kirakosian.ILockFreeSet;
import hse.kirakosian.LockFreeSet;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "", expect = FORBIDDEN)
@Outcome(id = "0 ", expect = FORBIDDEN)
@Outcome(id = "1 ", expect = FORBIDDEN)
@Outcome(id = "0 1 ", expect = ACCEPTABLE)
@Outcome(id = "1 0 ", expect = ACCEPTABLE)
@State
public class InsertTest {
    private final ILockFreeSet<Integer> lockFreeSet = new LockFreeSet<>();

    @Actor
    public void actor1() {
        lockFreeSet.add(0);
    }

    @Actor
    public void actor2() {
        lockFreeSet.add(1);
    }

    @Arbiter
    public void arbiter(final L_Result r) {
        final StringBuilder result = new StringBuilder();
        lockFreeSet.iterator().forEachRemaining(value -> result.append(value).append(" "));
        r.r1 = result.toString();
    }

}
