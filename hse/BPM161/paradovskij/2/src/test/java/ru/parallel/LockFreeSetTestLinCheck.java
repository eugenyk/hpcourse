package ru.parallel;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.LoggingLevel;
import com.devexperts.dxlab.lincheck.Options;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import com.devexperts.dxlab.lincheck.strategy.stress.StressOptions;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@StressCTest
public class LockFreeSetTestLinCheck {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(int key) {
        return set.add(key);
    }

    @Operation
    public boolean remove(int key) {
        return set.remove(key);
    }

    @Operation
    public boolean contains(int key) {
        return set.contains(key);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        ArrayList<Integer> elems = new ArrayList<>();
        set.iterator().forEachRemaining(elems::add);
        return elems;
    }

    @Test
    public void runLinCheck() {
        Options opts = new StressOptions().logLevel(LoggingLevel.DEBUG).threads(2);
        LinChecker.check(LockFreeSetTestLinCheck.class, opts);
    }
}