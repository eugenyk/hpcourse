package ru.spbau.maxim.hw2;

import org.junit.Test;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class LockFreeSetImplTest {
    @Test
    public void addRemoveTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertThat(set.isEmpty(), is(true));
        assertThat(set.contains(2), is(false));

        assertThat(set.add(1), is(true));
        assertThat(set.add(2), is(true));
        assertThat(set.add(3), is(true));
        assertThat(set.add(2), is(false));

        assertThat(set.contains(1), is(true));
        assertThat(set.contains(2), is(true));
        assertThat(set.contains(3), is(true));

        assertThat(set.isEmpty(), is(false));

        assertThat(set.contains(0), is(false));
        assertThat(set.contains(4), is(false));

        assertThat(set.remove(2), is(true));
        assertThat(set.remove(2), is(false));

        assertThat(set.contains(2), is(false));
        set.add(2);
        assertThat(set.contains(2), is(true));

        assertThat(set.remove(1), is(true));
        assertThat(set.remove(3), is(true));

        assertThat(set.isEmpty(), is(false));

        assertThat(set.remove(2), is(true));

        assertThat(set.isEmpty(), is(true));
    }
}