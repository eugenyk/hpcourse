package ru.ifmo.ct.khalansky.hpcourse.queue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.II_Result;

@JCStressTest
@Outcome(id = "5, 0",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "4, 1",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "3, 3",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "2, 6",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "1, 10", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "0, 15", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@State
public class OneWriterTwoReadersTest {

    PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    volatile int result1 = 0;
    volatile int result2 = 0;

    @Actor
    public void writer() {
        for (int i = 1; i <= 5; ++i) {
            queue.offer(i);
        }
    }

    @Actor
    public void reader1() {
        int prev = 0;
        for (int i = 1; i <= 5; ++i) {
            Integer value = queue.poll();
            if (value == null) {
                continue;
            }
            if (value <= prev) {
                throw new RuntimeException("Queue ordering is messed up");
            }
            prev = value;
            result2 += value;
        }
    }

    @Actor
    public void reader2() {
        int prev = 0;
        for (int i = 1; i <= 5; ++i) {
            Integer value = queue.poll();
            if (value == null) {
                continue;
            }
            if (value <= prev) {
                throw new RuntimeException("Queue ordering is messed up");
            }
            prev = value;
            result1 += value;
        }
    }

    @Arbiter
    public void arbiter(II_Result r) {
        r.r1 = queue.size();
        r.r2 = result1 + result2;
    }

}
