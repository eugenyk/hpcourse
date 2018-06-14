import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import static org.junit.Assert.assertTrue;

public class MultiThreadsTest {
    @Test
    public void insertAndRemoveTest() throws Exception {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        final CountDownLatch countDownLatch = new CountDownLatch(100);
        final List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            final int id = i;
            final Thread thread = new Thread(() -> {
                countDownLatch.countDown();
                while (true) {
                    try {
                        countDownLatch.await();
                        break;
                    } catch (InterruptedException ignored) {
                    }
                }
                if (id % 2 == 0) {
                    set.add(id / 2);
                } else {
                    //noinspection StatementWithEmptyBody
                    while (!set.contains(id / 2));
                    assertTrue(set.remove(id / 2));
                }
            });
            threads.add(thread);
            thread.start();
        }
        for (Thread thread : threads) {
            thread.join();
        }
    }

    @Test
    public void doubleInsertAndRemoveTest() throws Exception {
        final LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        final CountDownLatch countDownLatch = new CountDownLatch(150);
        final List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 150; i++) {
            final int id = i;
            final Thread thread = new Thread(() -> {
                countDownLatch.countDown();
                while (true) {
                    try {
                        countDownLatch.await();
                        break;
                    } catch (InterruptedException ignored) {
                    }
                }
                if (id % 3 != 0) {
                    //noinspection StatementWithEmptyBody
                    while (!set.add(id / 3));
                } else {
                    //noinspection StatementWithEmptyBody
                    while (!set.remove(id / 3));
                }
            });
            threads.add(thread);
            thread.start();
        }
        for (Thread thread : threads) {
            thread.join();
        }
        for (int i = 0; i < 50; i++) {
            assertTrue(set.contains(i));
        }
    }
}
