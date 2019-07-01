package net.vasalf.hw.lockfree.jcstress;

import net.vasalf.hw.lockfree.*;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

import java.util.concurrent.atomic.AtomicInteger;

@JCStressTest
@Outcome(id = "0", expect = Expect.ACCEPTABLE)
@Outcome(id = "1", expect = Expect.ACCEPTABLE)
@State
public class AddRemove3Test {
    MySet<Integer> set = new LockFreeSet<>();
    AtomicInteger successfullRemoves = new AtomicInteger(0);

    @Actor
    public void add() {
        set.add(1);
    }

    @Actor
    public void remove() {
        if (set.remove(1))
            successfullRemoves.incrementAndGet();
    }

    @Actor
    public void remove2() {
        if (set.remove(1))
            successfullRemoves.incrementAndGet();
    }

    @Arbiter
    public void resolve(I_Result result) {
        result.r1 = successfullRemoves.get();
    }
}
