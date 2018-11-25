package ru.ifmo.ct.khalansky.hpcourse.queue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
@Outcome(id = "0",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "1",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "3",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "6",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "10", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "15", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "21", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "28", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "36", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "45", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@Outcome(id = "55", expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@State
public class OneWriterOneReaderTest {

    PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    volatile int result = 0;

    @Actor
    public void writer() {
        for (int i = 1; i <= 10; ++i) {
            queue.offer(i);
        }
    }

    @Actor
    public void reader(I_Result r) {
        r.r1 = 0;
        int prev = 0;
        for (int i = 1; i <= 10; ++i) {
            Integer value = queue.poll();
            if (value == null) {
                continue;
            }
            if (value <= prev) {
                throw new RuntimeException("Queue ordering is messed up");
            }
            prev = value;
            r.r1 += value;
        }
    }

}
