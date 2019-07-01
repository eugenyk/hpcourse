package net.vasalf.hw.lockfree;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@StressCTest
public class LinCheckTest {
    private MySet<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(gen = IntGen.class) int item) {
        return set.add(item);
    }

    @Operation
    public boolean remove(@Param(gen = IntGen.class) int item) {
        return set.remove(item);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public boolean contains(@Param(gen = IntGen.class) int item) {
        return set.contains(item);
    }

    @Operation
    public List<Integer> toList() {
        List<Integer> ret = new ArrayList<>();
        Iterator<Integer> iterator = set.iterator();
        while (iterator.hasNext()) {
            ret.add(iterator.next());
        }
        return ret;
    }

    @Test
    public void test() {
        LinChecker.check(LinCheckTest.class);
    }
}
