package com.pravilov.hse;

import org.apache.commons.collections4.IteratorUtils;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.L_Result;
import org.openjdk.jcstress.util.StringUtils;

import java.util.stream.Collectors;

import static org.openjdk.jcstress.annotations.Expect.ACCEPTABLE;
import static org.openjdk.jcstress.annotations.Expect.FORBIDDEN;

@JCStressTest
@Outcome(id = "", expect = ACCEPTABLE,  desc = "Both removed.")
@Outcome(id = "1", expect = ACCEPTABLE,  desc = "actor1 removed.")
@Outcome(id = "2", expect = ACCEPTABLE,  desc = "actor2 removed.")
@Outcome(id = "1, 2", expect = FORBIDDEN,  desc = "No one removed.")
@State
public class JCStressRemoveTest {
    private LockFreeSetInterface<Integer> set = new LockFreeSet<>();

    @Actor
    public void actor1() {
        set.add(1);
        set.remove(2);
    }

    @Actor
    public void actor2() {
        set.add(2);
        set.remove(1);
    }

    @Arbiter
    public void arbiter(L_Result r) {
        r.r1 = StringUtils.join(IteratorUtils.toList(set.iterator()).stream().map(Object::toString).collect(Collectors.toList()), ", ");
    }
}