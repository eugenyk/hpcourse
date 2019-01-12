package ru.ifmo.ct.khalansky.hpcourse.queue;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
@Outcome(id = "30",  expect = Expect.ACCEPTABLE, desc = "Correct partial sum.")
@State
public class TwoWritersTwoReadersTest {

    PriorityQueue<Integer> queue = new LockFreePriorityQueue<>();
    volatile int result1 = 0;
    volatile int result2 = 0;
    volatile int taken1 = 0;
    volatile int taken2 = 0;

    @Actor
    public void writer1() {
        for (int i = 1; i <= 5; ++i) {
            queue.offer(i);
        }
    }

    @Actor
    public void writer2() {
        for (int i = 5; i >= 1; --i) {
            queue.offer(i);
        }
    }

    @Actor
    public void reader1() {
        for (int i = 0; i < 10; ++i) {
            Integer value = queue.poll();
            if (value != null) {
                result1 += value;
                taken1++;
            }
        }
    }

    @Actor
    public void reader2() {
        for (int i = 0; i < 10; ++i) {
            Integer value = queue.poll();
            if (value != null) {
                result2 += value;
                taken2++;
            }
        }
    }

    @Arbiter
    public void arbiter(I_Result r) {
        r.r1 = result1 + result2;
        int prev = 0;
        for (int i = taken1 + taken2; i < 10; ++i) {
            Integer value = queue.poll();
            if (prev > value) {
                throw new RuntimeException("Queue order was messed up");
            }
            prev = value;
            r.r1 += value;
        }
    }

}
