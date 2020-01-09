import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.Iterator;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "1 0 ", expect = FORBIDDEN)
@Outcome(id = "0 ", expect = FORBIDDEN)
@Outcome(id = "", expect = ACCEPTABLE)
@Outcome(id = "1 ", expect = ACCEPTABLE)
@Outcome(id = "0 1 ", expect = ACCEPTABLE)
@State
public class RemoveStressTest {
    private final LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Actor
    public void first() {
        set.add(0);
        set.add(1);
    }

    @Actor
    public void actor2() {
        set.remove(1);
        set.remove(0);
    }

    @Arbiter
    public void arbiter(final L_Result result) {
        StringBuilder builder = new StringBuilder();
        Iterator<Integer> it = set.iterator();
        while(it.hasNext()) {
            builder.append(it.next());
        }
        result.r1 = builder.toString();
    }


}