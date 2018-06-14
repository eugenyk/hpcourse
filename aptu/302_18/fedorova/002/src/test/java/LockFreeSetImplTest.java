import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class LockFreeSetImplTest{
    @Test
    public void main_test() throws Exception {
        LockFreeSet<Integer> a = new LockFreeSetImpl<>();
        assertTrue(a.isEmpty());
        assertTrue(a.add(1));
        assertFalse(a.add(1));
        assertTrue(a.add(2));
        assertTrue(a.add(3));
        assertTrue(a.contains(1));
        assertTrue(a.remove(1));
        assertFalse(a.remove(1));
        assertFalse(a.contains(1));
        assertFalse(a.isEmpty());
    }

    private final Random random = new Random(566);

    @Test
    public void thread_test() {
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
    
    @Test
    public void isEmpty_simple() throws Exception {
        LockFreeSet<Integer> set = new LockFreeSetImpl<>();
        assertTrue(set.isEmpty());
        set.add(1);
        assertFalse(set.isEmpty());
        set.remove(1);
        assertTrue(set.isEmpty());
        set.add(1);
        set.add(2);
        set.remove(2);
        assertFalse(set.isEmpty());
    }
}
