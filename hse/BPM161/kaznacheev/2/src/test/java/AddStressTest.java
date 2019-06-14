import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.Iterator;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "", expect = FORBIDDEN)
@Outcome(id = "0", expect = FORBIDDEN)
@Outcome(id = "1", expect = FORBIDDEN)
@Outcome(id = "01", expect = ACCEPTABLE)
@Outcome(id = "10", expect = ACCEPTABLE)
@State
public class AddStressTest {
    private final LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Actor
    public void first() {
        set.add(0);
    }

    @Actor
    public void second() {
        set.add(1);
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