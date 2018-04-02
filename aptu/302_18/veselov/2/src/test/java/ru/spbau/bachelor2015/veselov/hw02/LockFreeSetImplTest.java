package ru.spbau.bachelor2015.veselov.hw02;

import org.jetbrains.annotations.NotNull;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.hamcrest.CoreMatchers.equalTo;
import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.*;

public class LockFreeSetImplTest {
    @Test
    public void additionTest() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        int step = 100;
        int numberOfSteps = 100;

        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < numberOfSteps; i++) {
            int lowerBound = i * step;
            int upperBound = lowerBound + step;

            threads.add(new Thread(new RangeCreator(set, lowerBound, upperBound)));
        }

        runThreads(threads);

        for (int value = 0; value < numberOfSteps * step; value++) {
            assertThat(set.contains(value), is(equalTo(true)));
        }
    }

    @Test
    public void removementTest() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        int step = 100;
        int numberOfSteps = 100;

        for (int value = 0; value < numberOfSteps * step; value++) {
            set.add(value);
        }

        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < numberOfSteps; i++) {
            int lowerBound = i * step;
            int upperBound = lowerBound + step;

            threads.add(new Thread(new RangeRemover(set, lowerBound, upperBound)));
        }

        runThreads(threads);

        for (int value = 0; value < numberOfSteps * step; value++) {
            assertThat(set.contains(value), is(equalTo(false)));
        }


        assertThat(set.isEmpty(), is(equalTo(true)));
    }

    private void runThreads(final @NotNull List<Thread> threads) throws InterruptedException {
        for (Thread thread : threads) {
            thread.start();
        }

        for (Thread thread : threads) {
            thread.join();
        }
    }

    private class RangeCreator implements Runnable {
        private int lowerBound;

        private int upperBound;

        private final @NotNull LockFreeSet<Integer> set;

        public RangeCreator(
            final @NotNull LockFreeSet<Integer> set,
            final int lowerBound,
            final int upperBound
        ) {
            if (lowerBound >= upperBound) {
                throw new IllegalArgumentException();
            }

            this.set = set;

            this.lowerBound = lowerBound;
            this.upperBound = upperBound;
        }

        @Override
        public void run() {
            for (int value = lowerBound; value < upperBound; value++) {
                set.add(value);
            }
        }
    }

    private class RangeRemover implements Runnable {
        private int lowerBound;

        private int upperBound;

        private final @NotNull LockFreeSet<Integer> set;

        public RangeRemover(
            final @NotNull LockFreeSet<Integer> set,
            final int lowerBound,
            final int upperBound
        ) {
            if (lowerBound >= upperBound) {
                throw new IllegalArgumentException();
            }

            this.set = set;

            this.lowerBound = lowerBound;
            this.upperBound = upperBound;
        }

        @Override
        public void run() {
            for (int value = lowerBound; value < upperBound; value++) {
                set.remove(value);
            }
        }
    }
}