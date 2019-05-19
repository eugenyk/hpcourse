package ru.spbhse.karvozavr.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(name = "key") int key) {
        return set.add(key);
    }

    @Operation
    public boolean remove(@Param(name = "key") int key) {
        return set.add(key);
    }


    @Operation
    public boolean contains(@Param(name = "key") int key) {
        return set.contains(key);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        List<Integer> l = new ArrayList<>();
        set.iterator().forEachRemaining(l::add);
        return l;
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(this.getClass());
    }
}