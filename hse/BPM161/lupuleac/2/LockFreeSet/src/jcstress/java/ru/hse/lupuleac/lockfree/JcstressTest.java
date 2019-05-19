package ru.hse.lupuleac.lockfree;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;


public class JcstressTest {
    @State
    public static class SetState {
        final LockFreeSet<Integer> set = new LockFreeSet<>();
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, true", expect = ACCEPTABLE, desc = "add should " +
            "succeed")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddDifferentElements {
        @Actor
        public void actor1(SetState state, ZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZ_Result result) {
            result.r2 = state.set.add(1);
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, false", expect = ACCEPTABLE, desc = "first succeed")
    @Outcome(id = "false, true", expect = ACCEPTABLE, desc = "second succeed")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddOneElements {
        @Actor
        public void actor1(SetState state, ZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZ_Result result) {
            result.r2 = state.set.add(0);
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, false, true", expect = ACCEPTABLE, desc = "first " +
            "succeed")
    @Outcome(id = "false, true, true", expect = ACCEPTABLE, desc = "second " +
            "succeed")

    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddThreeTimesTwoElements {
        @Actor
        public void actor1(SetState state, ZZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZZ_Result result) {
            result.r2 = state.set.add(0);
        }

        @Actor
        public void actor3(SetState state, ZZZ_Result result) {
            result.r3 = state.set.add(1);
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, true", expect = ACCEPTABLE, desc = "first " +
            "succeed")
    @Outcome(id = "true, false", expect = ACCEPTABLE, desc = "second " +
            "succeed")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddAndRemove {
        @Actor
        public void actor1(SetState state, ZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZ_Result result) {
            result.r2 = state.set.remove(0);
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, true, true", expect = ACCEPTABLE, desc = "add, " +
            "remove, add again")
    @Outcome(id = "true, false, false", expect = ACCEPTABLE, desc = "remove, " +
            "first add, third failed to add")
    @Outcome(id = "false, false, true", expect = ACCEPTABLE, desc = "remove, " +
            "third add, first failed to add")
    @Outcome(id = "true, true, false", expect = ACCEPTABLE, desc = "first " +
            "adds, third fails, second removes")
    @Outcome(id = "false, true, true", expect = ACCEPTABLE, desc = "third " +
            "adds, first fails, second removes")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddTwiceAndRemove {
        @Actor
        public void actor1(SetState state, ZZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZZ_Result result) {
            result.r2 = state.set.remove(0);
        }

        @Actor
        public void actor3(SetState state, ZZZ_Result result) {
            result.r3 = state.set.add(0);
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, true", expect = ACCEPTABLE, desc = "add and check")
    @Outcome(id = "true, false", expect = ACCEPTABLE, desc = "check and add")

    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddContains {
        @Actor
        public void actor1(SetState state, ZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZ_Result result) {
            result.r2 = state.set.contains(0);
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, true, true", expect = ACCEPTABLE, desc = "add, " +
            "contains, remove")
    @Outcome(id = "true, true, false", expect = ACCEPTABLE, desc = "add, " +
            "remove, contains")
    @Outcome(id = "true, false, true", expect = ACCEPTABLE, desc = "remove, " +
            "add, contains")
    @Outcome(id = "true, false, false", expect = ACCEPTABLE, desc = "remove, " +
            "contains, add")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class AddContainsRemove {
        @Actor
        public void actor1(SetState state, ZZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZZ_Result result) {
            result.r2 = state.set.remove(0);
        }

        @Actor
        public void actor3(SetState state, ZZZ_Result result) {
            result.r3 = state.set.contains(0);
        }
    }


    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "false, true, true", expect = ACCEPTABLE, desc = "add, " +
            "contains, remove")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class IsEmptyRemove {
        @Actor
        public void actor1(SetState state, ZZZ_Result result) {
            result.r1 = state.set.remove(0);
        }

        @Actor
        public void actor2(SetState state, ZZZ_Result result) {
            result.r2 = state.set.isEmpty();
        }

        @Actor
        public void actor3(SetState state, ZZZ_Result result) {
            result.r3 = state.set.isEmpty();
        }
    }

    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "true, true, true", expect = ACCEPTABLE, desc = "is empty, " +
            "add, remove")
    @Outcome(id = "true, false, true", expect = ACCEPTABLE, desc =
            "add, is empty, remove")
    @Outcome(id = "true, false, false", expect = ACCEPTABLE, desc =
            "remove, add, is empty")
    @Outcome(id = "true, true, false", expect = ACCEPTABLE, desc =
            "is empty, remove, add")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class IsEmptyAddRemove {
        @Actor
        public void actor1(SetState state, ZZZ_Result result) {
            result.r1 = state.set.add(0);
        }

        @Actor
        public void actor2(SetState state, ZZZ_Result result) {
            result.r2 = state.set.isEmpty();
        }

        @Actor
        public void actor3(SetState state, ZZZ_Result result) {
            result.r3 = state.set.remove(0);
        }
    }


    @JCStressTest
    @Description("Test add two different elements")
    @Outcome(id = "1, 1, 0", expect = ACCEPTABLE, desc = "is empty, " +
            "add, remove")
    @Outcome(id = "0, 0, 0", expect = ACCEPTABLE, desc = "is empty, " +
            "add, remove")
    @Outcome(expect = FORBIDDEN, desc = "Case violating atomicity.")
    public static class Scan {
        @Actor
        public void actor1(SetState state, II_Result r) {
            state.set.add(1);
        }

        @Actor
        public void actor2(SetState state, II_Result r) {
            state.set.remove(1);
        }

        @Actor
        public void actor3(SetState state, II_Result r) {
            List<Integer> l = state.set.scan();
            r.r1 = l.size();
            if (l.size() > 0) {
                r.r2 = l.get(0);
            }
        }

    }


    @JCStressTest

// These are the test outcomes.
    @Outcome(id = "5, 0, 1, 2, 3, 4", expect = Expect.ACCEPTABLE,
            desc = "One update lost: atomicity failure.")
    @State
    public static class APISample_02_Arbiters {

        LockFreeSet<Integer> set = new LockFreeSet<>();

        @Actor
        public void actor1() {
            set.add(0);
        }

        @Actor
        public void actor2() {
            set.add(1);
        }

        @Actor
        public void actor3() {
            set.add(2);
        }

        @Actor
        public void actor4() {
            set.add(3);
        }

        @Actor
        public void actor5() {
            set.add(4);
        }

        @Arbiter
        public void arbiter(IIIIII_Result r) {
            List<Integer> values = set.scan();
            r.r1 = values.size();
            r.r2 = values.get(0);
            r.r3 = values.get(1);
            r.r4 = values.get(2);
            r.r5 = values.get(3);
            r.r6 = values.get(4);
        }

    }
}

