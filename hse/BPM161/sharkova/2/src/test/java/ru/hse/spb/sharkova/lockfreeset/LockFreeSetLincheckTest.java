package ru.hse.spb.sharkova.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

@StressCTest
public class LockFreeSetLincheckTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Operation
    public void add(int value) {
        set.add(value);
    }

    @Operation
    public void remove(int value) {
        set.add(value);
    }

    @Operation
    public void contains(int value) {
        set.contains(value);
    }

    @Operation
    public void isEmpty() {
        set.isEmpty();
    }

    @Operation
    public void iterator() {
        List<Integer> list = new ArrayList<>();
        set.iterator().forEachRemaining(list::add);
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(LockFreeSetLincheckTest.class);
    }
}