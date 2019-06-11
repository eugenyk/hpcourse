package net.vasalf.hw.lockfree.jcstress;

import net.vasalf.hw.lockfree.*;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

@JCStressTest
@Outcome(id = "true, true, true", expect = Expect.ACCEPTABLE)
@Outcome(id = "true, false, false", expect = Expect.ACCEPTABLE)
@State
public class AddRemoveTest {
    private MySet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add(ZZZ_Result result) {
        result.r1 = set.add(1);
    }

    @Actor
    public void remove(ZZZ_Result result) {
        result.r2 = set.remove(1);
    }

    @Arbiter
    public void isEmpty(ZZZ_Result result) {
        result.r3 = set.isEmpty();
    }
}
