package ru.spbau.mit;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.CyclicBarrier;

import static org.junit.Assert.assertTrue;

@RunWith(Parameterized.class)
public class ConcurrentLockFreeSetImplTest {
    @Parameters
    public static Collection<Object[]> data() {
        return Arrays.asList(new Object[][]{
                {2}, {4}, {8}, {16}, {32}, {50},
                {100}, {256}, {1024}, {4098}, {16392}
        });
    }

    private final int nhits;
    private final LockFreeSetImpl<Integer> set;

    public ConcurrentLockFreeSetImplTest(int nBuckets) {
        this.nhits = nBuckets;
        this.set = new LockFreeSetImpl<>();
    }

    @Test
    public void AppendRemoveEvenTest() throws Exception {
        final int nAdderThreads = 3;
        final int nRemoverThreads = 2;
        final int nThreads = nAdderThreads + nRemoverThreads;
//        assert nAdderThreads >= nRemoverThreads;

        CyclicBarrier startBarrier = new CyclicBarrier(nThreads);
        CountDownLatch finishLatch = new CountDownLatch(nThreads);

        Thread[] adderThreads = new Thread[nAdderThreads];
        Thread[] removerThreads = new Thread[nRemoverThreads];

        for (int i = 0; i < nAdderThreads; i++) {
            int finalI = i;
            adderThreads[i] = new Thread(() -> {
                try {
                    startBarrier.await();
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
                for (int j = 0; j < nhits; j++) {
                    assertTrue(set.add(nAdderThreads * j + finalI));
                }
                finishLatch.countDown();
            });
            adderThreads[i].start();
        }

        for (int i = 0; i < nRemoverThreads; i++) {
            int finalI = i;
            removerThreads[i] = new Thread(() -> {
                try {
                    startBarrier.await();
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
                for (int j = 0; j < nhits; j++) {
                    set.remove(nAdderThreads * j + finalI);
                }
                finishLatch.countDown();
            });
            removerThreads[i].start();
        }
        finishLatch.await();

        int[] elementsCounter = new int[nAdderThreads * nhits];

        for (Integer value : set) {
            elementsCounter[value]++;
        }

        for (int i = nRemoverThreads; i < nAdderThreads; ++i) {
            for (int j = 0; j < this.nhits; j++) {
                Assert.assertNotEquals(0, elementsCounter[nAdderThreads * j + i]);
            }
        }
    }
}