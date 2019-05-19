package ru.spb.hse.eliseeva.lockfreeset;

import org.openjdk.jcstress.annotations.*;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

import org.openjdk.jcstress.infra.results.*;

import java.util.Iterator;

@JCStressTest
@Outcome(id = "0, 0, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "1, 0, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "8, 0, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, 1, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "1, 8, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-2, 8, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, 0, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, 1, 8, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-2, 1, 8, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, 10, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, 8, 0, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, -2, 1, 8", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, 8, 10, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, -2, 8, 0", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(id = "-3, -2, 8, 10", expect = ACCEPTABLE, desc = "iterator correct")
@Outcome(expect = FORBIDDEN, desc = "iterator problem")
@State
public class SetIteratorTest {
    private final LockFreeSetInterface<Integer> lockFreeSet = new LockFreeSet<>();

    @Actor
    public void actor1(IIII_Result result) {
        lockFreeSet.add(1);
        lockFreeSet.add(-3);
        lockFreeSet.remove(1);
        lockFreeSet.add(10);
    }

    @Actor
    public void actor2(IIII_Result result) {
        lockFreeSet.add(8);
        lockFreeSet.add(-2);
    }

    @Actor
    public void actor3(IIII_Result result) {
        Iterator<Integer> iterator = lockFreeSet.iterator();
        result.r1 = iterator.hasNext() ? iterator.next() : 0;
        result.r2 = iterator.hasNext() ? iterator.next() : 0;
        result.r3 = iterator.hasNext() ? iterator.next() : 0;
        result.r4 = iterator.hasNext() ? iterator.next() : 0;
    }
}