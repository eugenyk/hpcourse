import org.junit.Test;

import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertEquals;

public class LockFreeSetTest {

    @Test
    public void smoke() throws Exception {
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


    @Test
    public void isEmptyTest() {
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        assertEquals(true, set.isEmpty());
    }

    @Test
    public void isNotEmptyTest() {
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        set.add("test");
        assertEquals(false, set.isEmpty());
    }

    @Test
    public void containsTest() {
        String value = "test";
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        set.add(value);
        assertEquals(true, set.contains(value));
    }

    @Test
    public void notContainsTest() {
        String value = "test";
        String mockValue = "";
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        set.add(mockValue);
        assertEquals(false, set.contains(value));
    }

    @Test
    public void addTest() {
        String value = "test";
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        assertEquals(true, set.add(value));
        assertEquals(true, set.contains(value));
    }

    @Test
    public void rejectTest() {
        String value = "test";
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        assertEquals(true, set.add(value));
        assertEquals(true, set.contains(value));
        assertEquals(false, set.add(value));
    }

    @Test
    public void removeTest() {
        String value = "test";
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        set.add(value);
        assertEquals(true, set.remove(value));
        assertEquals(false, set.contains(value));
    }

    @Test
    public void doNotRemoveTest() {
        String value = "test";
        LockFreeSet<String> set = new LockFreeSetImpl<>();
        assertEquals(false, set.remove(value));
        assertEquals(false, set.contains(value));
    }

    @Test
    public void setPropertyTest() {
        Set<Integer> set = new HashSet<>();
        LockFreeSet<Integer> testingSet = new LockFreeSetImpl<>();
        int dataCount = 500;
        int bound = 10;
        Random random = new Random();
        for (int i = 0; i < dataCount; i++) {
            int value = random.nextInt(bound);
            assertEquals(set.add(value), testingSet.add(value));
            assertEquals(true, testingSet.contains(value));
            assertEquals(false, testingSet.isEmpty());
        }

        for (Integer value : set) {
            assertEquals(true, testingSet.contains(value));
            assertEquals(true, testingSet.remove(value));
            assertEquals(false, testingSet.contains(value));
        }
        assertEquals(true, testingSet.isEmpty());
    }

}
