package net.vasalf.hw.lockfree.jcstress;

import net.vasalf.hw.lockfree.*;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

@JCStressTest
@Outcome(id = "true, true, true, true, true", expect = Expect.ACCEPTABLE)
@Outcome(id = "true, true, true, false, false", expect = Expect.ACCEPTABLE)
@Outcome(id = "true, false, true, true, false", expect = Expect.ACCEPTABLE)
@State
public class AddRemove2Test {
    private MySet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add(ZZZZZ_Result result) {
        result.r1 = set.add(1);
        result.r2 = set.remove(2);
    }

    @Actor
    public void remove(ZZZZZ_Result result) {
        result.r3 = set.add(2);
        result.r4 = set.remove(1);
    }

    @Arbiter
    public void isEmpty(ZZZZZ_Result result) {
        result.r5 = set.isEmpty();
    }
}
