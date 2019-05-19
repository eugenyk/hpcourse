package ru.spbau.farutin.lockfreeset;

import org.junit.Test;
import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;

import java.util.*;

@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(int value) {
        return set.add(value);
    }

    @Operation
    public boolean remove(int value) {
        return set.remove(value);
    }

    @Operation
    public boolean contains(int value) {
        return set.contains(value);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        Iterator<Integer> iterator = set.iterator();
        List<Integer> values = new ArrayList<>();
        iterator.forEachRemaining(values::add);

        return values;
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(LockFreeSetLinCheckTest.class);
    }
}
