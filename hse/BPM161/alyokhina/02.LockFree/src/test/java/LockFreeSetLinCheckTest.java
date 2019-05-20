
import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.*;

@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSet<Integer> lockFreeSet = new LockFreeSetImpl<>();

    @Operation
    public boolean add(int x) {
        return lockFreeSet.add(x);
    }

    @Operation
    public boolean remove(int x) {
        return lockFreeSet.add(x);
    }

    @Operation
    public boolean contains(int x) {
        return lockFreeSet.contains(x);
    }

    @Operation
    public boolean isEmpty() {
        return lockFreeSet.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        Iterator<Integer> iterator = lockFreeSet.iterator();
        List<Integer> list = new ArrayList<>();
        while (iterator.hasNext()) {
            list.add(iterator.next());
        }
        return list;
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(LockFreeSetLinCheckTest.class);
    }
}