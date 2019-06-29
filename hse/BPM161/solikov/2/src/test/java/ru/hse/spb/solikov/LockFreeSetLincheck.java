package ru.hse.spb.solikov;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@Param(name = "data", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LockFreeSetLincheck {
    private LockFreeSet<Integer> set = new LockFreeSetWithSnapshots<Integer>();

    @Operation
    public boolean add(@Param(name = "data") Integer data) {
        return set.add(data);
    }

    @Operation
    public boolean remove(@Param(name = "data") Integer data) {
        return set.remove(data);
    }

    @Operation
    public boolean contains(@Param(name = "data") Integer data) {
        return set.contains(data);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        List<Integer> list = new ArrayList<>();
        Iterator<Integer> iterator = set.iterator();
        while (iterator.hasNext()) {
            list.add(iterator.next());
        }
        return list;
    }

    @Test
    public void test() {
        LinChecker.check(LockFreeSetLincheck.class);
    }
}