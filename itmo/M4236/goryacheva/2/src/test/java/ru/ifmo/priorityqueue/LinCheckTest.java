package ru.ifmo.priorityqueue;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import org.junit.Test;

@Param(name = "key", gen = IntGen.class, conf = "1:1000")
@StressCTest
public class LinCheckTest {
    private LockFreePriorityQueue<Integer> LFPQ = new LockFreePriorityQueue<>();

    @Operation
    public boolean offer(@Param(name = "key") int key) {
        return LFPQ.offer(key);
    }

    @Operation
    public Integer pool() {
        return LFPQ.poll();
    }

    @Operation
    public Integer peek() {
        return LFPQ.peek();
    }

    @Test
    public void runTest() {
        LinChecker.check(LinCheckTest.class);
    }
}
