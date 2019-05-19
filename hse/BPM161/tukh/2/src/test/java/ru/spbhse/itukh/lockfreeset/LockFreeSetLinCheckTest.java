package ru.spbhse.itukh.lockfreeset;

import com.devexperts.dxlab.lincheck.LinChecker;
import com.devexperts.dxlab.lincheck.LoggingLevel;
import com.devexperts.dxlab.lincheck.Options;
import com.devexperts.dxlab.lincheck.annotations.Operation;
import com.devexperts.dxlab.lincheck.annotations.Param;
import com.devexperts.dxlab.lincheck.paramgen.IntGen;
import com.devexperts.dxlab.lincheck.strategy.stress.StressCTest;
import com.devexperts.dxlab.lincheck.strategy.stress.StressOptions;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@Param(name = "key", gen = IntGen.class, conf = "1:5")
@StressCTest
public class LockFreeSetLinCheckTest {
    private LockFreeSet<Integer> set = new LockFreeSet<>();

    @Operation
    public boolean add(@Param(name = "key") int key) {
        return set.add(key);
    }

    @Operation
    public boolean remove(@Param(name = "key") int key) {
        return set.remove(key);
    }

    @Operation
    public boolean contains(@Param(name = "key") int key) {
        return set.contains(key);
    }

    @Operation
    public boolean isEmpty() {
        return set.isEmpty();
    }

    @Operation
    public List<Integer> iterator() {
        Iterator<Integer> iterator = set.iterator();
        ArrayList<Integer> values = new ArrayList<>();
        while (iterator.hasNext()) {
            values.add(iterator.next());
        }
        return values;
    }

    @Test
    public void LinCheckTestTwoThreads() {
        Options opts = new StressOptions()
                .iterations(5)
                .threads(2)
                .logLevel(LoggingLevel.INFO);
        LinChecker.check(LockFreeSetLinCheckTest.class, opts);
    }

    @Test
    public void LinCheckTestThreeThreads() {
        Options opts = new StressOptions()
                .iterations(3)
                .threads(3)
                .logLevel(LoggingLevel.DEBUG);
        LinChecker.check(LockFreeSetLinCheckTest.class, opts);
    }
}
