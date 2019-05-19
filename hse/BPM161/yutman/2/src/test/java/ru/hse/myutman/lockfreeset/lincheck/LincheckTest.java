package ru.hse.myutman.lockfreeset.lincheck;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;
import ru.hse.myutman.lockfreeset.LockFreeSet;
import ru.hse.myutman.lockfreeset.LockFreeSetImpl;

import java.util.ArrayList;
import java.util.List;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LincheckTest {

    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Operation
    public boolean add(@Param(name = "key") int key) { return set.add(key); }

    @Operation
    public boolean remove(@Param(name = "key") int key) { return set.remove(key); }

    @Operation
    public boolean contains(@Param(name = "key") int key) { return set.contains(key); }

    @Operation
    public boolean isEmpty() { return set.isEmpty(); }

    @Operation
    public List<Integer> iterator() {
        List<Integer> ans = new ArrayList<>();
        set.iterator().forEachRemaining(ans::add);
        return ans;
    }

    @Test
    public void test() {
        LinChecker.check(LincheckTest.class);
    }
}
