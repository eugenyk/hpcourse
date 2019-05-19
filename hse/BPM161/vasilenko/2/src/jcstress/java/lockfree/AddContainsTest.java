package lockfree;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.ZZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, true, true, true", expect = ACCEPTABLE)
@State
public class AddContainsTest {
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
    public void add4() {
        set.add(4);
    }

    @Arbiter
    public void contains(ZZZZ_Result r) {
        r.r1 = set.contains(1);
        r.r2 = set.contains(2);
        r.r3 = set.contains(3);
        r.r4 = set.contains(4);
    }
}
