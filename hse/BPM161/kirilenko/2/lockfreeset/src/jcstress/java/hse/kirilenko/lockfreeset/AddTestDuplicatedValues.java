package hse.kirilenko.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.Z_Result;

import java.util.concurrent.atomic.AtomicInteger;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


@JCStressTest
@Description("Check add 40 same elems from 4 threads")
@Outcome(id = "true", expect = ACCEPTABLE, desc = "Add is correct")
@Outcome(id = "false", expect = FORBIDDEN, desc = "Add is incorrect")
@State
public class AddTestDuplicatedValues {
    private LockFreeSet<Integer> lfs = new LockFreeSet<>();
    private AtomicInteger succCnt = new AtomicInteger(0);
    private AtomicInteger failCnt = new AtomicInteger(0);

    @Actor
    public void actor1() {
        for (int i = 0; i < 10; i++) {
            if (lfs.add(i)) {
                succCnt.incrementAndGet();
            } else {
                failCnt.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor2() {
        for (int i = 0; i < 5; i++) {
            if (lfs.add(i)) {
                succCnt.incrementAndGet();
            } else {
                failCnt.incrementAndGet();
            }
        }

        for (int i = 9; i >= 5; i--) {
            if (lfs.add(i)) {
                succCnt.incrementAndGet();
            } else {
                failCnt.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor3() {
        for (int i = 0; i < 10; i+=2) {
            if (lfs.add(i)) {
                succCnt.incrementAndGet();
            } else {
                failCnt.incrementAndGet();
            }
        }

        for (int i = 1; i < 10; i+=2) {
            if (lfs.add(i)) {
                succCnt.incrementAndGet();
            } else {
                failCnt.incrementAndGet();
            }
        }
    }

    @Actor
    public void actor4() {
        for (int i = 9; i >=0; i--) {
            if (lfs.add(i)) {
                succCnt.incrementAndGet();
            } else {
                failCnt.incrementAndGet();
            }
        }
    }

    @Arbiter
    public void arbiter(Z_Result r) {
        boolean success = true;
        for (int i = 0; i < 10; i++) {
            success &= lfs.contains(i);
        }

        r.r1 = success && succCnt.get() == 10 && failCnt.get() == 30;
    }
}