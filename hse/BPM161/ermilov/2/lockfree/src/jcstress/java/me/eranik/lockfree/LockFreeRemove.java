package me.eranik.lockfree;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;

import java.util.ArrayList;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

@JCStressTest
@Description("Check that remove operation works correctly")
@Outcome(id = "true", expect = Expect.ACCEPTABLE, desc = "Removal from 3 workers is correct")
@Outcome(id = "false", expect = Expect.FORBIDDEN, desc = "Removal from 3 workers is incorrect")
@State
public class LockFreeRemove {
    private final int[] RANGE_START = {0, 500, 1000};
    private final int[] RANGE_END = {1000, 1500, 2000};
    private final int TOTAL_NUMBERS = 2000;

    private Set<Integer> set = new LockFreeSet<>();
    private AtomicInteger totalRemoved = new AtomicInteger(0);

    public LockFreeRemove() {
        IntStream.range(0, TOTAL_NUMBERS).forEach(i -> set.add(i));
    }

    private void add(int i) {
        ArrayList<Integer> values = IntStream.range(RANGE_START[i], RANGE_END[i])
                .boxed().collect(Collectors.toCollection(ArrayList::new));
        Collections.shuffle(values);

        for (int value : values) {
            if (set.remove(value)) {
                totalRemoved.incrementAndGet();
            }
        }
    }

    @Actor
    public void worker_0() {
        add(0);
    }

    @Actor
    public void worker_1() {
        add(1);
    }

    @Actor
    public void worker_2() {
        add(2);
    }

    @Arbiter
    public void checkResult(L_Result result) {
        result.r1 = totalRemoved.get() == TOTAL_NUMBERS;
    }

}
