package lockfree;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, false, true", expect = ACCEPTABLE)
@Outcome(id = "true, false, false", expect = ACCEPTABLE)
@Outcome(id = "true, true, false", expect = ACCEPTABLE)
@Outcome(id = "true, true, true", expect = ACCEPTABLE)
@Outcome(id = "false, false, true", expect = ACCEPTABLE)
@Outcome(id = "false, false, false", expect = ACCEPTABLE)
@Outcome(id = "false, true, false", expect = ACCEPTABLE)
@Outcome(id = "false, true, true", expect = ACCEPTABLE)
@State
public class AddRemoveIteratorTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add1() {
        set.add(1);
    }

    @Actor
    public void add2() {
        set.add(2);
    }

    @Actor
    public void add3() {
        set.add(3);
    }

    @Actor
    public void remove1() {
        set.remove(1);
    }

    @Actor
    public void remove2() {
        set.remove(2);
    }

    @Actor
    public void remove3() {
        set.remove(3);
    }

    @Actor
    public void iterator(ZZZ_Result r) {
        boolean[] results = {false, false, false};
        set.iterator().forEachRemaining(el -> results[el - 1] = true);
        r.r1 = results[0];
        r.r2 = results[1];
        r.r3 = results[2];
    }
}


