package ru.spb.hse.eliseeva.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.Iterator;

@Param(name = "value", gen = IntGen.class)
@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(name = "value") Integer value) {
        return lockFreeSet.add(value);
    }

    @Operation
    public boolean remove(@Param(name = "value") Integer value) {
        return lockFreeSet.remove(value);
    }

    @Operation
    public boolean contains(@Param(name = "value") Integer value) {
        return lockFreeSet.contains(value);
    }

    @Operation
    public String iterator() {
        StringBuilder result = new StringBuilder();
        Iterator iterator = lockFreeSet.iterator();
        while (iterator.hasNext()) {
            result.append(iterator.next().toString()).append(" ");
        }
        return result.toString();
    }

    @Operation
    public boolean isEmpty() {
        return lockFreeSet.isEmpty();
    }

    @Test
    public void runTest() {
        LinChecker.check(LockFreeSetLinCheckTest.class);
    }
}