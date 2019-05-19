package com.pravilov.hse;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@Param(name = "value", gen = IntGen.class)
@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(name = "value") Integer value) {
        return set.add(value);
    }

    @Operation
    public boolean remove(@Param(name = "value") Integer value) {
        return set.remove(value);
    }

    @Operation
    public boolean contains(@Param(name = "value") Integer value) {
        return set.contains(value);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        Iterator<Integer> iterator = set.iterator();
        List<Integer> list = new ArrayList<>();

        while(iterator.hasNext()) {
            list.add(iterator.next());
        }
        return list;
    }

    @Test
    public void LinCheckTest() {
        LinChecker.check(LockFreeSetLinCheckTest.class);
    }
}