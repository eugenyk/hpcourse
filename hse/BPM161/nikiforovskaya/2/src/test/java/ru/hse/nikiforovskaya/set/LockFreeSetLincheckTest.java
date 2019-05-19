package ru.hse.nikiforovskaya.set;

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
    public String iterator() {
        List<Integer> l = new ArrayList<>();
        set.iterator().forEachRemaining(l::add);
        return l.toString();
    }

    @Test
    public void runLinCheck() {
        LinChecker.check(this.getClass());
    }
}
