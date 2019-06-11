package net.vasalf.hw.lockfree.jcstress;

import net.vasalf.hw.lockfree.*;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

@JCStressTest
@Outcome(id = "true, true, true, true, true", expect = Expect.ACCEPTABLE)
@State
public class AddManyTest {
    private MySet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add1(ZZZZZ_Result result) {
        result.r1 = set.add(1);
    }

    @Actor
    public void add2(ZZZZZ_Result result) {
        result.r2 = set.add(2);
    }

    @Actor
    public void add3(ZZZZZ_Result result) {
        result.r3 = set.add(3);
    }

    @Actor
    public void add4(ZZZZZ_Result result) {
        result.r4 = set.add(4);
    }

    @Actor
    public void add5(ZZZZZ_Result result) {
        result.r5 = set.add(5);
    }

    @Arbiter
    public void contains(ZZZZZ_Result result) {
        result.r1 &= set.contains(1);
        result.r2 &= set.contains(2);
        result.r3 &= set.contains(3);
        result.r4 &= set.contains(4);
        result.r5 &= set.contains(5);
    }
}
