package ru.hse.myutman.lockfreeset;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;

public class LockFreeSetJCStressTest {

    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "All the values were correctly added")
    @State
    public static class RaceConditionTest {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        @Actor
        public void actor1() {
            set.add(1);
        }

        @Actor
        public void actor2() {
            set.add(2);
        }

        @Actor
        public void actor3() {
            set.add(3);
        }

        @Actor
        public void actor4() {
            set.add(4);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result result) {
            result.r1 = set.contains(1);
            result.r2 = set.contains(2);
            result.r3 = set.contains(3);
            result.r4 = set.contains(4);
        }
    }

    @JCStressTest
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "add1 remove add2 or add2 remove add1")
    @Outcome(id = "true, false, true, false", expect = ACCEPTABLE, desc = "add1 add2 remove")
    @Outcome(id = "false, true, true, false", expect = ACCEPTABLE, desc = "add2 add1 remove")
    @Outcome(id = "true, false, false, true", expect = ACCEPTABLE, desc = "remove add1 add2")
    @Outcome(id = "false, true, false, true", expect = ACCEPTABLE, desc = "remove add2 add1")
    @State
    public static class MultipleAddRemoveConsistencyTest {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        @Actor
        public void actor1(ZZZZ_Result result) {
            result.r1 = set.add(1);
        }

        @Actor
        public void actor2(ZZZZ_Result result) {
            result.r2 = set.add(1);
        }

        @Actor
        public void actor3(ZZZZ_Result result) {
            result.r3 = set.remove(1);
        }

        @Arbiter
        public void arbiter(ZZZZ_Result result) {
            result.r4 = set.contains(1);
        }
    }

    @JCStressTest
    @Outcome(id = "false, false, true, true", expect = ACCEPTABLE, desc = "contains 1 contains 2, add 1, add 2, contains 1, contains 2")
    @Outcome(id = "false, true, true, true", expect = ACCEPTABLE, desc = "contains 2, add 1, contains 1 , add 2, contains 1, contains 2")
    @Outcome(id = "true, false, true, true", expect = ACCEPTABLE, desc = "contains 1, add 2, contains 2 , add 1, contains 1, contains 2")
    @Outcome(id = "false, true, false, true", expect = ACCEPTABLE, desc = "contains 2, add 1, contains 2, contains 1, add 2, contains 1")
    @Outcome(id = "true, false, true, false", expect = ACCEPTABLE, desc = "contains 1, add 2, contains 1, contains 2, add 1, contains 2")
    @Outcome(id = "false, false, false, true", expect = ACCEPTABLE, desc = "contains 2, contains 1, add 1, contains 2, add 2, contains 1")
    @Outcome(id = "false, false, true, false", expect = ACCEPTABLE, desc = "contains 1, contains 2, add 2, contains 1, add 1, contains 2")
    @State
    public static class LinearizableTest {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        @Actor
        public void actor1(ZZZZ_Result result) {
            result.r1 = set.contains(2);
            set.add(1);
            result.r3 = set.contains(2);
        }

        @Actor
        public void actor2(ZZZZ_Result result) {
            result.r2 = set.contains(1);
            set.add(2);
            result.r4 = set.contains(1);
        }
    }

    @JCStressTest
    @Outcome(id = "false, false, false, false", expect = ACCEPTABLE, desc = "Nothing added")
    @Outcome(id = "true, false, false, false", expect = ACCEPTABLE, desc = "One added")
    @Outcome(id = "true, true, false, false", expect = ACCEPTABLE, desc = "Two added")
    @Outcome(id = "true, true, true, false", expect = ACCEPTABLE, desc = "Three added")
    @Outcome(id = "true, true, true, true", expect = ACCEPTABLE, desc = "Four added")
    @State
    public static class IteratorConsistenceTest {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        @Actor
        public void actor1() {
            set.add(1);
            set.add(2);
            set.add(3);
            set.add(4);
        }

        @Actor
        public void actor3(ZZZZ_Result result) {
            Iterator<Integer> iterator = set.iterator();
            Set<Integer> normalSet = new HashSet<>();
            iterator.forEachRemaining(normalSet::add);
            result.r1 = set.contains(1);
            result.r2 = set.contains(2);
            result.r3 = set.contains(3);
            result.r4 = set.contains(4);
        }
    }
}
