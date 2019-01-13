package ru.spbau.mit.lara;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class TestLincheck {
    private LockFreeSortedList<Integer> q = new LockFreeSortedList<>();

    @Operation
    public boolean offer(@Param(name = "key") int key) {
        return q.offer(key);
    }

    @Operation
    public Integer peek() {
        return q.peek();
    }

    @Operation
    public Integer pool() {
        return q.poll();
    }

    @Test
    public void runTest() {
        LinChecker.check(TestLincheck.class);
    }

}