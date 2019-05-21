package me.eranik.lockfree;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;


@Param(name="value", gen=IntGen.class, conf="1:3")
@StressCTest
public class LinCheckTest {
    private Set<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(name="value") int value) {
        return set.add(value);
    }

    @Operation
    public boolean remove(@Param(name="value") int value) {
        return set.remove(value);
    }

    @Operation
    public boolean contains(@Param(name="value") int value) {
        return set.contains(value);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        ArrayList<Integer> list = new ArrayList<>();
        set.iterator().forEachRemaining(list::add);
        return list;
    }

    @Test
    public void testLinearization() {
        LinChecker.check(this.getClass());
    }
}
