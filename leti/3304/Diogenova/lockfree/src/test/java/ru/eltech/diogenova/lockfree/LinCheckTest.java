package ru.eltech.diogenova.lockfree;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

@StressCTest
public class LinCheckTest {
    protected PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();

    @Operation
    public Integer poll() {
        return queue.poll();
    }

    @Operation
    public Integer peek() {
        return queue.peek();
    }

    //    @Operation
    //    public int size() {
    //        return queue.size();
    //    }

    @Operation
    public boolean isEmpty() {
        return queue.isEmpty();
    }

    @Operation
    public boolean add(@Param(gen = IntGen.class) Integer value) {
        return queue.add(value);
    }

    @Operation
    public boolean offer(@Param(gen = IntGen.class) Integer value) {
        return queue.offer(value);
    }

    @Test
    public void runTest() {
        LinChecker.check(this.getClass());
    }
}
