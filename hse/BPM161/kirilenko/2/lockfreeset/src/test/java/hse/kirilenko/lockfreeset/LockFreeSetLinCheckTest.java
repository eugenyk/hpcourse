package hse.kirilenko.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.*;

@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSet<Integer> lfs = new LockFreeSet<>();

    @Operation
    public boolean add(int key) {
        return lfs.add(key);
    }

    @Operation
    public boolean remove(int key) {
        return lfs.add(key);
    }

    @Operation
    public boolean contains(int key) {
        return lfs.contains(key);
    }

    @Operation
    public boolean isEmpty() {
        return lfs.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        Iterator<Integer> iterator = lfs.iterator();
        List<Integer> list = new ArrayList<>();

        while(iterator.hasNext()) {
            list.add(iterator.next());
        }
        return list;
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(LockFreeSetLinCheckTest.class);
    }
}