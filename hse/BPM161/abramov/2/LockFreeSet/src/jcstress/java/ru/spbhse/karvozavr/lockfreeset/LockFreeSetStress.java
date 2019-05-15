package ru.spbhse.karvozavr.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;


public class LockFreeSetStress {

    @JCStressTest
    @Outcome(id = "false", expect = Expect.ACCEPTABLE, desc = "Set is not empty.")
    @Outcome(id = "true", expect = Expect.ACCEPTABLE_INTERESTING, desc = "Set is empty.")
    @State
    public static class AddRemoveTest {
        private LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1() {
            for (int j = 0; j < 100000; ++j) {
                for (int i = 0; i < 10; ++i) {
                    set.add(i);
                }
            }
        }

        @Actor
        public void actor2() {
            for (int j = 0; j < 100000; ++j) {
                for (int i = 0; i < 10; ++i) {
                    set.remove(i);
                }
            }
        }

        @Arbiter
        public void arbiter(L_Result r) {
            r.r1 = set.isEmpty();
        }
    }
}
