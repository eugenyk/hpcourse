package net.vasalf.hw.lockfree.jcstress;

import net.vasalf.hw.lockfree.*;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.*;

import java.util.Iterator;
import java.util.NoSuchElementException;

@JCStressTest
@Outcome(id="true", expect = Expect.ACCEPTABLE)
@State
public class AddRemoveIterateTest {
    MySet<Integer> set = new LockFreeSet<>();

    @Actor
    public void add() {
        for (int i = 1; i <= 5; i++) {
            set.add(i);
        }
    }

    @Actor
    public void remove() {
        for (int i = 1; i <= 5; i++) {
            set.remove(i);
        }
    }

    @Actor
    public void iterate(Z_Result r) {
        r.r1 = true;
        boolean[] saw = new boolean[6];
        Iterator<Integer> iterator = set.iterator();
        while (iterator.hasNext()) {
            try {
                saw[iterator.next()] = true;
            } catch (NoSuchElementException e) {
                // ¯\_(ツ)_/¯
            }
        }
    }
}
