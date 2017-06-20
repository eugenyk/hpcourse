import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

import static org.junit.Assert.assertTrue;

/**
 * @author belaevstanislav
 */
public class LockFreeSetTest {
    private final Random random = new Random(23950521);

    @Test
    public void simpleTest() {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();

        List<Thread> threads = new ArrayList<>();
        random.ints(10000).sorted().distinct().forEach(i -> threads.add(new Thread(() -> {
                    assertTrue(set.add(i));
                    assertTrue(!set.isEmpty());
                    assertTrue(set.contains(i));
                    assertTrue(set.remove(i));
                }))
        );
        Collections.shuffle(threads, random);
        threads.forEach(Thread::start);
        threads.forEach(thread -> {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
    }


}