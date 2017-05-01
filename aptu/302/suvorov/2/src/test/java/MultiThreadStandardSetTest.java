import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class MultiThreadStandardSetTest extends MultiThreadSetTest {
    public MultiThreadStandardSetTest(int threads, int valsPerThread, int operations, boolean initializeFull) {
        super(threads, valsPerThread, operations, initializeFull);
    }

    @Override
    LockFreeSet<Integer> constructSet() {
        return new LockFreeSetStandardLibImpl<>();
    }
}
