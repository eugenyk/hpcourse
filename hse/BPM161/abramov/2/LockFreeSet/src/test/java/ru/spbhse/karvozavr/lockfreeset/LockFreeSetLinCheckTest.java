package ru.spbhse.karvozavr.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Operation
    public void add(int key) {
        set.add(key);
    }

    @Operation
    public void remove(int key) {
        set.add(key);
    }


    @Operation
    public void contains(int key) {
        set.contains(key);
    }

    @Operation
    public void isEmpty() {
        set.isEmpty();
    }

    @Operation
    public void iterator() {
        List<Integer> l = new ArrayList<>();
        set.iterator().forEachRemaining(l::add);
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(this.getClass());
    }
}