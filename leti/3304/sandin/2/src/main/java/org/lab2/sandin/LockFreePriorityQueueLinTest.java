package org.lab2.sandin;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@Param(name = "value", gen = IntGen.class, conf = "1:100")
@StressCTest
public class LockFreePriorityQueueLinTest {

    private LockFreePriorityQueue<Integer> q = new LockFreePriorityQueue<>();

    @Operation
    public void offer(@Param(name = "value") Integer value) {
        q.offer(value);
    }

    @Operation
    public Integer poll() {
        return q.poll();
    }

    @Operation
    public Integer peek() {
        return q.peek();
    }

    
    @Operation
    public Boolean isEmpty() {
        return q.isEmpty();
    }

    @Test
    public void test() {
        LinChecker.check(LockFreePriorityQueueLinTest.class);
    }
}