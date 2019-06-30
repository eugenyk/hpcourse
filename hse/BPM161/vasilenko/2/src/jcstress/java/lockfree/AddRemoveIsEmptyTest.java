package lockfree;

import org.openjdk.jcstress.annotations.Actor;
import org.openjdk.jcstress.annotations.JCStressTest;
import org.openjdk.jcstress.annotations.Outcome;
import org.openjdk.jcstress.annotations.State;
import org.openjdk.jcstress.infra.results.ZZZ_Result;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

@JCStressTest
@Outcome(id = "true, false, true", expect = ACCEPTABLE)
@Outcome(id = "true, true, true", expect = ACCEPTABLE)
@Outcome(id = "true, false, false", expect = ACCEPTABLE)
@Outcome(id = "true, true, false", expect = ACCEPTABLE)
@State
public class AddRemoveIsEmptyTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add(ZZZ_Result r) {
        r.r1 = set.add(1);
    }

    @Actor
    public void delete(ZZZ_Result r) {
        r.r2 = set.remove(1);
    }

    @Actor
    public void isEmpty(ZZZ_Result r) {
        r.r3 = set.isEmpty();
    }
}
