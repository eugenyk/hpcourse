package ru.spbau.gbarto;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

@Param(name = "key", gen = IntGen.class)
@StressCTest
public class LockFreeSetStress {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(name = "key") Integer key) {
        return set.add(key);
    }

    @Operation
    public boolean remove(@Param(name = "key") Integer key) {
        return set.remove(key);
    }

    @Operation
    public boolean contains(@Param(name = "key") Integer key) {
        return set.contains(key);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        List<Integer> ans = new ArrayList<>();
        set.iterator().forEachRemaining(ans::add);
        return ans;
    }

    @Test
    public void LinCheckTest() {
        LinChecker.check(LockFreeSetStress.class);
    }
}