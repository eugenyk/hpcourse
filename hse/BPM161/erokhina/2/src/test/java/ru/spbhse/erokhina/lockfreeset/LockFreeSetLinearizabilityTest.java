package ru.spbhse.erokhina.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import com.devexperts.dxlab.lincheck.annotations.Param;
import org.apache.commons.collections4.IteratorUtils;
import org.junit.Test;

import java.util.List;

@Param(name = "elem", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LockFreeSetLinearizabilityTest {
    private LockFreeSet<Integer> set = new LockFreeSetImpl<>();

    @Operation
    public boolean add(@Param(name = "elem") Integer elem) {
        return set.add(elem);
    }

    @Operation
    public boolean remove(@Param(name = "elem") Integer elem) {
        return set.remove(elem);
    }

    @Operation
    public boolean contains(@Param(name = "elem") Integer elem) {
        return set.contains(elem);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        return IteratorUtils.toList(set.iterator());
    }

    @Test
    public void runTest() {
        LinChecker.check(LockFreeSetLinearizabilityTest.class);
    }
}
