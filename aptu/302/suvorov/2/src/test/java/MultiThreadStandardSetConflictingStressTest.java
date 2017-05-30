import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class MultiThreadStandardSetConflictingStressTest  extends MultiThreadSetConflictingStressTest  {
    public MultiThreadStandardSetConflictingStressTest (int threads, int values) {
        super(threads, values);
    }

    @Override
    LockFreeSet<Integer> constructSet() {
        return new LockFreeSetStandardLibImpl<>();
    }
}
