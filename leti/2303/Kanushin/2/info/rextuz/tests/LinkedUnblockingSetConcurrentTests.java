package info.rextuz.tests;

import info.rextuz.hpcourse.LinkedUnblockingSet;
import org.junit.jupiter.api.Test;

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ThreadLocalRandom;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class LinkedUnblockingSetConcurrentTests {
    @Test
    void test_mash() throws InterruptedException {
        LinkedUnblockingSet<Integer> my_set = new LinkedUnblockingSet<>();
        List<Thread> threads = new LinkedList<>();
        for (int i = 0; i < 512; i++)
            threads.add(new Thread(new RandomAccessor(my_set)));
        for (Thread thread : threads)
            thread.start();
        for (Thread thread : threads)
            thread.join();
    }

    class RandomAccessor implements Runnable {
        LinkedUnblockingSet<Integer> my_set;

        RandomAccessor(LinkedUnblockingSet<Integer> my_set) {
            this.my_set = my_set;
        }

        @Override
        public void run() {
            for (int i = 0; i < 100; i++) {
                int randomNum = ThreadLocalRandom.current().nextInt(0, 100 + 1);

                my_set.add(randomNum);
                assertTrue(my_set.contains(randomNum));
                assertFalse(my_set.isEmpty());
            }
        }
    }
}
